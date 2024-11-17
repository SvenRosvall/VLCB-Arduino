// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#pragma once

#include "LED.h"
#include "Switch.h"
#include "Service.h"
#include "vlcbdefs.hpp"

namespace VLCB
{

const unsigned int SW_TR_HOLD = 4000U;  // Controller push button hold time for SLiM/FLiM transition in millis = 4 seconds

class LEDUserInterface : public Service
{
public:
  LEDUserInterface(byte greenLedPin, byte yellowLedPin, byte pushButtonPin);

  virtual VlcbServiceTypes getServiceID() override { return SERVICE_ID_HIDDEN; };
  virtual byte getServiceVersionID() override { return 1; };

  bool isButtonPressed();
  virtual void process(const Action *action) override;

private:
  LED greenLed;
  LED yellowLed;
  Switch pushButton;

  bool resetRequested();
  void handleAction(const Action *action);
  void checkRequestedAction();
  void indicateActivity();
  void indicateMode(VlcbModeParams mode);
};

}