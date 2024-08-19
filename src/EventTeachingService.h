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

class EventTeachingService : public Service 
{
public:
  virtual void process(const Action * action) override;

  virtual VlcbServiceTypes getServiceID() override { return SERVICE_ID_OLD_TEACH; }
  virtual byte getServiceVersionID() override { return 1; }

  void enableLearn();
  void inhibitLearn();

private:
  bool bLearn = false;

  void handleMessage(const VlcbMessage *msg);
  void handleLearnMode(const VlcbMessage *msg);
  void handleLearn(unsigned int nn);
  void handleUnlearnEvent(const VlcbMessage *msg, unsigned int nn, unsigned int en);
  void handleUnlearn(unsigned int nn);
  void handleRequestEventCount(unsigned int nn);
  void handleReadEvents(unsigned int nn);
  void handleReadEventIndex(unsigned int nn, byte eventIndex);
  void handleReadEventVariable(const VlcbMessage *msg, unsigned int nn);
  void handleClearEvents(unsigned int nn);
  void handleGetFreeEventSlots(unsigned int nn);
  void handleLearnEvent(const VlcbMessage *msg, unsigned int nn, unsigned int en);
  void handleLearnEventIndex(const VlcbMessage *msg);
  void handleRequestEventVariable(const VlcbMessage *msg, unsigned int nn, unsigned int en);
};

}  // VLCB
