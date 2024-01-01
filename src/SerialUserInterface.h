// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#pragma once

#include "Service.h"
#include "Configuration.h"
#include "Transport.h"

namespace VLCB
{

class SerialUserInterface : public Service
{
public:
  SerialUserInterface(Transport *transport);
  virtual void setController(Controller *ctrl) override;
  virtual byte getServiceID() override { return 98; };
  virtual byte getServiceVersionID() override { return 1; };

  virtual void process(const Command *cmd) override;

private:
  Controller * controller;
  Configuration * modconfig;
  Transport * transport;
  bool isResetRequested = false;

  void handleCommand(const Command *cmd);
  void processSerialInput();
  void indicateMode(VlcbModeParams i);
};

}