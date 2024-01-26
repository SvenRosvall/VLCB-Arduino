// Copyright (C) Martin Da Costa 2023 (martindc.merg@gmail.com)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

// TODO: Add AREQ and ASRQ opcode support.
// Check error messages
// Set Params flags
// Trap for EVs <2

#include <Streaming.h>
#include "EventProducerService.h"
#include <Controller.h>
#include <vlcbdefs.hpp>

namespace VLCB {

void EventProducerService::setController(Controller *cntrl) 
{
  this->controller = cntrl;
  this->module_config = cntrl->getModuleConfig();
}

void EventProducerService::begin()
{  
  if (module_config->currentMode == MODE_UNINITIALISED)
  {
    uninit = true;    
  }
}

void EventProducerService::setProducedEvents()
{ 
  for (byte i = 1; i <= module_config->EE_PRODUCED_EVENTS; i++)
  {
    createDefaultEvent(i);    
  }    
}

byte EventProducerService::createDefaultEvent(byte evValue)
{
  byte data[4];
  data[0] = highByte(module_config->nodeNum);
  data[1] = lowByte(module_config->nodeNum);
  data[2] = 0;
  data[3] = evValue;
  
  byte index = module_config->findEventSpace();
  
  module_config->writeEvent(index, data);
  module_config->writeEventEV(index, 1, evValue);
  module_config->updateEvHashEntry(index);
  
  return index;
}


void EventProducerService::process(const Action * action)
{
  // Do this if mode changes from uninitialised to normal
  if (((uninit) && (module_config->currentMode == MODE_NORMAL)))
  {
    setProducedEvents();
    uninit = false;
  }
  
  if (action != nullptr && action->actionType == ACT_MESSAGE_IN)
  {
    handleProdSvcMessage(&action->vlcbMessage);
  }
}

void EventProducerService::findOrCreateEventByEv(byte evIndex, byte evValue, byte nn_en[4])
{
  byte index = module_config->findExistingEventByEv(evIndex, evValue);
  if (index >= module_config->EE_MAX_EVENTS)
  {
    index = createDefaultEvent(evValue);
  }

  module_config->readEvent(index, nn_en);
  //DEBUG_SERIAL << F("eps>index = ") << index << F(" , Node Number = 0x") << _HEX(nn_en[0]) << _HEX(nn_en[1]) << endl;
  if ((nn_en[0] == 0xff) && (nn_en[1] == 0xff))
  {
    // This table entry was not initalised correctly.
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
  byte nn_en[4];
  findOrCreateEventByEv(1, evValue, nn_en);

  byte opCode;
  if ((nn_en[0] == 0) && (nn_en[1] == 0))
  {
    opCode = (state ? OPC_ASON : OPC_ASOF);
    nn_en[0] = highByte(module_config->nodeNum);
    nn_en[1] = lowByte(module_config->nodeNum); 
  }
  else
  {
    opCode = (state ? OPC_ACON : OPC_ACOF);
  }
  
  VlcbMessage msg;
  msg.len = 5;
  sendMessage(msg, opCode, nn_en);
}

void EventProducerService::sendEvent(bool state, byte evValue, byte data1)
{
  byte nn_en[4];
  findOrCreateEventByEv(1, evValue, nn_en);

  byte opCode;
  if ((nn_en[0] == 0) && (nn_en[1] == 0))
  {
    opCode = (state ? OPC_ASON1 : OPC_ASOF1);
    nn_en[0] = highByte(module_config->nodeNum);
    nn_en[1] = lowByte(module_config->nodeNum); 
  }
  else
  {
    opCode = (state ? OPC_ACON1 : OPC_ACOF1);
  }
  
  VlcbMessage msg;
  msg.len = 6;
  msg.data[5] = data1;
  sendMessage(msg, opCode, nn_en);

}

void EventProducerService::sendEvent(bool state, byte evValue, byte data1, byte data2)
{
  byte nn_en[4];
  findOrCreateEventByEv(1, evValue, nn_en);

  byte opCode;
  if ((nn_en[0] == 0) && (nn_en[1] == 0))
  {
    opCode = (state ? OPC_ASON2 : OPC_ASOF2);
    nn_en[0] = highByte(module_config->nodeNum);
    nn_en[1] = lowByte(module_config->nodeNum); 
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
}

void EventProducerService::sendEvent(bool state, byte evValue, byte data1, byte data2, byte data3)
{
  byte nn_en[4];
  findOrCreateEventByEv(1, evValue, nn_en);

  byte opCode;
  if ((nn_en[0] == 0) && (nn_en[1] == 0))
  {
    opCode = (state ? OPC_ASON3 : OPC_ASOF3);
    nn_en[0] = highByte(module_config->nodeNum);
    nn_en[1] = lowByte(module_config->nodeNum); 
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
}

void EventProducerService::handleProdSvcMessage(const VlcbMessage *msg) 
{
  unsigned int opc = msg->data[0];
  unsigned int nn = (msg->data[1] << 8) + msg->data[2];
  // DEBUG_SERIAL << ">VLCBProdSvc handling message op=" << _HEX(opc) << " nn=" << nn << " en" << en << endl;

  switch (opc) 
  {    

    case OPC_AREQ:
      // AREQ message - request for node state, only producer nodes

      if ((nn == module_config->nodeNum) && (eventhandler != nullptr)) 
      {
        (void)(*eventhandler)(0, msg);
      }
      break;

    case OPC_ASRQ:



      break;

  }
}
}