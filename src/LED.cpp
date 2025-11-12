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

LED::LED()
{
}

LED::LED(byte pin)
  : _pin(pin), _mode(Off), _timer_start(0UL)
{
  pinMode(_pin, OUTPUT);
}

void LED::setPin(byte pin, bool active)
{
  _pin = pin;
  pinMode(_pin, OUTPUT);
  _mode = Off;
  _active = active;
}

// return the current state, on or off
bool LED::getState()
{
  return _mode & IsOn;
}

// turn LED state on
void LED::on()
{
  _mode = On;
}

// turn LED state off
void LED::off()
{
  _mode = Off;
}

// toggle LED state from on to off or vv while blinking
void LED::toggle()
{
  _mode ^= IsOn;
}

// blink the LED
void LED::blink(unsigned int rate)
{
  _mode = Blinking_On;
  _interval = rate;
  _timer_start = millis();
}

// pulse the LED
void LED::pulse(unsigned int duration)
{
  unsigned long now = millis();
  // Set the interval to the max of remainder of current pulse and new pulse.
  if (_mode == Pulsing && _timer_start + _interval > now + duration)
  {
    // Current pulse is still in effect and the current pulse will last longer than the new pulse
    // Keep it.
  }
  else
  {
    // Start pulsing
    _interval = duration;
    _mode = Pulsing;
    _timer_start = now;
  }
  // the LED will illuminate now and then toggle once the timer expires
}

// actually operate the LED dependent upon its current state
// must be called frequently from loop() if the LED is set to blink or pulse
void LED::run()
{
  if (_mode & IsBlinking)
  {
    // blinking - toggle each time timer expires
    if ((millis() - _timer_start) >= _interval)
    {
      toggle();
      _timer_start = millis();
    }
  }

  // single pulse - switch off after timer expires
  if (_mode & IsPulsing)
  {
    if (millis() - _timer_start >= _interval)
    {
      _mode = Off;
    }
  }
  _update();
}

// write to the physical pin
void LED::_update()
{
  _state = getState();
  if (_active == LOW)
  {
    _state = !_state;
  }
  // DEBUG_SERIAL << F("> mcu pin = ") << pin << F(", state = ") << state << endl;
  digitalWrite(_pin, _state ? HIGH : LOW);
}

}