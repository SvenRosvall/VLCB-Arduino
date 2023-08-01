
/*

  Copyright (C) Duncan Greenwood 2017 (duncan_greenwood@hotmail.com)

  This work is licensed under the:
      Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
   To view a copy of this license, visit:
      http://creativecommons.org/licenses/by-nc-sa/4.0/
   or send a letter to Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.

   License summary:
    You are free to:
      Share, copy and redistribute the material in any medium or format
      Adapt, remix, transform, and build upon the material

    The licensor cannot revoke these freedoms as long as you follow the license terms.

    Attribution : You must give appropriate credit, provide a link to the license,
                  and indicate if changes were made. You may do so in any reasonable manner,
                  but not in any way that suggests the licensor endorses you or your use.

    NonCommercial : You may not use the material for commercial purposes. **(see note below)

    ShareAlike : If you remix, transform, or build upon the material, you must distribute
                 your contributions under the same license as the original.

    No additional restrictions : You may not apply legal terms or technological measures that
                                 legally restrict others from doing anything the license permits.

   ** For commercial use, please contact the original copyright holder(s) to agree licensing terms

    This software is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE

*/

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

LED::LED()
  : _state(LOW), _blink(false), _pulse(false), _lastTime(0UL)
{
}

//  set the pin for this LED
void LED::setPin(byte pin)
{
  _pin = pin;
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