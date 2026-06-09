//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0
//
//


#pragma once

#include "Service.h"

namespace VLCB
{

/// @brief A dummy service that reports internal metrics as service diagnostics.
/// 
/// Use this service to share internal metrics as diagnostics that can be viewed in
/// configuration utilities.
/// Supported diagnostics are:
/// 1) Available memory
/// 2) ActionQueue current size
/// 3) ActionQueue high water mark
/// 4) ActionQueue number of overflows
class InternalDiagnosticsService : public Service
{
public:
  VlcbServiceTypes getServiceID() const override { return static_cast<VlcbServiceTypes>(240); }
  byte getServiceVersionID() const override { return 1; }

  void reportDiagnostics(byte serviceIndex, byte diagnosticsCode) override;
  void reportAllDiagnostics(byte serviceIndex) override;
  virtual int getDiagnosticCount() override;
};

}