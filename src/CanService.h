//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

#pragma once

#include "Service.h"
#include "CanTransport.h"
#include <vlcbdefs.hpp>

namespace VLCB
{

struct VlcbMessage;

/// @brief Service for sending and receiving messages on a CAN bus
/// 
/// Delegates to a CanTransport object to do the actual transmission on the CAN bus.
/// @cond LIBRARY
/// The CanService class manages the CANID which can be explicitly set or dynamically
/// allocated via the CANID enumeration process.
/// 
/// It also puts incoming CANFrame's on to the ActionQueue as a VlcbMessage,
/// and gets outgoing VlcbMessage from the ActionQueue and passes it to the CanTransport 
/// object for transmission on the CAN bus.
/// @endcond 
class CanService : public Service
{

public:
  /// Construct a CanService with a concrete CanTransport object to use for 
  /// transmission on the CAN bus.
  CanService(CanTransport * tpt) : canTransport(tpt) {}

  /// @cond LIBRARY
  virtual VlcbServiceTypes getServiceID() const override { return SERVICE_ID_CAN; }
  virtual byte getServiceVersionID() const override { return 2; }
  virtual Data getServiceData();

  virtual void process(const Action * action) override;

protected:
  CanTransport * canTransport;
  /// @endcond 

private:
  void handleCanServiceMessage(const VlcbMessage *msg);
  void handleEnumeration(unsigned int nn);
  void handleSetCANID(const VlcbMessage *msg, unsigned int nn);

  bool sendMessage(const VlcbMessage *msg);
  bool sendRtrFrame();
  bool sendEmptyFrame(bool rtr = false);
  bool sendCanFrame(CANFrame *msg) { return canTransport->sendCanFrame(msg); }
  void startCANenumeration(bool fromENUM = false);

  void checkIncomingCanFrame();
  void checkCANenumTimout();
  byte findFreeCanId();

  bool enumeration_required = false;
  bool bCANenum = false;
  bool startedFromEnumMessage = false;
  unsigned long CANenumTime;
  byte enum_responses[16];     // 128 bits for storing CAN ID enumeration results
};

}
