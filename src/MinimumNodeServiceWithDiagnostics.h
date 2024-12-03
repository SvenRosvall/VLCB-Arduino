// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#pragma once

#include "MinimumNodeService.h"
#include <vlcbdefs.hpp>

namespace VLCB
{

struct VlcbMessage;

class MinimumNodeServiceWithDiagnostics : public MinimumNodeService
{
public:
  virtual void reportDiagnostics(byte serviceIndex, byte diagnosticsCode) override;
  virtual void reportAllDiagnostics(byte serviceIndex) override;

protected:
  virtual void handleMessage(const VlcbMessage *msg) override; 

private:
  void handleRequestDiagnostics(const VlcbMessage *msg, unsigned int nn);
};

}