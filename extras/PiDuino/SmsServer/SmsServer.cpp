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

// SMS received callback
// this function is called by the polling loop when a new SMS arrives,
// the m pointer can be used to access the module.
bool mySmsReceivedCB (unsigned int index, GsmDuino::Module * m) {
  GsmDuino::Sms sms;

  if (m->smsRead (sms, index)) {

    Console.println (sms.date());
    Console.print (F ("From: "));
    Console.println (sms.destination());
    Console.print (F ("Text: "));
    Console.println (sms.text());

    m->smsDelete (index);
    return true;
  }
  return false;
}

void setup() {

  Console.begin (115200);
  while (!Console) {
    ; // wait for serial port to connect. Needed for native USB
  }
  Serial1.begin (115200);
  gsm.smsSetReceivedCB (mySmsReceivedCB);
  gsm.begin (Serial1);
}

void loop () {

  gsm.poll (2000);
}
