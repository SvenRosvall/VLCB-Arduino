#include "LEDUserInterface.h"
#include "CBUS.h"
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
}

void LEDUserInterface::indicateResetting()
{
  pushButton.reset();
  greenLed.blink();
  yellowLed.blink();
}

void LEDUserInterface::indicateResetDone()
{
  Serial << "UI indicateResetDone()" << endl;
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

void LEDUserInterface::indicateMode(byte mode) {
  switch (mode) {

    case MODE_FLIM:
      //Serial << "UI indicateMode FLiM" << endl;
      yellowLed.on();
      greenLed.off();
      break;

    case MODE_SLIM:
      //Serial << "UI indicateMode SLiM" << endl;
      yellowLed.off();
      greenLed.on();
      break;

    case MODE_CHANGING:
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
    if (!pushButton.isPressed()) {

      // how long was it pressed for ?
      unsigned long press_time = pushButton.getLastStateDuration();
      //Serial << "  button released, pressed for " << pushButton.getLastStateDuration() << endl;

      // long hold > 6 secs
      if (press_time > SW_TR_HOLD) {
        //Serial << "  long press - change mode" << endl;
        return CHANGE_MODE;
      }

      // short 1-2 secs
      if (press_time >= 1000 && press_time < 2000) {
        //Serial << "  medium press - renegotiate" << endl;
        return RENEGOTIATE;
      }

      // very short < 0.5 sec
      if (press_time < 500) {
        //Serial << "  short press - enumeration" << endl;
        return ENUMERATION;
      }

    } else {
      // do any switch release processing here
    }
  }
  return NONE;
}

}