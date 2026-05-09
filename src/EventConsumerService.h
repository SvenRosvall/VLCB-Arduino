// Copyright (C) Martin Da Costa 2023 (martindc.merg@gmail.com)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#pragma once

#include "Service.h"
#include <vlcbdefs.hpp>

namespace VLCB {

struct VlcbMessage;
/// @brief Service for consuming events.
/// 
/// # Event Consumer Service API
///
/// This service provides a means of filtering received VLCB events 
/// and passing them to an event handler. A full description of events
/// can be found in the document `VLCB Event Model`.
/// The events that a module can handle are held in the Events Table.
/// Specific events are identified by their table index number.
///
class EventConsumerService : public Service 
{
public:
  /// Sets the callback function that is called when an event
  /// opcode that matches an Event Table entry is received.
  void setEventHandler(void (*fptr)(byte index, const VlcbMessage *msg));
  /// @cond LIBRARY
  virtual void process(const Action * action) override;

  virtual VlcbServiceTypes getServiceID() const override 
  {
    return SERVICE_ID_CONSUMER;
  }
  virtual byte getServiceVersionID() const override 
  {
    return 1;
  }

private:
  void (*eventhandler)(byte index, const VlcbMessage *msg) = nullptr;
  void handleConsumedMessage(const VlcbMessage *msg);
  void processAccessoryEvent(const VlcbMessage *msg, unsigned int nn, unsigned int en);  
  
protected:
  unsigned int diagEventsConsumed = 0;
  unsigned int diagEventsAcknowledged = 0;
/// @endcond
};

}  // VLCB
