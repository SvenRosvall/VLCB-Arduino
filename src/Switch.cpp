// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

// #include <Streaming.h>
#include "Switch.h"

namespace VLCB
{

//
/// a class to encapsulate a physical pushbutton switch, with non-blocking processing
//
Switch::Switch()
{
}

Switch::Switch(byte pin, byte pressedState)
  : _pin(pin), _pressedState(pressedState)
{
  if (_pressedState == LOW)
  {
    pinMode(_pin, INPUT_PULLUP);
  }

  reset();
  _currentState = readPin();
}

void Switch::setPin(byte pin, byte mode)
{
  _pin = pin;
  _pressedState = HIGH;
  pinMode(_pin, mode);
  if (mode == INPUT_PULLUP)
  {
    _pressedState == LOW;
  }
 
  reset();
  _currentState = readPin();
}

void Switch::reset()
{
  _lastState = !_pressedState;
  _stateChanged = false;
  _lastStateChangeTime = 0UL;
  _lastStateDuration = 0UL;
  _prevReleaseTime = 0UL;
  _prevStateDuration = 0UL;
}

byte Switch::readPin()
{
  return digitalRead(_pin);
}

void Switch::run()
{
  // check for state change

  // read the pin
  _currentState = readPin();

  // has state changed ?
  if (_currentState != _lastState)
  {
    // yes - state has changed since last call to this method
    // DEBUG_SERIAL << endl << F("  -- switch state has changed state from ") << _lastState << " to " << _currentState << endl;
    // DEBUG_SERIAL << F("  -- last state change at ") << _lastStateChangeTime << ", diff = " << millis() - _lastStateChangeTime << endl;

    _lastState = _currentState;
    _prevStateDuration = _lastStateDuration;
    _lastStateDuration = millis() - _lastStateChangeTime;
    _lastStateChangeTime = millis();
    _stateChanged = true;

    if (_currentState == _pressedState)
    {
      // DEBUG_SERIAL << F("  -- switch has been pressed") << endl;
    }
    else
    {
      // DEBUG_SERIAL << F("  -- switch has been released") << endl;

      // double-click detection
      // two clicks of less than 250ms, less than 500ms apart

      // DEBUG_SERIAL << F("  -- last state duration = ") << _lastStateDuration << endl;
      // DEBUG_SERIAL << F("  -- this release = ") << _lastStateChangeTime << F(", last release = ") << _prevReleaseTime << endl;

      // save release time
      _prevReleaseTime = _lastStateChangeTime;
    }
  }
  else
  {
    // no -- state has not changed
    _stateChanged = false;
  }
}

bool Switch::stateChanged()
{
  // has switch state changed ?
  // DEBUG_SERIAL << F("  -- switch state changed = ") << _stateChanged << endl;
  return _stateChanged;
}

bool Switch::getState()
{
  // return the current switch state read
  // DEBUG_SERIAL << F("  -- switch state = ") << _currentState << endl;
  return _currentState;
}

bool Switch::isPressed()
{
  // is the switch pressed ?
  // DEBUG_SERIAL << F("  -- switch pressed ? = ") << (_currentState == _pressedState) << endl;
  return (_currentState == _pressedState);
}

unsigned long Switch::getCurrentStateDuration()
{
  // how long has the switch been in its current state ?
  // DEBUG_SERIAL << F("  -- current state duration = ") << (millis() - _lastStateChangeTime) << endl;
  return (millis() - _lastStateChangeTime);
}

unsigned long Switch::getLastStateDuration()
{
  // how long was the last state active for ?
  // DEBUG_SERIAL << F("  -- last state duration = ") << _lastStateDuration << endl;
  return _lastStateDuration;
}

unsigned long Switch::getLastStateChangeTime()
{
  // when was the last state change ?
  // DEBUG_SERIAL << F("  -- last state change at ") << _lastStateChangeTime << endl;
  return _lastStateChangeTime;
}

void Switch::resetCurrentDuration()
{
  // reset the state duration counter
  _lastStateChangeTime = millis();
}

}