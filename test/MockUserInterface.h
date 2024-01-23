//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0
//
//

#pragma once

#include "Service.h"
#include "Configuration.h"
#include "vlcbdefs.hpp"

class MockUserInterface : public VLCB::Service
{
public:
  virtual void process(const VLCB::Command *cmd) override;
  virtual byte getServiceID() override { return 255; };
  virtual byte getServiceVersionID() override { return 1; };
  
  VlcbModeParams getIndicatedMode();

private:
  VlcbModeParams indicatedMode = MODE_UNINITIALISED;
};
