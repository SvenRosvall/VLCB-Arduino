//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

#pragma once

#include "Service.h"
#include <vlcbdefs.hpp>

namespace VLCB
{

class Configuration;

class NodeVariableService : public Service
{

public:

  virtual void setController(Controller *cntrl) override;
  virtual byte getServiceID() override { return SERVICE_ID_NV; }
  virtual byte getServiceVersionID() override { return 1; }

  virtual Processed handleMessage(unsigned int opc, VlcbMessage *msg) override;

private:

  Controller *controller;
  Configuration * module_config;  // Shortcut to reduce indirection code.

  Processed handleReadNV(const VlcbMessage *msg, unsigned int nn);
  Processed handleSetNV(const VlcbMessage *msg, unsigned int nn);
  Processed handleSetAndReadNV(const VlcbMessage *msg, unsigned int nn);
};

}
