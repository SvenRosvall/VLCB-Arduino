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

class EventTeachingService : public Service 
{
public:
  virtual void setController(Controller *cntrl) override;
  virtual Processed handleMessage(unsigned int opc, VlcbMessage *msg) override;
  void setcheckInputProduced(void (*fptr)());
  virtual byte getServiceID() override { return SERVICE_ID_OLD_TEACH; }
  virtual byte getServiceVersionID() override { return 1; }

  void enableLearn();
  void inhibitLearn();

private:
  Controller *controller;
  Configuration *module_config;  // Shortcut to reduce indirection code.
  byte (*checkInputProduced)();

  bool bLearn = false;
  const unsigned long TIME_OUT = 5000; // Time to apply producer input when teaching.

  Processed handleLearnMode(const VlcbMessage *msg);
  Processed handleLearn(unsigned int nn);
  Processed handleUnlearnEvent(const VlcbMessage *msg, unsigned int nn, unsigned int en);
  Processed handleUnlearn(unsigned int nn);
  Processed handleRequestEventCount(unsigned int nn);
  Processed handleReadEvents(VlcbMessage *msg, unsigned int nn);
  Processed handleReadEventIndex(VlcbMessage *msg, unsigned int nn);
  Processed handleReadEventVariable(const VlcbMessage *msg, unsigned int nn);
  Processed handleClearEvents(unsigned int nn);
  Processed handleGetFreeEventSlots(unsigned int nn);
  Processed handleLearnEvent(VlcbMessage *msg, unsigned int nn, unsigned int en);
  Processed handleLearnEventIndex(VlcbMessage *msg);
  Processed handleRequestEventVariable(const VlcbMessage *msg, unsigned int nn, unsigned int en);
};

}  // VLCB
