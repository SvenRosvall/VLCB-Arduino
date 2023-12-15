//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

#pragma once

#include "Service.h"
#include "UserInterface.h"
#include "CanTransport.h"
#include <vlcbdefs.hpp>

namespace VLCB
{

class Configuration;

class CanService : public Service
{

public:
  CanService(CanTransport * tpt) : canTransport(tpt) {}

  virtual void setController(Controller *cntrl) override;
  virtual byte getServiceID() override { return SERVICE_ID_CAN; }
  virtual byte getServiceVersionID() override { return 1; }

  virtual void process(const Command * cmd) override;
  virtual void process(UserInterface::RequestedAction requestedAction) override;

  void startCANenumeration(bool fromENUM = false);

private:

  Controller *controller;
  Configuration * module_config;  // Shortcut to reduce indirection code.
  CanTransport * canTransport;

  void handleCanServiceMessage(const VlcbMessage *msg);
  void handleEnumeration(unsigned int nn);
  void handleSetCANID(const VlcbMessage *msg, unsigned int nn);
};

}
