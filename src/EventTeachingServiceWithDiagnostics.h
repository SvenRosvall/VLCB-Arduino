// Copyright (C) Martin Da Costa 2024 (martindc.merg@gmail.com)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#pragma once

#include "EventTeachingService.h"
#include <vlcbdefs.hpp>

namespace VLCB {

class Configuration;
struct VlcbMessage;

class EventTeachingServiceWithDiagnostics : public EventTeachingService 
{
public:
  virtual void reportDiagnostics(byte serviceIndex, byte diagnosticsCode) override;
  virtual void reportAllDiagnostics(byte serviceIndex) override;

};

}  // VLCB
