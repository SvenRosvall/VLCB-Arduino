
/*

  Copyright (C) Duncan Greenwood 2017 (duncan_greenwood@hotmail.com)

  This work is licensed under the:
      Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
   To view a copy of this license, visit:
      http://creativecommons.org/licenses/by-nc-sa/4.0/
   or send a letter to Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.

   License summary:
    You are free to:
      Share, copy and redistribute the material in any medium or format
      Adapt, remix, transform, and build upon the material

    The licensor cannot revoke these freedoms as long as you follow the license terms.

    Attribution : You must give appropriate credit, provide a link to the license,
                  and indicate if changes were made. You may do so in any reasonable manner,
                  but not in any way that suggests the licensor endorses you or your use.

    NonCommercial : You may not use the material for commercial purposes. **(see note below)

    ShareAlike : If you remix, transform, or build upon the material, you must distribute
                 your contributions under the same license as the original.

    No additional restrictions : You may not apply legal terms or technological measures that
                                 legally restrict others from doing anything the license permits.

   ** For commercial use, please contact the original copyright holder(s) to agree licensing terms

    This software is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE

*/

#pragma once

#ifndef DEBUG_SERIAL
#define DEBUG_SERIAL Serial
#endif

#include <SPI.h>

#include <Configuration.h>
#include <cbusdefs.h>
#include <Transport.h>
#include "UserInterface.h"
#include "Service.h"
#include "initializer_list.h"

namespace VLCB
{

//
/// CAN/Controller message type
//

class CANFrame {

public:
  uint32_t id;
  bool ext;
  bool rtr;
  uint8_t len;
  uint8_t data[8] = {};
};

//
/// an abstract class to encapsulate CAN bus and Controller processing
/// it must be implemented by a derived subclass
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

  // implementations of these methods are provided in the base class

  bool sendWRACK(void);
  bool sendCMDERR(byte cerrno);
  void startCANenumeration(void);
  byte getCANID(unsigned long header);
  byte getModuleCANID() { return module_config->CANID; }
  bool isExt(CANFrame *msg);
  bool isRTR(CANFrame *msg);
  void process(byte num_messages = 3);
  void initFLiM(void);
  void setFLiM();
  void setSLiM(void);
  void revertSLiM(void);
  void checkModeChangeTimeout();
  void renegotiate(void);
  void setParams(unsigned char *mparams);
  void setName(unsigned char *mname);
  void checkCANenumTimout(void);
  void indicateMode(byte mode);
  void indicateActivity();
  void setFrameHandler(void (*fptr)(CANFrame *msg), byte *opcodes = NULL, byte num_opcodes = 0);

private:                                          // protected members become private in derived classes
  CANFrame _msg;
  UserInterface *_ui;
  Configuration *module_config;
  std::initializer_list<Service *> services;
  Transport * transport;

  unsigned char *_mparams;
  unsigned char *_mname;
  void (*framehandler)(CANFrame *msg);
  byte *_opcodes;
  byte _num_opcodes;
  byte enum_responses[16];                          // 128 bits for storing CAN ID enumeration results
  bool bModeChanging, bCANenum, bLearn;
  unsigned long timeOutTimer, CANenumTime;
  bool enumeration_required;

  bool filterByOpcodes(const CANFrame *msg) const;
  void callFrameHandler(CANFrame *msg);
  void performRequestedUserAction(UserInterface::RequestedAction requestedAction);
  byte findFreeCanId();

  // Quick way to access necessary stuff when migrating to services.
  // TODO: Review the necessary fields to see what is required by services
  // TODO: Create getter/setter for each field.
  friend class CbusService;
};

}