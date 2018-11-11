// Gsm signal meter

// Displays the signal quality as follows:
// RSSI(dBm)  BER(%)
// -53        0

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
SignalQuality q;

void setup() {

  Console.begin (115200);
  Console.setTimeout (-1);
  Console.println (F ("GsmDuino Signal Meter"));
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

  if (!gsm.waitRegistration (5000)) {

    Console.println (F ("Error: Network registration failed !"));
    exit (EXIT_FAILURE);
  }

  Console.println (F ("GSM module started successfully !\r\n"));

#ifdef __unix__
  Console.println (F ("Press Ctrl+C to abort ..."));
#endif
  Console.println (F ("RSSI(dBm)\tBER(%)"));
}

void loop () {

  if (gsm.signalQuality (q)) {
    Console.print (q.rssi);
    Console.print ("\t\t");
    Console.println (q.ber);
  }
}
