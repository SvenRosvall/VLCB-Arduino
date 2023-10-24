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
#include "UserInterface.h"
#include "Service.h"
#include "initializer_list.h"
#include "ArrayHolder.h"

namespace VLCB
{

//
/// CAN/Controller message type
//
struct CANFrame
{
  uint32_t id;
  bool ext;
  bool rtr;
  int8_t len; // Value 0-7 or -1 for messages handled in CanTransport
  uint8_t data[8];
};

//
/// Main object in VLCB. Coordinates transport, ui, configuration and services.
//
class Controller
{
public:
  Controller(UserInterface *ui, Transport *trpt, std::initializer_list<Service *> services);
  Controller(UserInterface * ui, Configuration *conf, Transport * trpt, std::initializer_list<Service *> services);
  
  Configuration * getModuleConfig() { return module_config; }

  void setName(const unsigned char *mname);
  const unsigned char *getModuleName() { return _mname; }

  const ArrayHolder<Service *> & getServices() { return services; }

  void setParams(unsigned char *mparams);
  void setParamFlag(unsigned char flag, bool b);
  unsigned char getParam(unsigned int param) { return _mparams[param]; }

  bool sendMessage(CANFrame *msg, bool rtr = false, bool ext = false, byte priority = DEFAULT_PRIORITY)
  {
    indicateActivity();
    return transport->sendMessage(msg, rtr, ext, priority);
  }
  
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

  void startCANenumeration();
  byte getModuleCANID() { return module_config->CANID; }
  static bool isExt(CANFrame *msg);
  static bool isRTR(CANFrame *msg);
  void process(byte num_messages = 3);
  void indicateMode(byte mode);
  void indicateActivity();
  void setLearnMode(byte reqMode);
  bool isSetProdEventTableFlag() { return setProdEventTable; }
  void clearProdEventTableFlag();
  void setFrameHandler(void (*fptr)(CANFrame *msg), byte *opcodes = NULL, byte num_opcodes = 0);

private:                                          // protected members become private in derived classes
  UserInterface *_ui;
  Configuration *module_config;
  ArrayHolder<Service *> services;
  Transport * transport;

  unsigned char *_mparams;
  const unsigned char *_mname;
  void (*framehandler)(CANFrame *msg) = NULL;
  byte *_opcodes = NULL;
  byte _num_opcodes = 0;
  bool setProdEventTable = false;

  bool filterByOpcodes(const CANFrame *msg) const;
  void callFrameHandler(CANFrame *msg);
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