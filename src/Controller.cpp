// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

// 3rd party libraries
#include <Streaming.h>

// Controller library
#include <Controller.h>
#include "MinimumNodeService.h"
#include "CanService.h"
#include <stdarg.h>

//
/// construct a Controller object with an external Configuration object named "config" that is defined
/// in user code
//

namespace VLCB
{

Controller::Controller(UserInterface * ui, Transport * trpt, std::initializer_list<Service *> services)
  : _ui(ui)
  , transport(trpt)
  , services(services)
{
  extern Configuration config;
  module_config = &config;

  transport->setController(this);

  for (Service * service : services)
  {
    service->setController(this);
  }
}

//
/// construct a Controller object with a Configuration object that the user provides.
/// note that this Configuration object must have a lifetime longer than the Controller object.
//
Controller::Controller(UserInterface * ui, Configuration *conf, Transport * trpt, std::initializer_list<Service *> services)
  : _ui(ui)
  , module_config(conf)
  , transport(trpt)
  , services(services)
{
  transport->setController(this);

  for (Service * service : services)
  {
    service->setController(this);
  }
}

//
/// Initialise VLCB
//

void Controller::begin()
{
  module_config->begin();
  for (Service * service : services)
  {
    service->begin();
  }
}

//
/// assign the module parameter set
//
void Controller::setParams(unsigned char *mparams)
{
  _mparams = mparams;
  byte flags = 0;
  
  for (Service * svc : services)
  {
    switch (svc->getServiceID())
    {
      case SERVICE_ID_MNS:
        flags |= PF_SD;
        break;
      case SERVICE_ID_PRODUCER:
        flags |= PF_PRODUCER;
        break;
      case SERVICE_ID_CONSUMER:
        flags |= PF_CONSUMER;
        break;
      case SERVICE_ID_CONSUME_OWN_EVENTS:
        flags |= PF_COE;
        break;
    }
  }
  if (module_config->currentMode == MODE_NORMAL)
  {
    flags |= PF_NORMAL; 
  }
  _mparams[PAR_FLAGS] = flags;
}

//
/// assign the module name
//
void Controller::setName(const unsigned char *mname)
{
  _mname = mname;
}

//
/// set the Controller LEDs to indicate the current mode
//

void Controller::indicateMode(VlcbModeParams mode)
{
  // DEBUG_SERIAL << F("> indicating mode = ") << mode << endl;
  if (_ui) 
  {
    _ui->indicateMode(mode);
  }
  
  setParamFlag(PF_NORMAL, mode == MODE_NORMAL);
}

void Controller::setParamFlag(unsigned char flag, bool set)
{ 
  if (set)
  {
    _mparams[PAR_FLAGS] |= flag;
  }
  else
  {
    _mparams[PAR_FLAGS] &= ~flag;
  }
}

void Controller::indicateActivity()
{
  if (_ui)
  {
    _ui->indicateActivity();
  }
}

//
/// main Controller message processing procedure
//
void Controller::process(byte num_messages)
{
  // process switch operations if the module is configured with one
  UserInterface::RequestedAction requestedAction = UserInterface::NONE;
  if (_ui)
  {
    _ui->run();

    requestedAction = _ui->checkRequestedAction();
    // if (requestedAction != UserInterface::NONE)
    //   DEBUG_SERIAL << "Controller::process() UI action=" << requestedAction << " mode=" << module_config->currentMode << endl;
  }

  for (Service * service : services)
  {
    service->process(requestedAction);
  }

  // get received CAN frames from buffer
  // process by default 3 messages per run so the user's application code doesn't appear unresponsive under load
  for (byte mcount = 0 ; transport->available() && mcount < num_messages ; ++mcount)
  {
    // at least one CAN frame is available in the reception buffer
    // retrieve the next one

    VlcbMessage msg = transport->getNextMessage();
    // DEBUG_SERIAL << "> Received a message" << endl;

    if (msg.len == 0xFF)
    {
      // received msg was something CAN specific, ignore it.
      continue;
    }

    indicateActivity();

    //
    /// process the message opcode
    /// if we got this far, it's a standard CAN frame (not extended, not RTR) with a data payload length > 0
    //

    if (msg.len > 0) 
    {
      unsigned int opc = msg.data[0];
      // DEBUG_SERIAL << "> Passing on message with op=" << _HEX(opc) << endl;
      for (Service * service : services)
      {
        if (service->handleMessage(opc, &msg) == PROCESSED)
        {
          break;
        }
      }
    } else {
      // DEBUG_SERIAL << F("> oops ... zero - length frame ?? ") << endl;
    }
  }  // while messages available

  // DEBUG_SERIAL << F("> end of opcode processing, time = ") << (micros() - mtime) << "us" << endl;

  //
  /// end of Controller message processing
  //
}

void setNN(VlcbMessage *msg, unsigned int nn)
{
  msg->data[1] = highByte(nn);
  msg->data[2] = lowByte(nn);
}

bool Controller::sendMessageWithNNandData(int opc, int len, ...)
{
  va_list args;
  va_start(args, len);
  VlcbMessage msg;
  msg.len = len + 3;
  msg.data[0] = opc;
  setNN(&msg, module_config->nodeNum);
  for (int i = 0 ; i < len ; ++i)
  {
    msg.data[3 + i] = va_arg(args, int);
  }
  va_end(args);
  return sendMessage(&msg);  
}

//
/// send a WRACK (write acknowledge) message
//
bool Controller::sendWRACK()
{
  // send a write acknowledgement response
  return sendMessageWithNN(OPC_WRACK);
}

//
/// send a CMDERR (command error) message
//
bool Controller::sendCMDERR(byte cerrno)
{
  // send a command error response
  return sendMessageWithNN(OPC_CMDERR, cerrno);
}

void Controller::sendGRSP(byte opCode, byte serviceType, byte errCode)
{
  sendMessageWithNN(OPC_GRSP, opCode, serviceType, errCode);
}

void Controller::startCANenumeration()
{
  // Delegate this to the CanService.
  // Find it first.
  for (Service * svc : services)
  {
    if (svc->getServiceID() == SERVICE_ID_CAN)
    {
      CanService * canSvc = (CanService *) svc;
      canSvc->startCANenumeration();
    }
  }
}

}