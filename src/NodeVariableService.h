//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

#pragma once

#include "Service.h"
#include <vlcbdefs.hpp>

namespace VLCB
{

struct VlcbMessage;

/// @brief Service for managing node variables
/// 
/// This service allows node variables to be updated through
/// a configuration utility such as FCU or MMC.
/// 
/// There are no user functions in this service.
class NodeVariableService : public Service
{
public:
  /// @cond LIBRARY
  virtual VlcbServiceTypes getServiceID() const override { return SERVICE_ID_NV; }
  virtual byte getServiceVersionID() const override { return 1; }
  virtual void process(const Action * action) override;
  virtual Data getServiceData() override;
  /// @endcond 

private:
  void handleMessage(const VlcbMessage *msg);
  void handleReadNV(const VlcbMessage *msg, unsigned int nn);
  void handleSetNV(const VlcbMessage *msg, unsigned int nn);
  void handleSetAndReadNV(const VlcbMessage *msg, unsigned int nn);
  
};

}
