// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#include "LED.h"

namespace VLCB
{

//
/// class for individual LED with non-blocking control
//

LED::LED(byte pin)
  : _pin(pin), _state(LOW), _blink(false), _pulse(false), _lastTime(0UL)
{
    pinMode(_pin, OUTPUT);
}

// return the current state, on or off
bool LED::getState()
{
  return _state;
}

// turn LED state on
void LED::on()
{
  _state = HIGH;
  _blink = false;
}

// turn LED state off
void LED::off()
{
  _state = LOW;
  _blink = false;
}

// toggle LED state from on to off or vv
void LED::toggle()
{
  _state = !_state;
}

// blink LED
void LED::blink()
{
  _blink = true;
}

// pulse the LED
void LED::pulse()
{
  _pulse = true;
  _pulseStart = millis();
  run();
}

// actually operate the LED dependent upon its current state
// must be called frequently from loop() if the LED is set to blink or pulse
void LED::run()
{
  if (_blink)
  {
    // blinking
    if ((millis() - _lastTime) >= BLINK_RATE)
    {
      toggle();
      _lastTime = millis();
    }
  }

  // single pulse
  if (_pulse)
  {
    if (millis() - _pulseStart >= PULSE_ON_TIME)
    {
      _pulse = false;
    }
  }

  _write(_pin, _pulse || _state);
}

// write to the physical pin
void LED::_write(byte pin, bool state)
{
  // DEBUG_SERIAL << F("> mcu pin = ") << pin << F(", state = ") << state << endl;
  digitalWrite(pin, state);
}

}