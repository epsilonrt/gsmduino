// GsmDuino SMS Server

// Wait for SMS reception and display them. 
// If the SMS text received contains PING in upper case, a PONGn response 
// is sent to the sender.

// Created 10 November 2018
// by Pascal JEAN https://github.com/epsilonrt

// This example code is in the public domain.
#ifdef __unix__
#include <Piduino.h>  // All the magic is here ;-)
#else
// Defines the serial port as the console on the Arduino platform
#define Console Serial
#endif

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

    Console.println (sms.date());
    Console.print (F ("From: "));
    Console.println (number);
    Console.print (F ("Text: "));
    Console.println (text);

    if (text.indexOf ("PING") >= 0) {
      GsmDuino::Sms pong;
      String str ("PONG");

      str += pongCounter;
      pong.setText (str);
      pong.setDestination (number);

      if (m->smsSend (pong)) {

        Console.print (str);
        Console.print (F (" sent to "));
        Console.println (number);
      }
      else {
        // Error
        Console.println (F ("Error: Unable to pong SMS !"));
      }

      pongCounter++;
    }
    else {

      // Error
      Console.println (F ("Error: Unable to read SMS !"));
    }

    m->smsDelete (index);
    return true;
  }
  return false;
}

void setup() {

  Console.begin (115200);
  Console.println (F ("GsmDuino SMS Server"));
  Console.println (F ("Waiting to initialize the module, may take a little while..."));

  gsmSerial.begin (115200);
  gsm.smsSetReceivedCB (mySmsReceivedCB);

  if (! gsm.begin (gsmSerial)) {
    Console.println (F ("Error: Unable to start the GSM module, check its connection and startup !"));
    exit (EXIT_FAILURE);
  }
  gsm.smsDeleteAll();

  Console.println (F ("GSM module started successfully !\r\n"));
#ifdef __unix__
  Console.println (F ("Press Ctrl+C to abort ..."));
#endif
}

void loop () {

  gsm.poll (2000);
}
