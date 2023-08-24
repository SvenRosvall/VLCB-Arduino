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

  // TODO: These methods deal with transportation. While refactoring they delegate to the transport.

  bool sendMessage(CANFrame *msg, bool rtr = false, bool ext = false, byte priority = DEFAULT_PRIORITY)
  {
    return transport->sendMessage(msg, rtr, ext, priority);
  }

  bool sendMessageWithNN(int opc);
  bool sendMessageWithNN(int opc, byte b1);
  bool sendMessageWithNN(int opc, byte b1, byte b2);
  bool sendMessageWithNN(int opc, byte b1, byte b2, byte b3);
  bool sendMessageWithNN(int opc, byte b1, byte b2, byte b3, byte b4, byte b5);
  bool sendWRACK();
  bool sendCMDERR(byte cerrno);
  void sendGRSP(byte opCode, byte serviceType, byte errCode);

  void startCANenumeration();
  byte getModuleCANID() { return module_config->CANID; }
  static bool isExt(CANFrame *msg);
  static bool isRTR(CANFrame *msg);
  void process(byte num_messages = 3);
  void setParams(unsigned char *mparams);
  void setName(unsigned char *mname);
  void indicateMode(byte mode);
  void indicateActivity();
  void setFrameHandler(void (*fptr)(CANFrame *msg), byte *opcodes = NULL, byte num_opcodes = 0);

private:                                          // protected members become private in derived classes
  UserInterface *_ui;
  Configuration *module_config;
  std::initializer_list<Service *> services;
  Transport * transport;

  unsigned char *_mparams;
  unsigned char *_mname;
  void (*framehandler)(CANFrame *msg) = NULL;
  byte *_opcodes = NULL;
  byte _num_opcodes = 0;

  bool filterByOpcodes(const CANFrame *msg) const;
  void callFrameHandler(CANFrame *msg);
  void performRequestedUserAction(UserInterface::RequestedAction requestedAction);

  // Quick way to access necessary stuff when migrating to services.
  // TODO: Review the necessary fields to see what is required by services
  // TODO: Create getter/setter for each field.
  friend class MinimumNodeService;
  friend class CanService;
  friend class CbusService;
  friend class EventConsumerService;
  friend class EventTeachingService;
};

}