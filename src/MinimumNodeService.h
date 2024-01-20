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

  virtual void setController(Controller *cntrl) override;
  virtual void process(const Command *cmd) override; 

  virtual byte getServiceID() override { return SERVICE_ID_MNS; }
  virtual byte getServiceVersionID() override { return 1; }
  
  virtual void begin() override;
  
  // backdoors for testing
  void setHeartBeat(bool f) { noHeartbeat = !f; }
  void setSetupMode();
  void setUninitialised();
  void setNormal();

private:

  Controller *controller;
  Configuration * module_config;  // Shortcut to reduce indirection code.

  bool requestingNewNN = false;
  unsigned long timeOutTimer;
  VlcbModeParams instantMode;
  
  void checkModeChangeTimeout();
  void initSetup();
  void initSetupFromNormal();

  void heartbeat();
  
  unsigned long lastHeartbeat = 0;
  byte heartbeatSequence = 0;
  bool noHeartbeat = false;
  unsigned int heartRate = 5000;

  void handleMessage(const VlcbMessage *msg);
  void handleRequestNodeParameters();
  void handleRequestNodeParameter(const VlcbMessage *msg, unsigned int nn);
  void handleSetNodeNumber(const VlcbMessage *msg, unsigned int nn);
  void handleRequestServiceDefinitions(const VlcbMessage *msg, unsigned int nn);
  void handleRequestDiagnostics(const VlcbMessage *msg, unsigned int nn);
  void handleModeMessage(const VlcbMessage *msg, unsigned int nn);
};

}