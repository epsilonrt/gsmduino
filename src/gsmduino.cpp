/* Copyright © 2018 Pascal JEAN, All rights reserved.
 * This file is part of the GsmDuino Library.
 *
 * The GsmDuino Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * The GsmDuino Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the GsmDuino Library; if not, see <http://www.gnu.org/licenses/>.
 */
#include "gsmduino.h"

#ifndef NDEBUG
#include <stdio.h>
#define PDEBUG(fmt,...)  fprintf (stderr, fmt, ##__VA_ARGS__)
#endif

#ifdef __AVR__
#include <avr/pgmspace.h>
#else
#include <cstring>
#define PROGMEM
#define PGM_P const char *
#define PSTR(s) ((const PROGMEM char *)(s))
#define pgm_read_ptr(p) ((void *)(p))
#define strstr_P(s1,s2) strstr(s1,s2)
#endif

namespace GsmDuino {

  const char resp_0[] PROGMEM = "OK";
  const char resp_1[] PROGMEM = "ERROR";
  const char resp_2[] PROGMEM = "NOT READY";
  const char resp_3[] PROGMEM = "READY";
  const char resp_4[] PROGMEM = "CONNECT OK";
  const char resp_5[] PROGMEM = "CONNECT FAIL";
  const char resp_6[] PROGMEM = "ALREADY CONNECT";
  const char resp_7[] PROGMEM = "SEND OK";
  const char resp_8[] PROGMEM = "SEND FAIL";
  const char resp_9[] PROGMEM = "DATA ACCEPT";
  const char resp_10[] PROGMEM = "CLOSED";
  const char resp_11[] PROGMEM = ">";

  const char* const respList[] PROGMEM = {
    resp_0,
    resp_1,
    resp_2,
    resp_3,
    resp_4,
    resp_5,
    resp_6,
    resp_7,
    resp_8,
    resp_9,
    resp_10,
    resp_11
  };
  const int respListSize = sizeof (respList) / sizeof (const char *);

  const char status_0[] PROGMEM = "REC UNREAD";
  const char status_1[] PROGMEM = "REC READ";
  const char status_2[] PROGMEM = "STO UNSENT";
  const char status_3[] PROGMEM = "STO SENT";
  const char status_4[] PROGMEM = "ALL";

  const char* const statusList[] PROGMEM = {
    status_0,
    status_1,
    status_2,
    status_3,
    status_4,
  };
  const int statusListSize = sizeof (statusList) / sizeof (const char *);

  // ---------------------------------------------------------------------------
  int findStrInList (const char * what, const char * const list[], const int size) {

    for (int i = 0; i < size; i++) {

      if (strstr_P (what, (PGM_P) pgm_read_ptr (list[i])) != NULL) {

        return i;
      }
    }
    return -1;
  }

  // ---------------------------------------------------------------------------
  //
  //                      GsmDuino::Sms Class
  //
  // ---------------------------------------------------------------------------
  // ---------------------------------------------------------------------------
  Sms::Sms() : _index (0), _status (StoUnsent) {

  }

  // ---------------------------------------------------------------------------
  Sms::Sms (const String & cmgrStr) : Sms() {

    parse (cmgrStr);
  }

  // ---------------------------------------------------------------------------
  Sms::Sms (const String & text, const String & number) : Sms() {

    setText (text);
    setDestination (number);
  }

