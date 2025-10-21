// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

//
//
//

#pragma once

#ifndef DEBUG_SERIAL
#define DEBUG_SERIAL Serial
#endif

#include <Arduino.h>      // for definition of byte datatype
// #include <Streaming.h>

namespace VLCB
{

const int BLINK_RATE = 500;    // flash at 1Hz, 500mS on, 500mS off
const int PULSE_ON_TIME = 10;

//
/// class to encapsulate a non-blocking LED
//
class LED
{
public:
  enum Mode : byte {
    // Choose values so that lsb indicates if LED is on or off.
    Off = 0,
    On = 1,
    Blinking_Off = 2,
    Blinking_On = 3,
    Pulsing = 5,
    
    IsOn = 0x01,
    IsBlinking = 0x02,
    IsPulsing = 0x04
  };
  explicit LED(byte pin);
  bool getState();
  void on();
  void off();
  void toggle();
  void blink(unsigned int rate = BLINK_RATE);
  void pulse(unsigned int duration = PULSE_ON_TIME);

  void run();

protected:
  byte _pin;
  byte _mode;
  unsigned int _interval;
  unsigned long _timer_start;

  void _update();
};

}