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
const int PULSE_ON_TIME = 5;

//
/// class to encapsulate a non-blocking LED
//
class LED
{
public:
  explicit LED(byte pin);
  bool getState();
  void on();
  void off();
  void toggle();
  void blink();
  virtual void run();
  void pulse();

protected:
  byte _pin;
  bool _state;
  bool _blink;
  bool _pulse;
  unsigned long _lastTime, _pulseStart;
  virtual void _write(byte pin, bool state);
};

}