  // ---------------------------------------------------------------------------
  /*
    AT+CMGR=24
    +CMGR: "REC UNREAD","+33660358994","","18/11/09,21:18:51+04"
    Test24

    OK
  */
  bool Sms::parse (const String & buffer) {
    const int sol1 = buffer.indexOf ("AT+CMGR=");
    const int sol2 = buffer.indexOf ("+CMGR:");

    if ( (sol1 != -1) && (sol2 != -1)) {
      String str, str2;
      int sol3, stx, etx, i;

      // AT+CMGR=24
      stx = buffer.indexOf ('=', sol1) + 1;
      etx = buffer.indexOf ('\r', sol1);
      str = buffer.substring (stx, etx);
      _index = str.toInt();

      // +CMGR: "REC UNREAD","+33660358994","","18/11/09,21:18:51+04"
      etx = buffer.indexOf ('\r', sol2);
      sol3 = etx + 2;
      str = buffer.substring (sol2 + 7, etx);

      // "REC UNREAD","
      stx = str.indexOf ('"') + 1;
      etx = str.indexOf ("\",", stx);
      str2 =  str.substring (stx, etx);
      i = findStrInList (str2.c_str(), statusList, statusListSize);
      if (i >= 0) {
        _status = static_cast<Status> (i);
      }

      // "+33660358994",
      stx = etx + 3;
      etx = str.indexOf ("\",", stx);
      _destination =  str.substring (stx, etx);

      // "",
      stx = etx + 3;
      etx = str.indexOf ("\",", stx);
      _name =  str.substring (stx, etx);

      // "18/11/09,21:18:51+04"
      stx = etx + 3;
      etx = str.indexOf ("\"", stx);
      _date =  str.substring (stx, etx);

      /*
        Test24

        OK
      */
      etx = buffer.indexOf ("\r\n\r\nOK", sol3);
      _text =  buffer.substring (sol3, etx);

      return true;
    }
    return false;
  }

  // ---------------------------------------------------------------------------
  void Sms::clear () {

    _status = StoUnsent;
    _text = "";
    _destination = "";
    _date = "";
    _name = "";
    _index = 0;
  }

  // ---------------------------------------------------------------------------
  void Sms::setText (const String & text) {

    if (status() == StoUnsent) {

      _text = text;
    }
  }

  // ---------------------------------------------------------------------------
  void Sms::setDestination (const String & number) {

    if (status() == StoUnsent) {

      _destination = number;
    }
  }

  // ---------------------------------------------------------------------------
  //
  //                             Module Class
  //
  // ---------------------------------------------------------------------------

  // ---------------------------------------------------------------------------
  Module::Module () :
    _serial (0), _resp (Ok),
    _smsPreviousIndex (0), _smsCurrentIndex (0), _smsReceivedCB (0) {

  }

  // ---------------------------------------------------------------------------
  Module::~Module() {
    close();
  }

  // ---------------------------------------------------------------------------
  bool
  Module::begin (Stream & serialPort) {

    if (!isOpen()) {

      _serial = &serialPort;
      _serial->println (F ("AT"));
      _resp = _waitResponse (1000);

      if (_resp == Ok) {

        _serial->println (F ("AT+CSCS=\"GSM\""));
        _resp = _waitResponse (10000);

        if (_resp == Ok) {

          // delay (100);
          int index =  _smsGetLastIndex();
          if (index >= 0) {

            _smsCurrentIndex = _smsPreviousIndex = index;
          }
        }
      }
      
      if (_resp != Ok) {
        
        close();
      }
    }
    return isOpen();
  }

  // ---------------------------------------------------------------------------
  void
  Module::close() {

    if (isOpen()) {
      _serial = 0;
    }
  }


  // ---------------------------------------------------------------------------
  bool Module::poll (unsigned long timeoutMs) {
    String buffer;

    if (_readData (buffer, timeoutMs)) {
      const int sol = buffer.indexOf ("+CMTI:");

      if (sol >= 0) {
        const int stx = buffer.indexOf (',', sol) + 1;
        const int etx = buffer.indexOf ('\r', sol);
        String strIndex = buffer.substring (stx, etx);

        _smsCurrentIndex = strIndex.toInt();
        if (_smsReceivedCB) {
          if (_smsReceivedCB (_smsCurrentIndex, this)) {
            _smsPreviousIndex = _smsCurrentIndex;
          }
        }
        return true;
      }
    }
    return false;
  }

