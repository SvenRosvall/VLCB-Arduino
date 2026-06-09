// Copyright (C) Martin Da Costa 2023 (martindc.merg@gmail.com)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#pragma once

#include "Service.h"
#include <vlcbdefs.hpp>

namespace VLCB {

struct VlcbMessage;

/// @brief Service for producing events.
/// 
/// This service provides a means of generating VLCB events and
/// interrogating their status. A full description of events can
/// be found in the document `VLCB Event Model`.
/// 
/// The events that a module can produce are either taught events
/// which are stored in the events table or direct events.
///
class EventProducerService : public Service {
public:
  /// Sets the callback function that is called when an Accessory Request
  /// opcode or Accessory Request Short Event opcode is received.
  void setRequestEventHandler(void (*fptr)(byte index, const VlcbMessage *msg));
/// @cond LIBRARY
  virtual void processAction(const Action & action) override;

  virtual VlcbServiceTypes getServiceID() const override
  {
    return SERVICE_ID_PRODUCER;
  }
  virtual byte getServiceVersionID() const override
  {
    return 1;
  }
/// @endcond

  /// @brief Send a short event.
  /// 
  /// @param state send an ON event if state is `true` or OFF event if `false`
  /// @param eventNumber event number for the short event.
  /// 
  /// The event does not need to be taught, and stored in the events table.
  void sendShortEvent(bool state, int eventNumber);

  /// @brief Send a long event.
  /// 
  /// @param state send an ON event if state is `true` or OFF event if `false`
  /// @param eventNumber event number for the long event.
  /// 
  /// The event does not need to be taught, and stored in the events table.
  /// The node number is the number of this node.
  void sendLongEvent(bool state, int eventNumber);

  /// @brief Send a long event with a spoofed node number.
  /// 
  /// @param state send an ON event if state is `true` or OFF event if `false`
  /// @param nodeNumber node number to use for the long event instead of the current node.
  /// @param eventNumber event number for the long event.
  /// 
  /// The event does not need to be taught, and stored in the events table.
  /// 
  /// Use spoofed node numbers only when many nodes send the same long event.
  /// It is recommended to use short events for many-to-many events.
  void sendLongEventWithSpoofedNodeNumber(bool state, int nodeNumber, int eventNumber);

  /// @brief Send an event from the taught events table.
  /// 
  /// @param state send an ON event if state is `true` or OFF event if `false`
  /// @param evIndex index into the taught events table for the event to use.
  /// 
  /// The indexed event determines if the sent event is short or long.
  void sendEventAtIndex(bool state, byte evIndex);
  
  /// @brief Send an event from the taught events table with data.
  /// 
  /// @param state send an ON event if state is `true` or OFF event if `false`
  /// @param evIndex index into the taught events table for the event to use.
  /// @param data1 data byte to be included in the event.
  /// 
  /// The indexed event determines if the sent event is short or long.
  void sendEventAtIndex(bool state, byte evIndex, byte data1);

  /// @brief Send an event from the taught events table with data.
  /// 
  /// @param state send an ON event if state is `true` or OFF event if `false`
  /// @param evIndex index into the taught events table for the event to use.
  /// @param data1 first data byte to be included in the event.
  /// @param data2 second data byte to be included in the event.
  /// 
  /// The indexed event determines if the sent event is short or long.
  void sendEventAtIndex(bool state, byte evIndex, byte data1, byte data2);
  
  /// @brief Send an event from the taught events table with data.
  /// 
  /// @param state send an ON event if state is `true` or OFF event if `false`
  /// @param evIndex index into the taught events table for the event to use.
  /// @param data1 first data byte to be included in the event.
  /// @param data2 second data byte to be included in the event.
  /// @param data3 third data byte to be included in the event.
  /// 
  /// The indexed event determines if the sent event is short or long.
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
/// @cond LIBRARY
protected:
  unsigned int diagEventsProduced = 0;
/// @endcond
};

}  // VLCB
