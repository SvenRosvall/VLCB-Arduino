// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#pragma once

#include "Service.h"

namespace VLCB
{

class Configuration;

// Quick and dirty service to migrate from the CBUS library to using a set of VLCB services.
// This is a service that implements all the CBUS op-codes.
// When creating new services, pick code from here.
// In the end there should not be any code left in this service.
class CbusService : public Service
{
public:
  virtual void setController(Controller *cntrl) override;
  virtual Processed handleMessage(unsigned int opc, CANFrame *msg) override;

  virtual byte getServiceID() override { return 91; }
  virtual byte getServiceVersionID() override { return 1; }

private:
  Controller * controller;
  Configuration * module_config;  // Shortcut to reduce indirection code.
 
 };

} // VLCB
