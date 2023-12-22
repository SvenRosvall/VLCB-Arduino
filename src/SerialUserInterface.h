// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#pragma once

#include "UserInterface.h"
#include "Configuration.h"
#include "Transport.h"

namespace VLCB
{

class SerialUserInterface : public UserInterface
{
public:
  SerialUserInterface(Configuration * modconfig, Transport * transport);

  virtual void run() override;
  virtual void indicateResetting() override;
  virtual void indicateResetDone() override;
  virtual bool resetRequested() override;
  virtual void indicateActivity() override;
  virtual void indicateMode(byte i) override;

private:
  Configuration * modconfig;
  Transport * transport;
  bool isResetRequested = false;
};

}