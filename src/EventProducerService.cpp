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
  if (module_config->currentMode != MODE_UNINITIALISED)
  {
    setProducedEvents();    
  }
}

void EventProducerService::setProducedEvents()
{  
  for (byte i = 0; i < module_config->EE_PRODUCED_EVENTS; i++)
  {
    if (module_config->getEvTableEntry(i) == 0)
    {
      // Event does not exist so create it
      byte data[4];
      data[0] = highByte(module_config->nodeNum);
      data[1] = lowByte(module_config->nodeNum);
      data[2] = 0;
      data[3] = i;
      
      module_config->writeEvent(i, data);  //index = i
      module_config->writeEventEV(i, 1, 1);  //set this event as a producer (ev value of 0 would be a consumer)
      module_config->writeEventEV(i, 2, 0);  //set default produced event type
      module_config->updateEvHashEntry(i);
    }
  }    
}

void EventProducerService::process(byte num)
{
  // Do this if mode changes to normal
  if (controller->isSetProdEventTableFlag())
  {
    setProducedEvents();
    controller->clearProdEventTableFlag();
  }
}

void EventProducerService::sendEvent(bool state, byte index)
{
  byte nn_en[4];
  module_config->readEvent(index, nn_en);
  byte opCode;
  unsigned int nn = ((nn_en[0] << 8) && nn_en[1]);
  if (nn == 0)
  {
    opCode = (state ? OPC_ASON : OPC_ASOF);
    controller->sendMessageWithNN(opCode, nn_en[2], nn_en[3]); 
  }
  else
  {
    opCode = (state ? OPC_ACON : OPC_ACOF);
    CANFrame msg;
    msg.len = 5;
    msg.data[0] = opCode;
    msg.data[1] = nn_en[0];
    msg.data[2] = nn_en[1];
    msg.data[3] = nn_en[2];
    msg.data[4] = nn_en[3];
    controller->sendMessage(&msg);
  }  
}

void EventProducerService::sendEvent(bool state, byte index, byte data1)
{
  byte nn_en[4];
  module_config->readEvent(index, nn_en);
  byte opCode;
  
  if ((nn_en[0] == 0) && (nn_en[1] == 0))
  {
    opCode = (state ? OPC_ASON1 : OPC_ASOF1);
    controller->sendMessageWithNN(opCode, nn_en[2], nn_en[3], data1);
  }
  else
  {
    opCode = (state ? OPC_ACON1 : OPC_ACOF1);
    CANFrame msg;
    msg.len = 6;
    msg.data[0] = opCode;
    msg.data[1] = nn_en[0];
    msg.data[2] = nn_en[1];
    msg.data[3] = nn_en[2];
    msg.data[4] = nn_en[3];
    msg.data[5] = data1;
    controller->sendMessage(&msg);
  }
    
}

void EventProducerService::sendEvent(bool state, byte index, byte data1, byte data2)
{
  byte nn_en[4];
  module_config->readEvent(index, nn_en);
  byte opCode;
  
  if ((nn_en[0] == 0) && (nn_en[1] == 0))
  {
    opCode = (state ? OPC_ASON2 : OPC_ASOF2);
    controller->sendMessageWithNN(opCode, nn_en[2], nn_en[3], data1, data2);
  }
  else
  {
    opCode = (state ? OPC_ACON2 : OPC_ACOF2);
    CANFrame msg;
    msg.len = 7;
    msg.data[0] = opCode;
    msg.data[1] = nn_en[0];
    msg.data[2] = nn_en[1];
    msg.data[3] = nn_en[2];
    msg.data[4] = nn_en[3];
    msg.data[5] = data1;
    msg.data[6] = data2;
    controller->sendMessage(&msg);
  }    
}

void EventProducerService::sendEvent(bool state, byte index, byte data1, byte data2, byte data3)
{
  byte nn_en[4];
  module_config->readEvent(index, nn_en);
  byte opCode;
  
  if ((nn_en[0] == 0) && (nn_en[1] == 0))
  {
    opCode = (state ? OPC_ASON3 : OPC_ASOF3);
    controller->sendMessageWithNN(opCode, nn_en[2], nn_en[3], data1, data2, data3);
  }
  else
  {
    opCode = (state ? OPC_ACON3 : OPC_ACOF3);
    CANFrame msg;
    msg.len = 8;
    msg.data[0] = opCode;
    msg.data[1] = nn_en[0];
    msg.data[2] = nn_en[1];
    msg.data[3] = nn_en[2];
    msg.data[4] = nn_en[3];
    msg.data[5] = data1;
    msg.data[6] = data2;
    msg.data[7] = data3;
    controller->sendMessage(&msg);
  }
    
}

Processed EventProducerService::handleMessage(unsigned int opc, CANFrame *msg) 
{
  unsigned int nn = (msg->data[1] << 8) + msg->data[2];
  unsigned int en = (msg->data[3] << 8) + msg->data[4];
  // DEBUG_SERIAL << ">VLCBSvc handling message op=" << _HEX(opc) << " nn=" << nn << " en" << en << endl;

  switch (opc) 
  {    

    case OPC_AREQ:
      // AREQ message - request for node state, only producer nodes

      if ((msg->data[1] == highByte(module_config->nodeNum)) && (msg->data[2] == lowByte(module_config->nodeNum))) 
      {
        (void)(*eventhandler)(0, msg);
      }

      return PROCESSED;

    case OPC_ASRQ:



      return PROCESSED;

    default:
      // unknown or unhandled OPC
      // DEBUG_SERIAL << F("> opcode 0x") << _HEX(opc) << F(" is not currently implemented")  << endl;
      return NOT_PROCESSED;
  }
}
}