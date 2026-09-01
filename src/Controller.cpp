// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

// 3rd party libraries
#include <Streaming.h>

// Controller library
#include <Controller.h>
#include <Service.h>
#include <stdarg.h>

//
/// construct a Controller object with an external Configuration object named "config" that is defined
/// in user code
//

namespace VLCB
{

//Controller::Controller()
//  : services()
//  , actionQueue(ACTION_QUEUE_SIZE)
//{
//  extern Configuration config;
//  module_config = &config;
//}

VlcbMessage & VlcbMessage::addData(byte b)
{
  data[len++] = b;
  return *this;
}

VlcbMessage & VlcbMessage::add2Bytes(unsigned int n)
{
  data[len++] = highByte(n);
  data[len++] = lowByte(n);
  return *this;
}

VlcbMessage & VlcbMessage::addNNEN(byte nn_en[EE_HASH_BYTES])
{
  for (int i = 0; i < EE_HASH_BYTES; ++i)
  {
    addData(nn_en[i]);
  }
  return *this;
}

Controller::Controller(Configuration *conf)
  : module_config(conf)
{
}

//Controller::Controller(std::initializer_list<Service *> services)
//  : services(services)
//  , actionQueue(ACTION_QUEUE_SIZE)
//{
//  extern Configuration config;
//  module_config = &config;
//
//  for (Service * service : services)
//  {
//    service->setController(this);
//  }
//}

//
/// construct a Controller object with a Configuration object that the user provides.
/// note that this Configuration object must have a lifetime longer than the Controller object.
//
Controller::Controller(Configuration *conf, std::initializer_list<Service *> services)
  : module_config(conf)
  , services(services)
{
  for (Service * service : services)
  {
    service->setController(this);
  }
}

void Controller::setServices(std::initializer_list<Service *> svc)
{
  services = svc;

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
void Controller::updateParamFlags()
{
  for (Service * svc : services)
  {
    switch (svc->getServiceID())
    {
      case SERVICE_ID_MNS:
        module_config->setFlag(PF_VLCB);
        break;
      case SERVICE_ID_PRODUCER:
        module_config->setFlag(PF_PRODUCER);
        break;
      case SERVICE_ID_CONSUMER:
        module_config->setFlag(PF_CONSUMER);
        break;
      case SERVICE_ID_CONSUME_OWN_EVENTS:
        module_config->setFlag(PF_COE);
        break;
      default:
        break;
    }
  }
  if (module_config->currentMode == MODE_NORMAL)
  {
    module_config->setFlag(PF_NORMAL); 
  }
}

//
/// set the Controller LEDs to indicate the current mode
//

void Controller::indicateMode(VlcbModeParams mode)
{
  //DEBUG_SERIAL << F("ctrl> indicating mode = ") << mode << endl;
  Action action = {ACT_INDICATE_MODE, mode};
  putAction(action);
  
  setParamFlag(PF_NORMAL, mode == MODE_NORMAL);
}

void Controller::setParamFlag(VlcbParamFlags flag, bool set)
{ 
  if (set)
  {
    module_config->setFlag(flag);
  }
  else
  {
    module_config->clearFlag(flag);
  }
}

void Controller::indicateActivity()
{
  putAction(ACT_INDICATE_ACTIVITY);
}

//
/// main Controller message processing procedure
//
void Controller::process()
{
  //Serial << F("Ctrl::process() start, action queue size = ") << actionQueue.size();
  if (actionQueue.available())
  {
    // Get the next action and store it locally so that it is not overwritten if the action queue gets full.
    Action action = actionQueue.pop();
    //Serial << F(" action type = ") << action.actionType << endl;
    for (Service *service: services)
    {
      service->processAction(action);
    }
  }

  for (Service *service: services)
  {
    service->process();
  }

  timedResponses.process();
  
  module_config->commitToEEPROM();
}

bool Controller::sendMessage(const VlcbMessage &msg)
{
  Action action = {ACT_MESSAGE_OUT, msg};
  actionQueue.put(action);
  return true;
}

bool Controller::sendMessage(const VlcbMessage *msg)
{
  return sendMessage(*msg);
}

bool Controller::sendMessageWithNNandData(VlcbOpCodes opc, int dataLen, byte data1, byte data2, byte data3, byte data4, byte data5)
{
  VlcbMessage msg(opc);
  msg.addNN(module_config->nodeNum);
  if (dataLen >= 1)
  {
    msg.addData(data1);
  }
  if (dataLen >= 2)
  {
    msg.addData(data2);
  }
  if (dataLen >= 3)
  {
    msg.addData(data3);
  }
  if (dataLen >= 4)
  {
    msg.addData(data4);
  }
  if (dataLen >= 5)
  {
    msg.addData(data5);
  }
  return sendMessage(msg);  
}

//
/// send a WRACK (write acknowledge) message
//
bool Controller::sendWRACK()
{
  // send a write acknowledgement response
  return sendMessage(VlcbMessage(OPC_WRACK).addNN(module_config->nodeNum));
}

//
/// send a CMDERR (command error) message
//
bool Controller::sendCMDERR(byte cerrno)
{
  // send a command error response
  return sendMessage(VlcbMessage(OPC_CMDERR).addNN(module_config->nodeNum).addData(cerrno));
}

void Controller::sendGRSP(VlcbOpCodes opCode, byte serviceType, byte errCode)
{
  sendMessage(VlcbMessage(OPC_GRSP).addNN(module_config->nodeNum).addData(opCode).addData(serviceType).addData(errCode));
}

void Controller::sendDGN(byte serviceIndex, byte diagCode, unsigned int counter)
{
  sendMessage(VlcbMessage(OPC_DGN).addNN(module_config->nodeNum).addData(serviceIndex).addData(diagCode).add2Bytes(counter));
}

void Controller::putAction(const Action &action)
{
  // Serial << F("C>put action with type=") << action.actionType << endl;
  actionQueue.put(action);
}

void Controller::putAction(ACTION action)
{
  putAction(Action{action});
}

bool Controller::pendingAction()
{
  return actionQueue.available();
}

bool Controller::pendingTasks()
{
  return timedResponses.pendingTasks();
}

void Controller::messageActedOn()
{
  putAction(ACT_INDICATE_WORK);
  ++diagMsgsActed;
}

void Controller::addTimedResponseTask(TimedResponse::Task * task)
{
  timedResponses.add(task);
}

}
