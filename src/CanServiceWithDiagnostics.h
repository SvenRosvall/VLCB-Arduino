//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

#pragma once

#include "CanService.h"

namespace VLCB
{

class CanServiceWithDiagnostics : public CanService
{
public:
  CanServiceWithDiagnostics(CanTransport * tpt) : CanService(tpt) {}

  virtual void reportDiagnostics(byte serviceIndex, byte diagnosticsCode) override;
  virtual void reportAllDiagnostics(byte serviceIndex) override;
};

} // VLCB
