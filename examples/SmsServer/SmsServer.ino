// GsmDuino SMS Server

// Wait for SMS reception and display them. 

// Created 10 November 2018
// by Pascal JEAN https://github.com/epsilonrt

// This example code is in the public domain.
#include "gsmduino.h"

GsmDuino::Module  gsm;

// SMS received callback
// this function is called by the polling loop when a new SMS arrives,
// the m pointer can be used to access the module.
bool mySmsReceivedCB (unsigned int index, GsmDuino::Module * m) {
  GsmDuino::Sms sms;

  if (m->smsRead (sms, index)) {

    Serial.println (sms.date());
    Serial.print (F ("From: "));
    Serial.println (sms.destination());
    Serial.print (F ("Text: "));
    Serial.println (sms.text());

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
  Serial1.begin (115200);
  gsm.smsSetReceivedCB (mySmsReceivedCB);
  gsm.begin (Serial1);
}

void loop () {

  gsm.poll (2000);
}
