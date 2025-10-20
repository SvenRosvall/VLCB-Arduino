// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#include "LEDUserInterface.h"
#include "Controller.h"
#include <Streaming.h>

namespace VLCB
{

// These intervals are defined in VLCB Technical Introduction chapter 6 - Module User Interface
const int PULSE_ACTIVITY = 20;
const int PULSE_WORK = 250;

LEDUserInterface::LEDUserInterface(byte greenLedPin, byte yellowLedPin, byte pushButtonPin)
  : greenLed(greenLedPin), yellowLed(yellowLedPin), pushButton(pushButtonPin)
{

}

bool LEDUserInterface::isButtonPressed()
{
  return pushButton.isPressed();
}

void LEDUserInterface::process(const Action *action)
{
  pushButton.run();
  greenLed.run();
  yellowLed.run();

  if (resetRequested())
  {
    //DEBUG_SERIAL << F("> Button is pressed for mode change") << endl;
    indicateMode(MODE_SETUP);
  }
  
  handleAction(action);

  checkRequestedAction();
}

void LEDUserInterface::handleAction(const Action *action)
{
  if (action == nullptr)
  {
    return;
  }

  switch (action->actionType)
  {
    case ACT_INDICATE_ACTIVITY:
      greenLed.pulse(PULSE_ACTIVITY);
      break;
      
    case ACT_INDICATE_WORK:
      greenLed.pulse(PULSE_WORK);
      break;

    case ACT_INDICATE_MODE:
      indicateMode(action->mode);
      break;
      
    default:
      break;
  }
}

bool LEDUserInterface::resetRequested()
{
  //Serial << "UI resetRequested() isPressed=" << pushButton.isPressed() << " currentStateDuration=" << pushButton.getCurrentStateDuration() << endl;
  return pushButton.isPressed() && pushButton.getCurrentStateDuration() > SW_TR_HOLD;
}

void LEDUserInterface::indicateMode(VlcbModeParams mode)
{
  switch (mode)
  {
    case MODE_NORMAL:
      //Serial << F("UI indicateMode Normal") << endl;
      yellowLed.on();
      greenLed.off();
      break;

    case MODE_UNINITIALISED:
      //Serial << F("UI indicateMode Uninitialised") << endl;
      yellowLed.off();
      greenLed.on();
      break;

    case MODE_SETUP:
      //Serial << F("UI indicateMode changing") << endl;
      yellowLed.blink();
      greenLed.off();
      break;

    default:
      break;
  }
}

void LEDUserInterface::checkRequestedAction()
{
  //Serial << F("UI> checkRequestedAction()") << endl;
  if (pushButton.stateChanged())
  {
    //Serial << F("  button state changed to ") << pushButton.isPressed() << endl; Serial.flush();
    // has switch been released ?
    if (!pushButton.isPressed())
    {

      // how long was it pressed for ?
      unsigned long press_time = pushButton.getLastStateDuration();
      //Serial << F("  button released, pressed for ") << pushButton.getLastStateDuration() << endl; Serial.flush();

      // long hold > 4 secs
      if (press_time > SW_TR_HOLD)
      {
        //Serial << F("  long press - change mode") << endl;
        controller->putAction(ACT_CHANGE_MODE);
        return;
      }

      // short 1-2 secs
      if (press_time >= 1000 && press_time < 2000)
      {
        //Serial << F("  medium press - renegotiate") << endl;
        controller->putAction(ACT_RENEGOTIATE);
        return;
      }

      // very short < 0.5 sec
      if (press_time < 500)
      {
        //Serial << F("  short press - enumeration") << endl;
        controller->putAction(ACT_START_CAN_ENUMERATION);
        return;
      }

    } else {
      // do any switch release processing here
    }
  }
}

}