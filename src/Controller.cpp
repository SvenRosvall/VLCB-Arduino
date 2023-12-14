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

// TODO: This is a sign something is wrong. Need to be this big to handle 
// large number of generated messages such as when asking for node parameters.
// Each entry uses 10 bytes so a size of 30 uses 300 bytes.
// This will get worse for for example queries for all events.
// Need a mechanism like TimedResponse to create messages slowly so they can
// be sent through the transport without requiring this buffering. 
const int COMMAND_QUEUE_SIZE = 30;

Controller::Controller(UserInterface * ui, Transport * trpt, std::initializer_list<Service *> services)
  : _ui(ui)
  , transport(trpt)
  , services(services)
  , commandQueue(COMMAND_QUEUE_SIZE)
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
    , commandQueue(COMMAND_QUEUE_SIZE)
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

void Controller::indicateMode(byte mode)
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

static bool isEventMessage(VlcbMessage & message)
{
  if (message.len == 0)
  {
    return false;
  }
  switch (message.data[0])
  {
    case OPC_ACON:
    case OPC_ACON1:
    case OPC_ACON2:
    case OPC_ACON3:
    case OPC_ACOF:
    case OPC_ACOF1:
    case OPC_ACOF2:
    case OPC_ACOF3:
    case OPC_ASON:
    case OPC_ASON1:
    case OPC_ASON2:
    case OPC_ASON3:
    case OPC_ASOF:
    case OPC_ASOF1:
    case OPC_ASOF2:
    case OPC_ASOF3:
    case OPC_ARON:
    case OPC_AROF:
      return true;
    default:
      return false;
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
  
  transport->process(); // Let Transport check for incoming messages and put it on the command queue.

  if (commandQueue.available())
  {
    Command cmd = *commandQueue.get();
    
    // TODO: Pass this to each Service::process(cmd);
    // But for now just call handleMessage()
    switch (cmd.commandType)
    {
      case CMD_MESSAGE_OUT:
        // TODO: This is an intermediate step until CanTransport has been converted to a Service.

        indicateActivity();
        transport->sendMessage(&cmd.vlcbMessage);

        if (!(isEventMessage(cmd.vlcbMessage) && (_mparams[PAR_FLAGS] & PF_COE)))
        {
          break;
        }
        // else Fall through: A message sent out should also be picked up by the consumer service.

        // TODO Note: When this is replaced with process(cmd) the EventConsumerService shall accept
        // CMD_MESSAGE_IN, but also CMD_MESSAGE_OUT but only for events. 

      case CMD_MESSAGE_IN:
      {
        // TODO: This is an intermediate step until services have been migrated to the new interface.

        indicateActivity();
        VlcbMessage &msg = cmd.vlcbMessage;
        unsigned int opc = msg.data[0];
        // DEBUG_SERIAL << "> Passing on message with op=" << _HEX(opc) << endl;
        for (Service *service: services)
        {
          if (service->handleMessage(opc, &msg) == PROCESSED)
          {
            break;
          }
        }
        // DEBUG_SERIAL << F("> end of opcode processing, time = ") << (micros() - mtime) << "us" << endl;
      }
      break;

      default:
        break;
    }
  }


  //
  /// end of Controller message processing
  //
}

void setNN(VlcbMessage *msg, unsigned int nn)
{
  msg->data[1] = highByte(nn);
  msg->data[2] = lowByte(nn);
}

bool Controller::sendMessage(VlcbMessage *msg)
{
  Command cmd = {CMD_MESSAGE_OUT, *msg};
  commandQueue.put(&cmd);
  return true;
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

bool Controller::pendingCommands()
{
  return commandQueue.available();
}

}