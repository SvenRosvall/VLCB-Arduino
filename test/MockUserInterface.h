//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0
//
//

#pragma once

#include "UserInterface.h"

class MockUserInterface : public VLCB::UserInterface
{
public:
  virtual void run() override;
  virtual void indicateResetting() override;
  virtual void indicateResetDone() override;
  virtual void indicateActivity() override;
  virtual void indicateMode(byte mode) override;
  virtual bool resetRequested() override;
  virtual RequestedAction checkRequestedAction() override;
};
