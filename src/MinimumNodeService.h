// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#pragma once

#include "Service.h"
#include <vlcbdefs.hpp>

namespace VLCB
{

class Configuration;
struct VlcbMessage;

class MinimumNodeService : public Service
{

public:

  virtual void process(const Action *action) override; 

  virtual VlcbServiceTypes getServiceID() override { return SERVICE_ID_MNS; }
  virtual byte getServiceVersionID() override { return 1; }
  
  virtual void begin() override;
  
  // backdoors for testing
  void setHeartBeat(bool f) { noHeartbeat = !f; }
  void setSetupMode();
  void setUninitialised();
  void setNormal(unsigned int nn);

private:

  VlcbModeParams instantMode;
  
  void initSetupFromUninitialised();
  void initSetupFromNormal();
  void initSetupCommon();

  void heartbeat();
  
  unsigned long lastHeartbeat = 0;
  byte heartbeatSequence = 0;
  bool noHeartbeat = false;
  unsigned int heartRate = 5000;

  void handleRequestNodeParameters();
  void handleRequestNodeParameter(const VlcbMessage *msg, unsigned int nn);
  void handleSetNodeNumber(const VlcbMessage *msg, unsigned int nn);
  void handleRequestServiceDefinitions(const VlcbMessage *msg, unsigned int nn);
  void handleModeMessage(const VlcbMessage *msg, unsigned int nn);

protected:
  virtual void handleMessage(const VlcbMessage *msg);
  unsigned int diagMsgsActed = 0;
  unsigned int diagNodeNumberChanges = 0;
};

}