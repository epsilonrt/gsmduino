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

  const char pin_0[] PROGMEM = "READY";
  const char pin_1[] PROGMEM = "SIM PIN";
  const char pin_2[] PROGMEM = "SIM PUK";
  const char pin_3[] PROGMEM = "PH_SIM PIN";
  const char pin_4[] PROGMEM = "PH_SIM PUK";
  const char pin_5[] PROGMEM = "SIM PIN2";
  const char pin_6[] PROGMEM = "SIM PUK2";

  const char* const statusPinList[] PROGMEM = {
    pin_0,
    pin_1,
    pin_2,
    pin_3,
    pin_4,
    pin_5,
    pin_6
  };
  const int statusPinListSize = sizeof (statusPinList) / sizeof (const char *);

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
  PinStatus
  Module::begin (Stream & serialPort, const String & pin) {
    PinStatus status = UnknownPinStatus;

    if (!isOpen()) {

      _serial = &serialPort;
      _serial->println (F ("AT"));
      _resp = _waitResponse (10000);

      if (_resp == Ok) {

        status = pinStatus();
        if ( (status == PinReady) || (status == WaitingPin)) {

          if (status == WaitingPin) {

            if (pin.empty()) {

              _resp = Error;
            }
            else {

              if (!pinEnter (pin)) {

                _resp = Error;
              }
            }
          }

          if (_resp == Ok) {

            _serial->println (F ("AT+CSCS=\"GSM\""));
            _resp = _waitResponse (10000);

            if (_resp == Ok) {

              int index =  _smsGetLastIndex();
              if (index >= 0) {

                _smsCurrentIndex = _smsPreviousIndex = index;
              }
            }
          }
        }
      }
      else {

        _resp = Error;
      }

      if (_resp != Ok) {

        close();
      }
    }
    return status;
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
  GsmDuino::NetworkRegistration Module::networkRegistration() {
    NetworkRegistration reg = UnknownRegistration;
    String buffer;

    if (stringParameter ("AT+CREG?", buffer)) {
      const int stx = buffer.indexOf (',') + 1;
      String str = buffer.substring (stx, stx + 1);

      reg = static_cast<NetworkRegistration> (str.toInt());
    }
    return reg;
  }

  // ---------------------------------------------------------------------------
  bool Module::waitRegistration (unsigned long timeOut) {

    if (isOpen()) {
      PinStatus status;
      unsigned long tmax = ( (long) timeOut != -1) ? millis() + timeOut : timeOut;

      status = pinStatus() ;
      if (status == PinReady) {
        NetworkRegistration nr;

        nr = networkRegistration();

        while ( (nr != RegisteredHome) && (nr != RegisteredRoaming) &&
                (millis() < tmax)) {

          nr = networkRegistration();
        }
        return (nr == RegisteredHome) || (nr == RegisteredRoaming);
      }
    }
    return false;
  }

  // ---------------------------------------------------------------------------
  bool Module::signalQuality (GsmDuino::SignalQuality & quality) {
    String buffer;

    if (stringParameter ("AT+CSQ", buffer)) {
      int etx = buffer.indexOf (',');
      String str = buffer.substring (6, etx);

      quality.rssi = -113 + (str.toInt() * 2);
      str = buffer.substring (etx + 1);
      quality.ber = str.toInt();
      return true;
    }
    return false;
  }

  // ---------------------------------------------------------------------------
  GsmDuino::Functionality Module::functionality() {
    Functionality f = UnknownFunctionality;
    String buffer;

    if (stringParameter ("AT+CFUN?", buffer)) {
      String str = buffer.substring (7);
      int i = str.toInt();

      if (i >= 0) {
        f = static_cast<Functionality> (i);
      }
    }
    return f;
  }

  // ---------------------------------------------------------------------------
  bool Module::setFunctionality (Functionality f, bool resetBefore) {

    if (isOpen()) {

      _serial->print (F ("AT+CFUN="));
      _serial->print (static_cast<int> (f));
      if (resetBefore) {

        _serial->print (",1");
      }
      _serial->println();
      _resp = _waitResponse (10000);
      return _resp == Ok;
    }
    return false;
  }

  // ---------------------------------------------------------------------------
  bool Module::reset() {
    Functionality f = functionality();
    if (f != UnknownFunctionality) {
      if (setFunctionality (f, true)) {

        do {
          _serial->println (F ("AT"));
          _resp = _waitResponse (1000);
        }
        while (_resp != Ok);
        return true;
      }
    }
    return false;
  }

  // ---------------------------------------------------------------------------
  GsmDuino::PinStatus Module::pinStatus() {
    PinStatus pinstatus = UnknownPinStatus;
    String buffer;

    if (stringParameter ("AT+CPIN?", buffer, 5000)) {
      String str = buffer.substring (7);
      int i = findStrInList (str.c_str(), statusPinList, statusPinListSize);

      if (i >= 0) {
        pinstatus = static_cast<PinStatus> (i);
      }
    }
    return pinstatus;
  }

  // ---------------------------------------------------------------------------
  bool Module::pinIsEnabled () {
    String buffer;

    if (stringParameter ("AT+CLCK=\"SC\",2", buffer, 15000)) {

      String str = buffer.substring (7);
      return str.toInt() != 0;
    }
    return false;
  }

  // ---------------------------------------------------------------------------
  bool Module::pinSetEnabled (bool enabled, const String & pin) {

    if (isOpen()) {

      _serial->print (F ("AT+CLCK=\"SC\","));
      _serial->write (enabled ? '1' : '0');
      _serial->print (F (",\""));
      _serial->print (pin);
      _serial->println (F ("\""));
      _resp = _waitResponse (15000);
      return _resp == Ok;
    }
    return false;
  }

  // ---------------------------------------------------------------------------
  bool Module::pinEnter (const String & pin) {

    if (isOpen()) {

      _serial->print (F ("AT+CPIN=\""));
      _serial->print (pin);
      _serial->println (F ("\""));
      _resp = _waitResponse (5000);
      return _resp == Ok;
    }
    return false;
  }

  // ---------------------------------------------------------------------------
  bool Module::pinChange (const String & oldpin, const String & newpin) {

    if (isOpen()) {

      if (pinIsEnabled()) {
        _serial->print (F ("AT+CPWD=\"SC\",\""));
        _serial->print (oldpin);
        _serial->print (F ("\",\""));
        _serial->print (newpin);
        _serial->println (F ("\""));
        _resp = _waitResponse (15000);
        return _resp == Ok;
      }
    }
    return false;
  }

  // ---------------------------------------------------------------------------
  bool Module::subscriberNumber (String & n) {
    String buffer;

    if (stringParameter ("AT+CNUM", buffer)) {
      const int stx = buffer.indexOf (",\"") + 2;
      const int etx = buffer.indexOf ("\"", stx);

      n = buffer.substring (stx, etx);
      return true;
    }
    return false;
  }

  // ---------------------------------------------------------------------------
  bool Module::stringParameter (const String & atCmd, String & str,
                                unsigned long timeOut) {

    if (isOpen()) {
      String buffer;

      _serial->println (atCmd);
      if (_readData (buffer, timeOut)) {

        _resp = _getResponse (buffer.c_str());
        if (_resp == Ok) {
          const int sol = buffer.indexOf ("\r\n") + 2;

          if (sol >= 0) {
            const int etx = buffer.indexOf ('\r', sol);

            str = buffer.substring (sol, etx);
            return true;
          }
        }
      }
    }
    return false;
  }

  // ---------------------------------------------------------------------------
  // AT+CIMI Request international mobile subscriber identity
  bool Module::imei (String & i) {

    return stringParameter ("AT+CIMI", i, 20000);
  }

  // ---------------------------------------------------------------------------
  // AT+CGMI Request manufacturer identification
  bool Module::manufacturer (String & m) {

    return stringParameter ("AT+CGMI", m);
  }

  // ---------------------------------------------------------------------------
  // AT+CGMM Request Model Identification
  bool Module::model (String & m) {

    return stringParameter ("AT+CGMM", m);
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
  GsmDuino::Response Module::_getResponse (const char * str) {
    int i = findStrInList (str, respList, respListSize);

    if (i >= 0) {

      return static_cast<Response> (i);
    }

    return UnknownResponse;
  }

  // ---------------------------------------------------------------------------
  GsmDuino::Response Module::_waitResponse (unsigned long timeoutMs) {
    unsigned long tmax = millis() + timeoutMs;

    while (millis() < tmax) {

      if (_serial->available() > 0) {

        return _getResponse (_serial->readString().c_str());
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
