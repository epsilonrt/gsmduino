// GsmDuino pin code demo

// Shows how to use the functions to validate, invalidate, enter and change
// the PIN code of the SIM card.

// Created 11 November 2018
// by Pascal JEAN https://github.com/epsilonrt

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
PinStatus ps;
bool enabled;
int c;
String str;

void getPin (String & pin) {

  Console.print (F ("Pin ? "));
  Console.flush();
  pin = Console.readStringUntil ('\n');
}

void setup() {

  Console.begin (115200);
  Console.setTimeout (-1);
  Console.println (F ("GsmDuino pin code demo"));
  Console.println (F ("Waiting to initialize the module, may take a little while..."));

  gsmSerial.begin (115200);

  ps = gsm.begin (gsmSerial);
  if (ps == WaitingPin) {

    getPin (str);
    ps = gsm.begin (gsmSerial, str);
  }

  if (ps != PinReady) {

    Console.println (F ("Error: Unable to start the GSM module, check its connection and startup !"));
    exit (EXIT_FAILURE);
  }

  if (!gsm.waitRegistration (5000)) {

    Console.println (F ("Error: Network registration failed !"));
    exit (EXIT_FAILURE);
  }

  Console.println (F ("GSM module started successfully !\r\n"));
}

void reset() {

  Console.println (F ("\r\nWaiting to reset the module, may take a little while..."));
  gsm.reset();

  if (gsm.pinStatus() == WaitingPin) {

    getPin (str);
    gsm.pinEnter (str);
    Console.println (gsm.responseMessage());
  }

  if (!gsm.waitRegistration (20000)) {

    Console.println (F ("Error: Network registration failed !"));
  }
}

void enablePin (bool e) {

  if (e != enabled) {

    getPin (str);
    gsm.pinSetEnabled (e, str);
    Console.println (gsm.responseMessage());
  }
}

void changePin() {
  String opin, npin;

  Console.print (F ("Old "));
  getPin (opin);
  Console.print (F ("New "));
  getPin (npin);
  Console.print (F ("New "));
  getPin (str);
  if (str == npin) {
    
    gsm.pinChange (opin, npin);
    Console.println (gsm.responseMessage());
  }
  else {

    Console.print (F ("new PIN codes are not the same !"));
  }
}

void loop () {

  ps = gsm.pinStatus();
  enabled = gsm.pinIsEnabled();

  Console.println (F ("--~~ GsmPin ~~--"));

  Console.print (F ("Enable : "));
  Console.println (enabled);

  Console.print (F ("Status : "));
  switch (ps) {
    case PinReady:
      Console.println (F ("PinReady"));
      break;
    case WaitingPin:
      Console.println (F ("WaitingPin"));
      getPin (str);
      gsm.pinEnter (str);
      Console.println (gsm.responseMessage());
      break;
    case WaitingPuk:
      Console.println (F ("WaitingPuk"));
      break;
    case WaitingPhonePin:
      Console.println (F ("WaitingPhonePin"));
      break;
    case WaitingPhonePuk:
      Console.println (F ("WaitingPhonePuk"));
      break;
    case WaitingPin2:
      Console.println (F ("WaitingPin2"));
      break;
    case WaitingPuk2:
      Console.println (F ("WaitingPuk2"));
      break;
    default :
      Console.println (F ("UnknownPinStatus"));
      break;
  }

  Console.println ();
  Console.println (F ("-- Menu --"));
  Console.println (F ("R: Reset"));
  Console.println (F ("E: Enable pin"));
  Console.println (F ("D: Disable pin"));
  Console.println (F ("C: Change pin"));
  Console.println (F ("P: Enter pin"));
  Console.println (F ("U: Update status"));
  Console.println (F ("Q: Quit"));

  do {
    c = Console.read();
  }
  while (c < 0);

  Console.println();
  switch (c) {
    case 'e':
    case 'E':
      enablePin (true);
      break;
    case 'd':
    case 'D':
      enablePin (false);
      break;
    case 'r':
    case 'R':
      reset();
      break;
    case 'c':
    case 'C':
      changePin();
      break;
    case 'p':
    case 'P':
      getPin (str);
      gsm.pinEnter (str);
      Console.println (gsm.responseMessage());
      break;
    case 'q':
    case 'Q':
      exit(EXIT_SUCCESS);
      break;
    default:
      break;
  }
}
