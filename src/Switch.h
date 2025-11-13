// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#pragma once

#ifndef DEBUG_SERIAL
#define DEBUG_SERIAL Serial
#endif

#include <Arduino.h>            // for definition of byte datatype

namespace VLCB
{

// non-blocking switch class

class Switch
{
public:
  explicit Switch(byte pin, byte pressedState = LOW);
  Switch();
  void setPin(byte pin, byte mode);
  void run();
  void reset();
  bool stateChanged();
  bool getState();
  bool isPressed();
  unsigned long getCurrentStateDuration();
  unsigned long getLastStateDuration();
  unsigned long getLastStateChangeTime();
  void resetCurrentDuration();

protected:
  byte readPin();
  byte _pin;
  byte _pressedState;
  byte _currentState;
  byte _lastState;
  byte _stateChanged;
  unsigned long _lastStateChangeTime;
  unsigned long _lastStateDuration;
  unsigned long _prevReleaseTime;
  unsigned long _prevStateDuration;
};

}