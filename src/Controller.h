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
  uint8_t len;
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

  // TODO: These methods deal with transportation. While refactoring they delegate to the transport.
  bool sendMessage(CANFrame *msg, bool rtr = false, bool ext = false, byte priority = DEFAULT_PRIORITY)
  {
    return transport->sendMessage(msg, rtr, ext, priority);
  }
  
  void begin();
  bool sendMessageWithNN(int opc);
  bool sendMessageWithNN(int opc, byte b1);
  bool sendMessageWithNN(int opc, byte b1, byte b2);
  bool sendMessageWithNN(int opc, byte b1, byte b2, byte b3);
  bool sendMessageWithNN(int opc, byte b1, byte b2, byte b3, byte b4);
  bool sendMessageWithNN(int opc, byte b1, byte b2, byte b3, byte b4, byte b5);
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
};

}