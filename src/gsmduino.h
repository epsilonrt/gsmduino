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

#ifndef GSMDUINO_H
#define GSMDUINO_H

#include <Arduino.h>
#include <Stream.h>

/**
 *  @defgroup GsmDuino GsmDuino
 *  @{
 */
namespace GsmDuino {

  enum Response {
    Ok = 0,
    Error,
    NotReady,
    Ready,
    ConnectOk,
    ConnectFail,
    AlreadyConnect,
    SendOk,
    SendFail,
    DataAccept,
    Closed,
    ReadyToReceive,
    //
    Timeout,
    Unknown = -1
  };

  class Sms {
    public:

      enum Status {
        RecUnread = 0,
        RecRead,
        StoUnsent,
        StoSent,
        All
      };

      Sms();
      Sms (const String & cmgrStr);
      Sms (const String & text, const String & number);

      bool parse (const String & cmgrStr);
      void setText (const String & text);
      void setDestination (const String & number);
      void clear();

      inline const String & text() const;
      inline const String & destination() const;
      inline const String & date() const;
      inline const String & name() const;
      inline Status status() const;
      inline unsigned int index() const;
      
    protected:
      unsigned int _index;
      String _text;
      String _destination;
      String _name;
      String _date;
      Status _status;
  };

  class Module {

    public:
      Module ();
      ~Module();

      typedef bool (*smsReceivedCB) (unsigned int smsIndex, Module * m);

      bool begin (Stream & serialPort);
      void close();
      inline bool isOpen() const;

      bool poll (unsigned long timeoutMs = 0);
      bool smsSend (Sms & sms);
      bool smsRead (Sms & sms, unsigned int index = 0);
      bool smsDelete (unsigned int index);
      unsigned int smsAvailable();
      bool smsDeleteAll();
      inline unsigned int smsLastIndex() const;
      inline void smsSetReceivedCB (smsReceivedCB cb);

      const char * responseMessage() const;
      inline Response response() const;

    protected:
      Stream * _serial;
      Response _resp;
      int _smsPreviousIndex;
      int _smsCurrentIndex;
      smsReceivedCB _smsReceivedCB;

      Response _waitResponse (unsigned long timeoutMs);
      bool _readData (String & data, unsigned long timeoutMs = 100000);
      int _smsGetLastIndex();
  };

  inline const String & Sms::text() const {
    return _text;
  }


  inline const String & Sms::destination() const {
    return _destination;
  }

  inline const String & Sms::date() const {
    return _date;
  }
  
  inline const String & Sms::name() const {
    return _name;
  }

  inline Sms::Status Sms::status() const {
    return _status;
  }
  
  inline unsigned int Sms::index() const {
    return _index;
  }

  inline bool Module::isOpen() const {
    return _serial != NULL;
  }

  inline Response Module::response() const {
    return _resp;
  }

  inline unsigned int Module::smsLastIndex() const {
    return _smsCurrentIndex;
  }

  inline void Module::smsSetReceivedCB (Module::smsReceivedCB cb) {
    _smsReceivedCB = cb;
  }
}
/**
 *  @}
 */

/* ========================================================================== */
#endif /*GSMDUINO_H defined */
