// Copyright (C) Martin Da Costa 2023 (martindc.merg@gmail.com)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

// TODO: 
// Check error messages
// Set Params flags
// Trap for EVs <2

#include <Streaming.h>
#include "EventProducerService.h"
#include <Controller.h>
#include <vlcbdefs.hpp>

namespace VLCB {

//
/// register the user handler for learned events
//
void EventProducerService::setRequestEventHandler(void (*fptr)(byte index, const VlcbMessage *msg)) 
{
  requesteventhandler = fptr;
}

void EventProducerService::begin()
{  
  if (controller->getModuleConfig()->currentMode == MODE_UNINITIALISED)
  {
    uninit = true;    
  }
}

void EventProducerService::setProducedEvents()
{ 
  for (byte i = 1; i <= controller->getModuleConfig()->EE_PRODUCED_EVENTS; i++)
  {
    createDefaultEvent(i);    
  }    
}

byte EventProducerService::createDefaultEvent(byte evValue)
{
  // This function is only called when an event needs to be created, so no need to check if event exists.
  Configuration *module_config = controller->getModuleConfig();
  unsigned int nodeNum = module_config->nodeNum;
  
  byte index = module_config->findEventSpace();
  //TODO: Consider full event table error message. Should error come from Configuration?
  
  // Find next available event number.
  unsigned int eventNum;
  for (eventNum = 1; eventNum <= module_config->getNumEvents(); eventNum++)
  {
    if (module_config->findExistingEvent(nodeNum, eventNum) == module_config->getNumEvents())
    {
      break;
    }
  }
  
  // DEBUG_SERIAL << F("eps>Event Number = ") << eventNum << endl;

  byte nn_en[EE_HASH_BYTES];
  Configuration::setTwoBytes(&nn_en[0], nodeNum);
  Configuration::setTwoBytes(&nn_en[2], eventNum);
   
  module_config->writeEvent(index, nn_en);
  module_config->writeEventEV(index, 1, evValue);
  
  for (byte i = 2; i <= module_config->getNumEVs(); i++)
  {
    module_config->writeEventEV(index, i, 0);
  }
  module_config->updateEvHashEntry(index);
  
  return index;
}


void EventProducerService::process(const Action * action)
{
  // Do this if mode changes from uninitialised to normal
  if (((uninit) && (controller->getModuleConfig()->currentMode == MODE_NORMAL)))
  {
    setProducedEvents();
    uninit = false;
  }
  
  if (action != nullptr && action->actionType == ACT_MESSAGE_IN)
  {
    handleProdSvcMessage(&action->vlcbMessage);
  }
}

void EventProducerService::findOrCreateEventByEv(byte evIndex, byte evValue, byte nn_en[EE_HASH_BYTES])
{
  Configuration *module_config = controller->getModuleConfig();
  byte index = module_config->findExistingEventByEv(evIndex, evValue);
  if (index >= module_config->getNumEvents())
  {
    index = createDefaultEvent(evValue);
  }

  module_config->readEvent(index, nn_en);
  //DEBUG_SERIAL << F("eps>index = ") << index << F(" , Node Number = 0x") << _HEX(nn_en[0]) << _HEX(nn_en[1]) << endl;
  if ((nn_en[0] == 0xff) && (nn_en[1] == 0xff))
  {
    // This table entry was not initalised correctly.
    // This may happen if an event is deleted but the hash table is not updated.
    index = createDefaultEvent(evValue);
    module_config->readEvent(index, nn_en);
  }
}

void EventProducerService::sendMessage(VlcbMessage &msg, byte opCode, const byte *nn_en)
{
  msg.data[0] = opCode;
  msg.data[1] = nn_en[0];
  msg.data[2] = nn_en[1];
  msg.data[3] = nn_en[2];
  msg.data[4] = nn_en[3];
  controller->sendMessage(&msg);
}

void EventProducerService::sendEvent(bool state, byte evValue)
{
  byte nn_en[EE_HASH_BYTES];
  findOrCreateEventByEv(1, evValue, nn_en);

  byte opCode;
  if ((nn_en[0] == 0) && (nn_en[1] == 0))
  {
    opCode = (state ? OPC_ASON : OPC_ASOF);
    Configuration::setTwoBytes(&nn_en[0], controller->getModuleConfig()->nodeNum);
  }
  else
  {
    opCode = (state ? OPC_ACON : OPC_ACOF);
  }
  
  VlcbMessage msg;
  msg.len = 5;
  sendMessage(msg, opCode, nn_en);
  ++diagEventsProduced;
}

void EventProducerService::sendEvent(bool state, byte evValue, byte data1)
{
  byte nn_en[EE_HASH_BYTES];
  findOrCreateEventByEv(1, evValue, nn_en);

  byte opCode;
  if ((nn_en[0] == 0) && (nn_en[1] == 0))
  {
    opCode = (state ? OPC_ASON1 : OPC_ASOF1);
    Configuration::setTwoBytes(&nn_en[0], controller->getModuleConfig()->nodeNum);
  }
  else
  {
    opCode = (state ? OPC_ACON1 : OPC_ACOF1);
  }
  
  VlcbMessage msg;
  msg.len = 6;
  msg.data[5] = data1;
  sendMessage(msg, opCode, nn_en);
  ++diagEventsProduced;
}

void EventProducerService::sendEvent(bool state, byte evValue, byte data1, byte data2)
{
  byte nn_en[EE_HASH_BYTES];
  findOrCreateEventByEv(1, evValue, nn_en);

  byte opCode;
  if ((nn_en[0] == 0) && (nn_en[1] == 0))
  {
    opCode = (state ? OPC_ASON2 : OPC_ASOF2);
    Configuration::setTwoBytes(&nn_en[0], controller->getModuleConfig()->nodeNum);
  }
  else
  {
    opCode = (state ? OPC_ACON2 : OPC_ACOF2);
  }
  
  VlcbMessage msg;
  msg.len = 7;
  msg.data[5] = data1;
  msg.data[6] = data2;
  sendMessage(msg, opCode, nn_en);
  ++diagEventsProduced;
}

void EventProducerService::sendEvent(bool state, byte evValue, byte data1, byte data2, byte data3)
{
  byte nn_en[EE_HASH_BYTES];
  findOrCreateEventByEv(1, evValue, nn_en);

  byte opCode;
  if ((nn_en[0] == 0) && (nn_en[1] == 0))
  {
    opCode = (state ? OPC_ASON3 : OPC_ASOF3);
    Configuration::setTwoBytes(&nn_en[0], controller->getModuleConfig()->nodeNum);
  }
  else
  {
    opCode = (state ? OPC_ACON3 : OPC_ACOF3);
  }
  
  VlcbMessage msg;
  msg.len = 8;
  msg.data[5] = data1;
  msg.data[6] = data2;
  msg.data[7] = data3;
  sendMessage(msg, opCode, nn_en);
  ++diagEventsProduced;
}

void EventProducerService::handleProdSvcMessage(const VlcbMessage *msg) 
{
  unsigned int opc = msg->data[0];
  unsigned int nn = Configuration::getTwoBytes(&msg->data[1]);
  unsigned int en = Configuration::getTwoBytes(&msg->data[3]);
  
  if (requesteventhandler != nullptr)
  {
    switch (opc)
    {
      case OPC_ASRQ:
        if ((!isThisNodeNumber(nn)) && (nn != 0000))
        {
          return;
        }
        nn = 0000;
        break;
        
      case OPC_AREQ:
        break;
        
      default:
        return;
    }
    
    // Handler only called for producer events.  Producer events are recognised by having EV1
    // set to an input channel (ev value > 0)
    Configuration *module_config = controller->getModuleConfig();
    byte index = module_config->findExistingEvent(nn, en);
 
    if (index < module_config->getNumEvents())
    {
      if (module_config->getEventEVval(index, 1) != 0)
      {
        (void)(*requesteventhandler)(index, msg);
      }
    }      
  }
}

void EventProducerService::sendEventResponse(bool state, byte index)
{
  byte nn_en[EE_HASH_BYTES];
  controller->getModuleConfig()->readEvent(index, nn_en);
  //DEBUG_SERIAL << ">EPService node number = 0x" << _HEX(nn_en[0]) << _HEX(nn_en[1])<< endl;
  
  byte opCode;
  if ((nn_en[0] == 0) && (nn_en[1] == 0))
  {
    opCode = (state ? OPC_ARSON : OPC_ARSOF);
    Configuration::setTwoBytes(&nn_en[0], controller->getModuleConfig()->nodeNum);
  }
  else
  {
    opCode = (state ? OPC_ARON : OPC_AROF);
  }
  
  VlcbMessage msg;
  msg.len = 5;
  sendMessage(msg, opCode, nn_en);
}

void EventProducerService::sendEventResponse(bool state, byte index, byte data1)
{
  byte nn_en[EE_HASH_BYTES];
  controller->getModuleConfig()->readEvent(index, nn_en);
  //DEBUG_SERIAL << ">EPService node number = 0x" << _HEX(nn_en[0]) << _HEX(nn_en[1])<< endl;
  
  byte opCode;
  if ((nn_en[0] == 0) && (nn_en[1] == 0))
  {
    opCode = (state ? OPC_ARSON1 : OPC_ARSOF1);
    Configuration::setTwoBytes(&nn_en[0], controller->getModuleConfig()->nodeNum);
  }
  else
  {
    opCode = (state ? OPC_ARON1 : OPC_AROF1);
  }
  
  VlcbMessage msg;
  msg.len = 6;
  msg.data[5] = data1;
  sendMessage(msg, opCode, nn_en);
}

void EventProducerService::sendEventResponse(bool state, byte index, byte data1, byte data2)
{
  byte nn_en[EE_HASH_BYTES];
  controller->getModuleConfig()->readEvent(index, nn_en);
  //DEBUG_SERIAL << ">EPService node number = 0x" << _HEX(nn_en[0]) << _HEX(nn_en[1])<< endl;
  
  byte opCode;
  if ((nn_en[0] == 0) && (nn_en[1] == 0))
  {
    opCode = (state ? OPC_ARSON2 : OPC_ARSOF2);
    Configuration::setTwoBytes(&nn_en[0], controller->getModuleConfig()->nodeNum);
  }
  else
  {
    opCode = (state ? OPC_ARON2 : OPC_AROF2);
  }
  
  VlcbMessage msg;
  msg.len = 7;
  msg.data[5] = data1;
  msg.data[6] = data2;
  sendMessage(msg, opCode, nn_en);
}

void EventProducerService::sendEventResponse(bool state, byte index, byte data1, byte data2, byte data3)
{
  byte nn_en[EE_HASH_BYTES];
  controller->getModuleConfig()->readEvent(index, nn_en);
  //DEBUG_SERIAL << ">EPService node number = 0x" << _HEX(nn_en[0]) << _HEX(nn_en[1])<< endl;
  
  byte opCode;
  if ((nn_en[0] == 0) && (nn_en[1] == 0))
  {
    opCode = (state ? OPC_ARSON3 : OPC_ARSOF3);
    Configuration::setTwoBytes(&nn_en[0], controller->getModuleConfig()->nodeNum);
  }
  else
  {
    opCode = (state ? OPC_ARON3 : OPC_AROF3);
  }
  
  VlcbMessage msg;
  msg.len = 8;
  msg.data[5] = data1;
  msg.data[6] = data2;
  msg.data[7] = data3;
  sendMessage(msg, opCode, nn_en);
}
}