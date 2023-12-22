// Copyright (C) Martin Da Costa 2023 (martindc.merg@gmail.com)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#pragma once

#include "Service.h"
#include <vlcbdefs.hpp>

namespace VLCB {

class Configuration;

class EventProducerService : public Service {
public:
  virtual void setController(Controller *cntrl) override;
  virtual void process(const Command * cmd) override;

  virtual byte getServiceID() override
  {
    return SERVICE_ID_PRODUCER;
  }
  virtual byte getServiceVersionID() override
  {
    return 1;
  }
  void begin() override;
  void sendEvent(bool state, byte evValue);
  void sendEvent(bool state, byte evValue, byte data1);
  void sendEvent(bool state, byte evValue, byte data1, byte data2);
  void sendEvent(bool state, byte evValue, byte data1, byte data2, byte data3);

private:
  Controller *controller;
  Configuration *module_config;  // Shortcut to reduce indirection code.
  void (*eventhandler)(byte index, const VlcbMessage *msg);

  void handleProdSvcMessage(const VlcbMessage *msg);
  void setProducedEvents();
  byte createDefaultEvent(byte evValue);
  void findOrCreateEventByEv(byte evIndex, byte evValue, byte tarr[]);
  void sendMessage(VlcbMessage &msg, byte opCode, const byte *nn_en);

  bool uninit = false;
};

}  // VLCB
