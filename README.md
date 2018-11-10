# GsmDuino

## Abstract

**GsmDuino** is a library for GSM/GPRS module. 

It can be used on Arduino and Pi boards thanks to 
[PiDuino](https://github.com/epsilonrt/piduino).


Unlike other libraries, GsmDuino uses any software or hardware serial link as 
long as it is derived from the `Stream` class.

**GsmDuino** uses the AT commands defined in V.25TER, 3GPP TS 27.005

Here is a simple example to display all received SMS:

```c++
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
  Serial1.begin (115200);
  gsm.smsSetReceivedCB (mySmsReceivedCB);
  gsm.begin (Serial1);
}

void loop () {

  gsm.poll (2000);
}
```

This project is under development. For now, only SMS management is functional.

------
