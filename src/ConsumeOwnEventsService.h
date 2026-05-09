// Copyright (C) Martin Da Costa 2023 (martindc.merg@gmail.com)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#pragma once

#include "Service.h"
#include <vlcbdefs.hpp>

namespace VLCB {

/// @brief Service for consuming events that this node has produced.
/// 
/// # Consue Own Events Service API
/// 
/// This service is used when a user node wants to act on a produced
/// event as if it was an incoming event. 
class ConsumeOwnEventsService : public Service
{
public:
  /// @cond LIBRARY
  virtual VlcbServiceTypes getServiceID() const override
  {
    return SERVICE_ID_CONSUME_OWN_EVENTS;
  }
  
  virtual byte getServiceVersionID() const override
  {
    return 1;
  }

  virtual void process(const Action * action) override
  {}
  /// @endcond 
};

}  // VLCB
