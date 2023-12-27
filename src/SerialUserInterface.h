// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#pragma once

#include "UserInterface.h"
#include "Configuration.h"
#include "Transport.h"
#include "Controller.h"

namespace VLCB
{

class SerialUserInterface : public UserInterface
{
public:
  SerialUserInterface(Configuration * modconfig, Transport * transport);

  virtual void process(const Command *cmd) override;
  virtual void indicateResetting() override;
  virtual void indicateResetDone() override;
  virtual bool resetRequested() override;

private:
  Configuration * modconfig;
  Transport * transport;
  bool isResetRequested = false;

  void handleCommand(const Command *cmd);
  void processSerialInput();
  void indicateMode(VlcbModeParams i);
};

}