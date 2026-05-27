// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#pragma once

#ifndef DEBUG_SERIAL
#define DEBUG_SERIAL Serial
#endif

#include "initializer_list.h"
#include "ArrayHolder.h"
#include "CircularBuffer.h"
#include "Configuration.h"
#include "TimedResponse.h"

namespace VLCB
{

// TODO: This is a sign something is wrong. Need to be this big to handle 
// large number of generated messages such as when asking for node parameters.
// Each entry uses 10 bytes so a size of 30 uses 300 bytes.
// This will get worse for for example queries for all events.
// Need a mechanism like TimedResponse to create messages slowly so they can
// be sent through the transport without requiring this buffering. 
const int ACTION_QUEUE_SIZE = 30;

//
/// CAN/Controller message type
//
struct VlcbMessage
{
  uint8_t len; // Value 0-7 or FF for messages handled in CanTransport
  uint8_t data[8];
};

// Action type
enum ACTION : byte
{
  ACT_MESSAGE_IN,
  ACT_MESSAGE_OUT,
  ACT_START_CAN_ENUMERATION,
  ACT_CHANGE_MODE,
  ACT_RENEGOTIATE,
  ACT_INDICATE_ACTIVITY,
  ACT_INDICATE_WORK,
  ACT_INDICATE_MODE,
  // ...
};

struct Action
{
  enum ACTION actionType;
  union
  {
    VlcbMessage vlcbMessage; // with ACT_MESSAGE_IN & ACT_MESSAGE_OUT
    bool fromENUM; // with ACT_START_CAN_ENUMERATION
    VlcbModeParams mode; // with ACT_INDICATE_MODE
  };
};

class Service;

//
/// Main object in VLCB. Coordinates transport, ui, configuration and services.
//
class Controller
{
public:
//  Controller();
  Controller(Configuration *conf);
//  Controller(std::initializer_list<Service *> services);
  Controller(Configuration *conf, std::initializer_list<Service *> services);
  
  void setServices(std::initializer_list<Service *> services);

  Configuration * getModuleConfig() const { return module_config; }

  void setName(const char *mname) { module_config->setName(mname); }

  const ArrayHolder<Service *> & getServices() { return services; }

  void updateParamFlags();
  void setParamFlag(VlcbParamFlags flag, bool set);
  Parameters & getParams() { return module_config->getParams(); }
  unsigned char getParam(VlcbParams param) const { return module_config->getParam(param); }

  bool sendMessage(const VlcbMessage *msg);

  void begin();
  inline bool sendMessageWithNN(VlcbOpCodes opc);
  inline bool sendMessageWithNN(VlcbOpCodes opc, byte b1);
  inline bool sendMessageWithNN(VlcbOpCodes opc, byte b1, byte b2);
  inline bool sendMessageWithNN(VlcbOpCodes opc, byte b1, byte b2, byte b3);
  inline bool sendMessageWithNN(VlcbOpCodes opc, byte b1, byte b2, byte b3, byte b4);
  inline bool sendMessageWithNN(VlcbOpCodes opc, byte b1, byte b2, byte b3, byte b4, byte b5);
  bool sendWRACK();
  bool sendCMDERR(byte cerrno);
  void sendGRSP(VlcbOpCodes opCode, byte serviceType, byte errCode);
  void sendDGN(byte serviceIndex, byte diagCode, unsigned int counter);

  byte getModuleCANID() const { return module_config->CANID; }
  void process();
  void indicateMode(VlcbModeParams mode);
  void indicateActivity();
  
  void putAction(const Action & action);
  void putAction(ACTION action);
  bool pendingAction();

  void messageActedOn();
  unsigned int getMessagesActedOn() { return diagMsgsActed; }
  
  const CircularBuffer<Action, ACTION_QUEUE_SIZE> & getActionQueue() const { return actionQueue; }

  void addTimedResponse(TimedResponse::Task * task);

private:
  Configuration *module_config;
  ArrayHolder<Service *> services;

  CircularBuffer<Action, ACTION_QUEUE_SIZE> actionQueue;
  TimedResponse timedResponses;

  bool sendMessageWithNNandData(VlcbOpCodes opc) { return sendMessageWithNNandData(opc, 0, 0); }
  bool sendMessageWithNNandData(VlcbOpCodes opc, int len, ...);

  // Really an MNS diagnostic but placed here as its data is collected across all services.
  unsigned int diagMsgsActed = 0;
};

bool Controller::sendMessageWithNN(VlcbOpCodes opc)
{
  return sendMessageWithNNandData(opc);
}

bool Controller::sendMessageWithNN(VlcbOpCodes opc, byte b1)
{
  return sendMessageWithNNandData(opc, 1, b1);
}

bool Controller::sendMessageWithNN(VlcbOpCodes opc, byte b1, byte b2)
{
  return sendMessageWithNNandData(opc, 2, b1, b2);
}

bool Controller::sendMessageWithNN(VlcbOpCodes opc, byte b1, byte b2, byte b3)
{
  return sendMessageWithNNandData(opc, 3, b1, b2, b3);
}

bool Controller::sendMessageWithNN(VlcbOpCodes opc, byte b1, byte b2, byte b3, byte b4)
{
  return sendMessageWithNNandData(opc, 4, b1, b2, b3, b4);
}

bool Controller::sendMessageWithNN(VlcbOpCodes opc, byte b1, byte b2, byte b3, byte b4, byte b5)
{
  return sendMessageWithNNandData(opc, 5, b1, b2, b3, b4, b5);
}

}
