// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#pragma once

#include "LED.h"
#include "Switch.h"
#include "UserInterface.h"
#include "vlcbdefs.hpp"

namespace VLCB
{

const unsigned int SW_TR_HOLD = 6000U;  // Controller push button hold time for SLiM/FLiM transition in millis = 6 seconds

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
  virtual void indicateMode(VlcbModeParams i) override;
  virtual RequestedAction checkRequestedAction()override;

private:
  LED greenLed;
  LED yellowLed;
  Switch pushButton;

};

}