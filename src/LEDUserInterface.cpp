// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#include "LEDUserInterface.h"
#include "Controller.h"
#include <Streaming.h>

namespace VLCB
{

LEDUserInterface::LEDUserInterface(byte greenLedPin, byte yellowLedPin, byte pushButtonPin)
  : greenLed(greenLedPin), yellowLed(yellowLedPin), pushButton(pushButtonPin)
{

}

bool LEDUserInterface::isButtonPressed()
{
  return pushButton.isPressed();
}

void LEDUserInterface::run()
{
  pushButton.run();
  greenLed.run();
  yellowLed.run();

  if (resetRequested())
  {
    //DEBUG_SERIAL << "> Button is pressed for mode change" << endl;
    indicateMode(MODE_SETUP);
  }
}

void LEDUserInterface::indicateResetting()
{
  pushButton.reset();
  greenLed.blink();
  yellowLed.blink();
}

void LEDUserInterface::indicateActivity()
{
  greenLed.pulse();
}

bool LEDUserInterface::resetRequested()
{
  //Serial << "UI resetRequested() isPressed=" << pushButton.isPressed() << " currentStateDuration=" << pushButton.getCurrentStateDuration() << endl;
  return pushButton.isPressed() && pushButton.getCurrentStateDuration() > SW_TR_HOLD;
}

void LEDUserInterface::indicateMode(VlcbModeParams mode) {
  switch (mode) {

    case MODE_NORMAL:
      //Serial << "UI indicateMode Normal" << endl;
      yellowLed.on();
      greenLed.off();
      break;

    case MODE_UNINITIALISED:
      //Serial << "UI indicateMode Uninitialised" << endl;
      yellowLed.off();
      greenLed.on();
      break;

    case MODE_SETUP:
      //Serial << "UI indicateMode changing" << endl;
      yellowLed.blink();
      greenLed.off();
      break;

    default:
      break;
  }
}

UserInterface::RequestedAction LEDUserInterface::checkRequestedAction()
{
  //Serial << "UI checkRequestedAction()" << endl;
  if (pushButton.stateChanged())
  {
    //Serial << "  state changed to " << pushButton.isPressed() << endl;
    // has switch been released ?
    if (!pushButton.isPressed())
    {

      // how long was it pressed for ?
      unsigned long press_time = pushButton.getLastStateDuration();
      //Serial << "  button released, pressed for " << pushButton.getLastStateDuration() << endl;

      // long hold > 6 secs
      if (press_time > SW_TR_HOLD)
      {
        //Serial << "  long press - change mode" << endl;
        return CHANGE_MODE;
      }

      // short 1-2 secs
      if (press_time >= 1000 && press_time < 2000)
      {
        //Serial << "  medium press - renegotiate" << endl;
        return RENEGOTIATE;
      }

      // very short < 0.5 sec
      if (press_time < 500)
      {
        //Serial << "  short press - enumeration" << endl;
        return ENUMERATION;
      }

    } else {
      // do any switch release processing here
    }
  }
  return NONE;
}

bool LEDUserInterface::isButtonPressedForReset(VlcbModeParams mode)
{
  indicateResetting();

  // wait for button press for (5 sec) button press -- as a 'safety' mechanism
  while (true)
  {
    run();

    if (!pushButton.isPressed())
    {
      // Button release early
      break;
    }
      
    if (pushButton.getCurrentStateDuration() > SW_TR_HOLD)
    {
      // Button held down long enough
      // DEBUG_SERIAL << F("> performing module reset ...") <<  endl;
      return true;
    }
  }

  // DEBUG_SERIAL << F("> button released early, reset not performed") << endl;
  indicateMode(mode);
  return false;
}

}