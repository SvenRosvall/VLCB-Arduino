// Copyright (C) Martin Da Costa 2023 (martindc.merg@gmail.com)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#pragma once

#include "Service.h"
#include "ConsumeOwnEventsService.h"
#include <vlcbdefs.hpp>

namespace VLCB {

class Configuration;

class EventProducerService : public Service {
public:
  EventProducerService(ConsumeOwnEventsService *s = nullptr) : coeService(s) {}
  virtual void setController(Controller *cntrl) override;
  virtual void process(UserInterface::RequestedAction requestedAction) override; 
  virtual Processed handleMessage(unsigned int opc, VlcbMessage *msg) override;

  virtual byte getServiceID() override 
  {
    return SERVICE_ID_PRODUCER;
  }
  virtual byte getServiceVersionID() override 
  {
    return 1;
  }
  void begin();
  void sendEvent(bool state, byte index);
  void sendEvent(bool state, byte index, byte data1);
  void sendEvent(bool state, byte index, byte data1, byte data2);
  void sendEvent(bool state, byte index, byte data1, byte data2, byte data3);

private:
  Controller *controller;
  Configuration *module_config;  // Shortcut to reduce indirection code.
  ConsumeOwnEventsService *coeService;
  void (*eventhandler)(byte index, VlcbMessage *msg);
 
  void setProducedEvents();
  bool uninit = false;
};

}  // VLCB
