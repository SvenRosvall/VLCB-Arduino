#pragma once

#include "CBUSLED.h"
#include "CBUSswitch.h"
#include "UserInterface.h"

#define SW_TR_HOLD 6000U                   // CBUS push button hold time for SLiM/FLiM transition in millis = 6 seconds

class LEDUserInterface : public UserInterface
{
public:
  LEDUserInterface(byte greenLedPin, byte yellowLedPin, byte pushButtonPin);

  bool isButtonPressed();
  virtual void run() override;
  virtual void indicateResetting() override;
  virtual void indicateResetDone() override;
  virtual bool resetRequested() override;
  virtual void indicateActivity() override;
  virtual void indicateMode(byte i) override;
  virtual RequestedAction checkRequestedAction()override;

private:
  CBUSLED greenLed;
  CBUSLED yellowLed;
  CBUSSwitch pushButton;

};
