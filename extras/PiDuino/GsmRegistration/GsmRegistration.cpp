// Gsm Network Registration Demo

// Displays the status of the registration on the network and miscellaneous 
// information about the module as follows:

// Manufacturer: SIMCOM_Ltd
// Model: SIMCOM_SIM800C
// IMEI: 2082XXXXXXXXXXX
// Number: +337628XXXX
// Network Registration: RegisteredHome

// Created 11 November 2018

// This example code is in the public domain.
#ifdef __unix__
#include <Piduino.h>  // All the magic is here ;-)
#else
// Defines the serial port as the console on the Arduino platform
#define Console Serial
#endif

#include "gsmduino.h"
using namespace GsmDuino;

Module  gsm;
HardwareSerial & gsmSerial = Serial1;
NetworkRegistration nr;

void setup() {

  Console.begin (115200);
  Console.setTimeout (-1);
  Console.println (F ("GsmDuino Network Registration Demo"));
  Console.println (F ("Waiting to initialize the module, may take a little while..."));

  gsmSerial.begin (115200);

  PinStatus ps = gsm.begin (gsmSerial);
  if (ps == WaitingPin) {
    String pin;

    Console.print (F ("Pin ? "));
    Console.flush();
    pin = Console.readStringUntil ('\n');
    ps = gsm.begin (gsmSerial, pin);
  }

  if (ps != PinReady) {

    Console.println (F ("Error: Unable to start the GSM module, check its connection and startup !"));
    exit (EXIT_FAILURE);
  }

  Console.println (F ("GSM module started successfully !\r\n"));
}

void loop () {
  String str;

  if (gsm.manufacturer (str)) {

    Console.print (F ("Manufacturer: "));
    Console.println (str);
  }
  if (gsm.model (str)) {

    Console.print (F ("Model: "));
    Console.println (str);
  }
  if (gsm.imei (str)) {

    Console.print (F ("IMEI: "));
    Console.println (str);
  }
  if (gsm.subscriberNumber (str)) {

    Console.print (F ("Number: "));
    Console.println (str);
  }

  nr = gsm.networkRegistration();
  Console.print (F ("Network Registration: "));
  switch (nr) {
    case NotRegistered:
      Console.println (F ("NotRegistered"));
      break;
    case RegisteredHome:
      Console.println (F ("RegisteredHome"));
      break;
    case RegistrationInProgress:
      Console.println (F ("RegistrationInProgress"));
      break;
    case RegistrationDenied:
      Console.println (F ("RegistrationDenied"));
      break;
    case RegisteredRoaming:
      Console.println (F ("RegisteredRoaming"));
      break;
    default :
      Console.println (F ("UnknownRegistration"));
      break;
  }

  exit (EXIT_SUCCESS);
}
