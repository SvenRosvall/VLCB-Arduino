// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#pragma once

#ifndef DEBUG_SERIAL
#define DEBUG_SERIAL Serial
#endif

#include <SPI.h>

#include <Configuration.h>
#include <Transport.h>
#include "Service.h"
#include "initializer_list.h"
#include "ArrayHolder.h"
#include "CircularBuffer.h"

namespace VLCB
{

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

//
/// Main object in VLCB. Coordinates transport, ui, configuration and services.
//
class Controller
{
public:
  Controller(std::initializer_list<Service *> services);
  Controller(Configuration *conf, std::initializer_list<Service *> services);

  Configuration * getModuleConfig() { return module_config; }

  void setName(const unsigned char *mname);
  const unsigned char *getModuleName() { return _mname; }

  const ArrayHolder<Service *> & getServices() { return services; }

  void setParams(unsigned char *mparams);
  void setParamFlag(unsigned char flag, bool b);
  unsigned char getParam(unsigned int param) { return _mparams[param]; }

  bool sendMessage(const VlcbMessage *msg);

  void begin();
  inline bool sendMessageWithNN(int opc);
  inline bool sendMessageWithNN(int opc, byte b1);
  inline bool sendMessageWithNN(int opc, byte b1, byte b2);
  inline bool sendMessageWithNN(int opc, byte b1, byte b2, byte b3);
  inline bool sendMessageWithNN(int opc, byte b1, byte b2, byte b3, byte b4);
  inline bool sendMessageWithNN(int opc, byte b1, byte b2, byte b3, byte b4, byte b5);
  bool sendWRACK();
  bool sendCMDERR(byte cerrno);
  void sendGRSP(byte opCode, byte serviceType, byte errCode);

  byte getModuleCANID() { return module_config->CANID; }
  void process();
  void indicateMode(VlcbModeParams mode);
  void indicateActivity();
  void setLearnMode(byte reqMode);
  
  void putAction(const Action & action);
  void putAction(ACTION action);
  bool pendingAction();

private:
  Configuration *module_config;
  ArrayHolder<Service *> services;

  unsigned char *_mparams;
  const unsigned char *_mname;
  
  CircularBuffer<Action> actionQueue;

  bool sendMessageWithNNandData(int opc) { return sendMessageWithNNandData(opc, 0, 0); }
  bool sendMessageWithNNandData(int opc, int len, ...);
};

bool Controller::sendMessageWithNN(int opc)
{
  return sendMessageWithNNandData(opc);
}

bool Controller::sendMessageWithNN(int opc, byte b1)
{
  return sendMessageWithNNandData(opc, 1, b1);
}

bool Controller::sendMessageWithNN(int opc, byte b1, byte b2)
{
  return sendMessageWithNNandData(opc, 2, b1, b2);
}

bool Controller::sendMessageWithNN(int opc, byte b1, byte b2, byte b3)
{
  return sendMessageWithNNandData(opc, 3, b1, b2, b3);
}

bool Controller::sendMessageWithNN(int opc, byte b1, byte b2, byte b3, byte b4)
{
  return sendMessageWithNNandData(opc, 4, b1, b2, b3, b4);
}

bool Controller::sendMessageWithNN(int opc, byte b1, byte b2, byte b3, byte b4, byte b5)
{
  return sendMessageWithNNandData(opc, 5, b1, b2, b3, b4, b5);
}

}
