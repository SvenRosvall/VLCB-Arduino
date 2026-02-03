// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#pragma once

#include <Arduino.h>
#include <vlcbdefs.hpp>

namespace VLCB
{

class Controller;
struct Action;

class Service
{
protected:
  bool isThisNodeNumber(unsigned int nn);

  Controller * controller;

public:
  void setController(Controller * ctrl) { this->controller = ctrl; }
  virtual void begin() {}
  virtual VlcbServiceTypes getServiceID() const = 0;
  virtual byte getServiceVersionID() const = 0;

  virtual void process(const Action * action) = 0;

  virtual void reportDiagnostics(byte serviceIndex, byte diagnosticsCode);
  virtual void reportAllDiagnostics(byte serviceIndex);

  struct Data { byte data1, data2, data3; };
  virtual Data getServiceData();
};

}
