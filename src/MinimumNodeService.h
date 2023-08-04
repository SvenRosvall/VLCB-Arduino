// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#pragma once

#include "Service.h"

namespace VLCB
{

class Configuration;

class MinimumNodeService : public Service
{

public:

  virtual void setController(Controller *cntrl) override;
  Processed handleMessage(unsigned int opc, CANFrame *msg) override;

private:

  Controller *controller;
  Configuration * module_config;  // Shortcut to reduce indirection code.
};

}