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
/// @brief User interface "service" that communicates with LEDs and a push button.
/// 
/// This service manages the green and yellow LEDs and mode push button on
/// a VLCB module. The LEDs indicate current mode and any activity. The push
/// button allows the user to change mode. 
/// See `VLCB Technical Introduction` document for details about how the LEDs
/// and push button are used.
class LEDUserInterface : public Service
{
public:
  LEDUserInterface(byte greenLedPin, byte yellowLedPin, byte pushButtonPin);

  /// @cond LIBRARY
  virtual VlcbServiceTypes getServiceID() const override { return SERVICE_ID_NONE; };
  virtual byte getServiceVersionID() const override { return 1; };

  bool isButtonPressed();
  virtual void process(const Action *action) override;
  /// @endcond 

private:
  LED greenLed;
  LED yellowLed;
  Switch pushButton;

  bool resetRequested();
  void handleAction(const Action *action);
  void checkRequestedAction();

  void indicateMode(VlcbModeParams mode);
};

}