  // ---------------------------------------------------------------------------
  bool Module::smsSend (Sms & sms) {

    if (isOpen()) {

      _serial->println (F ("AT+CMGF=1")); // set sms to text mode
      _resp = _waitResponse (10000);

      if (_resp == Ok) {

        // delay (1000);
        _serial->print (F ("AT+CMGS=\""));
        _serial->print (sms.destination());
        _serial->println (F ("\""));
        _resp = _waitResponse (60000);

        if (_resp == ReadyToReceive) {

          _serial->println (sms.text());
          _resp = _waitResponse (1000);

          _serial->print ( (char) 26); // CTRL+Z
          _serial->flush();
          _resp = _waitResponse (20000);

          return _resp == Ok;
        }
      }
    }
    return false;
  }

  // ---------------------------------------------------------------------------
  bool Module::smsRead (Sms & sms, unsigned int index) {

    if (isOpen()) {

      _serial->println (F ("AT+CMGF=1"));
      _resp = _waitResponse (10000);

      if (_resp == Ok) {
        String buffer;

        _serial->print (F ("AT+CMGR="));
        _serial->println ( (index == 0) ? _smsCurrentIndex : index);

        if (_readData (buffer)) {

          return sms.parse (buffer);
        }
      }
    }
    return false;
  }

  // ---------------------------------------------------------------------------
  bool Module::smsDelete (unsigned int index) {

    if (isOpen()) {

      _serial->println (F ("AT+CMGF=1"));
      _resp = _waitResponse (10000);

      if (_resp == Ok) {

        _serial->print (F ("AT+CMGD="));
        _serial->println (index);
        _resp = _waitResponse (5000);
        return _resp == Ok;
      }
    }
    return false;
  }

  // ---------------------------------------------------------------------------
  unsigned int Module::smsAvailable() {

    if (isOpen()) {

      if (_smsCurrentIndex > _smsPreviousIndex) {

        _smsPreviousIndex = _smsCurrentIndex;
        return _smsCurrentIndex;
      }
    }
    return 0;
  }

  // ---------------------------------------------------------------------------
  bool Module::smsDeleteAll() {

    if (isOpen()) {

      _serial->println (F ("AT+CMGDA=\"DEL ALL\""));
      _resp = _waitResponse (25000);

      if (_resp == Ok) {

        _smsPreviousIndex = 0;
        _smsCurrentIndex = 0;
        return true;
      }
    }
    return false;
  }

  // ---------------------------------------------------------------------------
  const char *
  Module::responseMessage() const {

    return (PGM_P) pgm_read_ptr (respList[_resp]);
  }

  // ---------------------------------------------------------------------------
  //
  //                     GsmDuino::Module Private Functions
  //
  // ---------------------------------------------------------------------------

  // ---------------------------------------------------------------------------
  GsmDuino::Response Module::_waitResponse (unsigned long timeoutMs) {
    unsigned long tmax = millis() + timeoutMs;

    while (millis() < tmax) {

      if (_serial->available() > 0) {
        String buffer = _serial->readString();

        int i = findStrInList (buffer.c_str(), respList, respListSize);
        if (i >= 0) {

          return static_cast<Response> (i);
        }
      }
    }

    return Timeout;
  }

  // ---------------------------------------------------------------------------
  bool Module::_readData (String & data, unsigned long timeoutMs) {
    unsigned long tmax = millis() + timeoutMs;

    while (millis() < tmax) {

      if (_serial->available() > 0) {
        data = _serial->readString();
        return true;
      }
    }
    return false;
  }

  // ---------------------------------------------------------------------------
  int Module::_smsGetLastIndex() {

    _serial->println (F ("AT+CMGF=1"));
    _resp = _waitResponse (10000);

    if (_resp == Ok) {
      String buffer;

      _serial->println (F ("AT+CMGL=\"ALL\",0"));
      if (_readData (buffer)) {
        int pos = buffer.lastIndexOf ("+CMGL:");

        if (pos != -1) {
          int index;
          String strIndex = buffer.substring (pos + 7, buffer.indexOf (',', pos + 7));

          index = strIndex.toInt();
          return index;
        }
      }
    }
    else {

      return -1;
    }
    return 0;
  }
}
/* ========================================================================== */
