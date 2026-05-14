// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#pragma once

#include "Service.h"
#include <vlcbdefs.hpp>

namespace VLCB
{

struct VlcbMessage;
/// @brief Mandatory Service for node
///
/// # Minimum Node Service API
///
/// This service is mandatory within the VLCB structure. It manages the setting up 
/// of a node by handling all of the communication with the external management system.
/// The MNS will also act upon various Mode Commands either setting flags or instigating
/// operations in other services, if present. In normal sketch use, there are no
/// application calls directly to the MNS.
///
class MinimumNodeService : public Service
{

public:
  /// @cond LIBRARY
  virtual void process() override; 
  virtual void process(const Action &action) override; 

  virtual VlcbServiceTypes getServiceID() const override { return SERVICE_ID_MNS; }
  virtual byte getServiceVersionID() const override { return 1; }
  
  virtual void begin() override;

  ///@name Backdoors for testing
  /// Access to these functions is provided purely for the purpose of testing VLCB by forcing
  /// specific modes.
  /// Under no circumstances should these functions be called from a normal application sketch.
  ///@{
  /// Forces Heartbeat on or off for testing purposes only.
  void setHeartBeat(bool f) { noHeartbeat = !f; }
  /// Forces instantMode to SETUP_MODE for testing purposes only.
  void setSetupMode();
  /// Called when ACT_CHANGE_MODE is seen on the Action bus and the current mode status is
  /// MODE_SETUP or MODE_NORMAL. It will cause the mode to revert to MODE_UNINITIALISED.
  void setUninitialised();
  /// Called when OPC_SNN is received. It will set MODE_NORMAL and the node number to nn. 
  void setNormal(unsigned int nn);
  ///@}
private:

  VlcbModeParams instantMode;
  /// Called when ACT_CHANGE_MODE is seen on the Action bus and the current status is
  /// MODE_UNINITIALISED. It will start a CAN enumeration before calling initSetupCommon().
  void initSetupFromUninitialised();
  /// Called when ACT_RENEGOTIATE is seen on the Action bus and the current status is
  /// MODE_NORMAL. It will call initSetupCommon().
  void initSetupFromNormal();
  /// Initiates a transistion to NORMAL mode by sending OPC_RQNN
  void initSetupCommon();
  /// Called by process(). If status is MODE_NORMAL and heartbeat is enabled and setup
  /// is not in progress, it will cause OPC_HEARTB to be sent at a frequency determined
  /// by heartRate.
  void heartbeat();
  
  unsigned long lastHeartbeat = 0;
  byte heartbeatSequence = 0;
  bool noHeartbeat = false;
  unsigned int heartRate = 5000;
  bool notFcuCompatible = false;  //Compatible is default

  /// Gets and sends node parameters when ACT_MESSAGE_IN is OPC_RQNP and the node is in
  /// MODE_SETUP.
  void handleRequestNodeParameters();
  /// Gets and sends node parameters when ACT_MESSAGE_IN is OPC_RQNP and the node is in
  /// MODE_NORMAL.
  void handleRequestNodeParameter(const VlcbMessage *msg, unsigned int nn);
  /// Sets the Node Number according to the content of the VLCB message in ACT_MESSAGE_IN
  /// with OPC_SNN. This is part of the initialisation sequence.
  void handleSetNodeNumber(const VlcbMessage *msg, unsigned int nn);
  /// Gets and sends service data according to the content of the VLCB message in ACT_MESSAGE_IN
  /// with OPC_RQSD.
  void handleRequestServiceDefinitions(const VlcbMessage *msg, unsigned int nn);
  /// Sets the MODE according to the content of the VLCB message in ACT_MESSAGE_IN
  /// with OPC_MODE.
  void handleModeMessage(const VlcbMessage *msg, unsigned int nn);
  
protected:
  /// Called when ACT_MESSAGE_IN is seen in the Action queue. If the VLCB message is of
  /// relevance to the MNS, it will act upon it before removing it from the Action queue
  /// either directly or in a called function.
  virtual void handleMessage(const VlcbMessage *msg);
  /// Called whenever the node number is changed.  Count is recorded and held in
  /// Minimum Node Service With Diagnostics.
  virtual void diagNodeNumberChanged() {};

  /// @endcond
};
}