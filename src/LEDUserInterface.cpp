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

void LEDUserInterface::process(const Command *cmd)
{
  pushButton.run();
  greenLed.run();
  yellowLed.run();

  if (resetRequested())
  {
    //DEBUG_SERIAL << F("> Button is pressed for mode change") << endl;
    indicateMode(MODE_SETUP);
  }
  
  handleCommand(cmd);

  checkRequestedAction();
}

void LEDUserInterface::handleCommand(const Command *cmd)
{
  if (cmd == nullptr)
  {
    return;
  }

  switch (cmd->commandType)
  {
    case CMD_INDICATE_ACTIVITY:
      indicateActivity();
      break;

      // TODO: Not yet implemented.
//    case CMD_INDICATE_RESETTING:
//      indicateResetting();
//      break;
//
//    case CMD_INDICATE_RESET_DONE:
//      indicateResetDone();
//      break;

    case CMD_INDICATE_MODE:
      indicateMode(cmd->mode);
      break;
      
    default:
      break;
  }
}

void LEDUserInterface::indicateResetting()
{
  pushButton.reset();
  greenLed.blink();
  yellowLed.blink();
}

void LEDUserInterface::indicateResetDone()
{
  //Serial << F("UI> indicateResetDone()") << endl;
  greenLed.off();
  yellowLed.off();
  greenLed.run();
  yellowLed.run();
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

      // long hold > 6 secs
      if (press_time > SW_TR_HOLD)
      {
        //Serial << F("  long press - change mode") << endl;
        controller->putCommand(CMD_CHANGE_MODE);
        return;
      }

      // short 1-2 secs
      if (press_time >= 1000 && press_time < 2000)
      {
        //Serial << F("  medium press - renegotiate") << endl;
        controller->putCommand(CMD_RENEGOTIATE);
        return;
      }

      // very short < 0.5 sec
      if (press_time < 500)
      {
        //Serial << F("  short press - enumeration") << endl;
        controller->putCommand(CMD_START_CAN_ENUMERATION);
        return;
      }

    } else {
      // do any switch release processing here
    }
  }
}

}