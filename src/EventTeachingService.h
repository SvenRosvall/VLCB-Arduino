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
  virtual Processed handleMessage(unsigned int opc, CANFrame *msg) override;

  virtual byte getServiceID() override { return SERVICE_ID_OLD_TEACH; }
  virtual byte getServiceVersionID() override { return 1; }

  void enableLearn();
  void inhibitLearn();

private:
  Controller *controller;
  Configuration *module_config;  // Shortcut to reduce indirection code.

  bool bLearn = false;

  Processed handleLearnMode(const CANFrame *msg);
  Processed handleLearn(unsigned int nn);
  Processed handleUnlearnEvent(const CANFrame *msg, unsigned int nn, unsigned int en);
  Processed handleUnlearn(unsigned int nn);
  Processed handleRequestEventCount(unsigned int nn);
  Processed handleReadEvents(CANFrame *msg, unsigned int nn);
  Processed handleReadEventIndex(CANFrame *msg, unsigned int nn);
  Processed handleReadEventVariable(const CANFrame *msg, unsigned int nn);
  Processed handleClearEvents(unsigned int nn);
  Processed handleGetFreeEventSlots(unsigned int nn);
  Processed handleLearnEvent(CANFrame *msg, unsigned int nn, unsigned int en);
  Processed handleLearnEventIndex(CANFrame *msg);
  Processed handleRequestEventVariable(const CANFrame *msg, unsigned int nn, unsigned int en);
};

}  // VLCB
