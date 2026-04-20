// Copyright (C) Martin Da Costa 2023 (martindc.merg@gmail.com)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#pragma once

#include "Service.h"
#include <vlcbdefs.hpp>

namespace VLCB {

struct VlcbMessage;
/// # Producer Service API
///
/// This service provides a means of generating VLCB events and
/// interrogating their status. A full description of events can
/// be found in the document `VLCB Event Model`.
/// The events that a module can produce are held in the Events Table.
/// Specific events are identified by their table index number.
///
class EventProducerService : public Service {
public:
  /// Sets the callback function that is called when an Accessory Request
  /// opcode or Accessory Request Short Event opcode is received.
  void setRequestEventHandler(void (*fptr)(byte index, const VlcbMessage *msg));
/// \cond LIBRARY
  virtual void process(const Action * action) override;

  virtual VlcbServiceTypes getServiceID() const override
  {
    return SERVICE_ID_PRODUCER;
  }
  virtual byte getServiceVersionID() const override
  {
    return 1;
  }
/// \endcond

  /// Causes an event to be sent with `state` indicating `on` for TRUE
  /// and `off` for FALSE. Short or Long event is determined by the
  /// Event Table entry at `evIndex`.
  void sendEventAtIndex(bool state, byte evIndex);
  
  /// Causes an event with one data byte to be sent with `state` indicating `on` for TRUE
  /// and `off` for FALSE. Short or Long event is determined by the
  /// Event Table entry at `evIndex`. The data sent is `data1`.
  void sendEventAtIndex(bool state, byte evIndex, byte data1);
  
  /// Causes an event with two data bytes to be sent with `state` indicating `on` for TRUE
  /// and `off` for FALSE. Short or Long event is determined by the
  /// Event Table entry at `evIndex`. The data sent is `data1` and `data2`.
  void sendEventAtIndex(bool state, byte evIndex, byte data1, byte data2);
  
  /// Causes an event with three data bytes to be sent with `state` indicating `on` for TRUE
  /// and `off` for FALSE. Short or Long event is determined by the
  /// Event Table entry at `evIndex`. The data sent is `data1` and `data2` and `data3`.
  void sendEventAtIndex(bool state, byte evIndex, byte data1, byte data2, byte data3);
  
  /// Causes an Accessory Response to be sent with `state` indicating `on` for TRUE
  /// and `off` for FALSE. Short or Long event is determined by the nature of the 
  /// request and the Event Table entry at `evIndex`.
  void sendEventResponse(bool state, byte index);
  
  /// Causes an Accessory Response with one data byte to be sent with `state` indicating
  /// `on` for TRUE and `off` for FALSE. Short or Long event is determined by the nature
  /// of the request and the Event Table entry at `evIndex`. The data sent is `data1`.
  void sendEventResponse(bool state, byte index, byte data1);
  
  /// Causes an Accessory Response with two data bytes to be sent with `state` indicating
  /// `on` for TRUE and `off` for FALSE. Short or Long event is determined by the nature
  /// of the request and the Event Table entry at `evIndex`. The data sent is `data1` and `data2`.
  void sendEventResponse(bool state, byte index, byte data1, byte data2);
  
  /// Causes an Accessory Response with three data bytes to be sent with `state` indicating
  /// `on` for TRUE and `off` for FALSE. Short or Long event is determined by the nature
  /// of the request and the Event Table entry at `evIndex`. The data sent is `data1` and
  /// `data2` and `data3`.
  void sendEventResponse(bool state, byte index, byte data1, byte data2, byte data3);
  

private:
  void (*requesteventhandler)(byte index, const VlcbMessage *msg);
  void handleProdSvcMessage(const VlcbMessage *msg);

  void sendMessage(VlcbMessage &msg, byte opCode, const byte *nn_en);
/// \cond LIBRARY
protected:
  unsigned int diagEventsProduced = 0;
/// \endcond
};

}  // VLCB
