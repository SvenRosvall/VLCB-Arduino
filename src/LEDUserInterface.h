#pragma once

#include "CBUSLED.h"
#include "CBUSswitch.h"
#include "UserInterface.h"

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
