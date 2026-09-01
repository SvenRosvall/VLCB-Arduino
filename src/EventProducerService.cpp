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

void EventProducerService::processAction(const Action & action)
{
  if (action.actionType == ACT_MESSAGE_IN)
  {
    handleProdSvcMessage(&action.vlcbMessage);
  }
}

void EventProducerService::sendShortEvent(bool state, int eventNumber)
{
  controller->sendMessage(VlcbMessage(state ? OPC_ASON : OPC_ASOF).addNN(controller->getModuleConfig()->nodeNum).addEN(eventNumber));
  ++diagEventsProduced;
}

void EventProducerService::sendLongEvent(bool state, int eventNumber)
{
  controller->sendMessage(VlcbMessage(state ? OPC_ACON : OPC_ACOF).addNN(controller->getModuleConfig()->nodeNum).addEN(eventNumber));
  ++diagEventsProduced;
}

void EventProducerService::sendLongEventWithSpoofedNodeNumber(bool state, int nodeNumber, int eventNumber)
{
  controller->sendMessage(VlcbMessage(state ? OPC_ACON : OPC_ACOF).addNN(nodeNumber).addEN(eventNumber));
  ++diagEventsProduced;
}

static VlcbOpCodes findEventOpCode(bool isResponse, bool state, bool isShortEvent, int nDataBytes)
{
  int opCode = OPC_ACON; // Starting point
  if (!state)
  {
    opCode++; // OFF op-codes are one higher.
  }
  if (isShortEvent)
  {
    opCode += 8;
  }
  if (isResponse)
  {
    if (isShortEvent)
    {
      opCode += 5;
    }
    else
    {
      opCode += 3;
    }
  }
  opCode += 0x20 * nDataBytes;
  return (VlcbOpCodes) opCode;
}

void EventProducerService::sendEventAtIndexVarData(bool isResponse, bool state, byte evIndex,
                                                   int dataLen, byte data1=0, byte data2=0, byte data3=0)
{
  byte nn_en[EE_HASH_BYTES];
  controller->getModuleConfig()->readEvent(evIndex, nn_en);

  bool isShortEvent = (nn_en[0] == 0) && (nn_en[1] == 0);
  if (isShortEvent)
  {
    Configuration::setTwoBytes(&nn_en[0], controller->getModuleConfig()->nodeNum);
  }

  VlcbOpCodes opCode = findEventOpCode(isResponse, state, isShortEvent, dataLen);
  VlcbMessage msg(opCode);
  msg.addNNEN(nn_en);
  
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

  controller->sendMessage(msg);
  ++diagEventsProduced;
}

void EventProducerService::sendEventAtIndex(bool state, byte evIndex)
{
  sendEventAtIndexVarData(false, state, evIndex, 0);
}

void EventProducerService::sendEventAtIndex(bool state, byte evIndex, byte data1)
{
  sendEventAtIndexVarData(false, state, evIndex, 1, data1);
}

void EventProducerService::sendEventAtIndex(bool state, byte evIndex, byte data1, byte data2)
{
  sendEventAtIndexVarData(false, state, evIndex, 2, data1, data2);
}

void EventProducerService::sendEventAtIndex(bool state, byte evIndex, byte data1, byte data2, byte data3)
{
  sendEventAtIndexVarData(false, state, evIndex, 3, data1, data2, data3);
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

void EventProducerService::sendEventResponse(bool state, byte evIndex)
{
  sendEventAtIndexVarData(true, state, evIndex, 0);
}

void EventProducerService::sendEventResponse(bool state, byte evIndex, byte data1)
{
  sendEventAtIndexVarData(true, state, evIndex, 1, data1);
}

void EventProducerService::sendEventResponse(bool state, byte evIndex, byte data1, byte data2)
{
  sendEventAtIndexVarData(true, state, evIndex, 2, data1, data2);
}

void EventProducerService::sendEventResponse(bool state, byte evIndex, byte data1, byte data2, byte data3)
{
  sendEventAtIndexVarData(true, state, evIndex, 3, data1, data2, data3);
}
}
