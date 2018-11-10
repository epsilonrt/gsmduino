// GsmDuino SMS Server with Ping-Pong feature

// Wait for SMS reception and display them. 
// If the SMS text received contains PING in upper case, a PONGn response 
// is sent to the sender.

// Created 10 November 2018
// by Pascal JEAN https://github.com/epsilonrt

// This example code is in the public domain.
#include "gsmduino.h"

GsmDuino::Module  gsm;
HardwareSerial & gsmSerial = Serial1;
unsigned int pongCounter = 1;

// SMS received callback
// this function is called by the polling loop when a new SMS arrives,
// the m pointer can be used to access the module.
bool mySmsReceivedCB (unsigned int index, GsmDuino::Module * m) {
  GsmDuino::Sms sms;

  if (m->smsRead (sms, index)) {
    const String & text = sms.text();
    const String & number = sms.destination();

    Serial.println (sms.date());
    Serial.print (F ("From: "));
    Serial.println (number);
    Serial.print (F ("Text: "));
    Serial.println (text);

    if (text.indexOf ("PING") >= 0) {
      GsmDuino::Sms pong;
      String str ("PONG");

      str += pongCounter;
      pong.setText (str);
      pong.setDestination (number);

      if (m->smsSend (pong)) {

        Serial.print (str);
        Serial.print (F (" sent to "));
        Serial.println (number);
      }
      else {
        // Error
        Serial.println (F ("Error: Unable to pong SMS !"));
      }

      pongCounter++;
    }
    else {

      // Error
      Serial.println (F ("Error: Unable to read SMS !"));
    }

    m->smsDelete (index);
    return true;
  }
  return false;
}

void setup() {

  Serial.begin (115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB
  }
  Serial.println (F ("GsmDuino SMS Server"));
  Serial.println (F ("Waiting to initialize the module, may take a little while..."));

  gsmSerial.begin (115200);
  gsm.smsSetReceivedCB (mySmsReceivedCB);

  if (! gsm.begin (gsmSerial)) {
    Serial.println (F ("Error: Unable to start the GSM module, check its connection and startup !"));
    exit (EXIT_FAILURE);
  }
  gsm.smsDeleteAll();

  Serial.println (F ("GSM module started successfully !\r\n"));
}

void loop () {

  gsm.poll (2000);
}
