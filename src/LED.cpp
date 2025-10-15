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
  : _pin(pin), _state(LOW), _blink(false), _pulse(false), _timer_start(0UL)
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
  _pulse = false;
}

// turn LED state off
void LED::off()
{
  _state = LOW;
  _blink = false;
  _pulse = false;
}

// toggle LED state from on to off or vv
void LED::toggle()
{
  _state = !_state;
}

// blink the LED
void LED::blink(unsigned int rate)
{
  _blink = true;
  // Start blinking cycle with the LED on.
  _state = HIGH;
  _pulse = false;
  _blink_rate = rate;
  _timer_start = 0;    // timer will expire immediately and illumiate LED
}

// pulse the LED
void LED::pulse(unsigned int duration)
{
  _blink = false;
  _pulse = true;
  _state = HIGH;
  _pulse_duration = duration;
  _timer_start = millis();    // the LED will illuminate now and then toggle once the timer expires
}

// actually operate the LED dependent upon its current state
// must be called frequently from loop() if the LED is set to blink or pulse
void LED::run()
{
  if (_blink)
  {
    // blinking - toggle each time timer expires
    if ((millis() - _timer_start) >= _blink_rate)
    {
      toggle();
      _timer_start = millis();
    }
  }

  // single pulse - switch off after timer expires
  if (_pulse)
  {
    if (millis() - _timer_start >= _pulse_duration)
    {
      _pulse = false;
      _state = LOW;
    }
  }
  _update();
}

// write to the physical pin
void LED::_update()
{
  // DEBUG_SERIAL << F("> mcu pin = ") << pin << F(", state = ") << state << endl;
  digitalWrite(_pin, _state ? HIGH : LOW);
}

}