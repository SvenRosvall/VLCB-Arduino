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

VlcbOpCodes EventProducerService::findEventOpCode(bool state, bool isShortEvent, const EventOpCodeChoices & choices)
{
  if (!isShortEvent)
    if (state)
      return choices.longOn;
    else
      return choices.longOff;
  else
    if (state)
      return choices.shortOn;
    else
      return choices.shortOff;
}

VlcbMessage EventProducerService::createEventMessage(bool state, byte evIndex, const EventOpCodeChoices &opCodeChoices)
{
  byte nn_en[EE_HASH_BYTES];
  controller->getModuleConfig()->readEvent(evIndex, nn_en);

  bool isShortEvent = (nn_en[0] == 0) && (nn_en[1] == 0);
  if (isShortEvent)
  {
    Configuration::setTwoBytes(&nn_en[0], controller->getModuleConfig()->nodeNum);
  }

  VlcbOpCodes opCode = findEventOpCode(state, isShortEvent, opCodeChoices);
  VlcbMessage msg(opCode);
  msg.addNNEN(nn_en);
  return msg;
}

void EventProducerService::sendEventAtIndex(bool state, byte evIndex)
{
  VlcbMessage msg = createEventMessage(state, evIndex, {OPC_ACON, OPC_ACOF, OPC_ASON, OPC_ASOF});
  controller->sendMessage(msg);
  ++diagEventsProduced;
}

void EventProducerService::sendEventAtIndex(bool state, byte evIndex, byte data1)
{
  VlcbMessage msg = createEventMessage(state, evIndex, {OPC_ACON1, OPC_ACOF1, OPC_ASON1, OPC_ASOF1});
  msg.addData(data1);
  controller->sendMessage(msg);
  ++diagEventsProduced;
}

void EventProducerService::sendEventAtIndex(bool state, byte evIndex, byte data1, byte data2)
{
  VlcbMessage msg = createEventMessage(state, evIndex, {OPC_ACON2, OPC_ACOF2, OPC_ASON2, OPC_ASOF2});
  msg.addData(data1);
  msg.addData(data2);
  controller->sendMessage(msg);
  ++diagEventsProduced;
}

void EventProducerService::sendEventAtIndex(bool state, byte evIndex, byte data1, byte data2, byte data3)
{
  VlcbMessage msg = createEventMessage(state, evIndex, {OPC_ACON3, OPC_ACOF3, OPC_ASON3, OPC_ASOF3});
  msg.addData(data1);
  msg.addData(data2);
  msg.addData(data3);
  controller->sendMessage(msg);
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

void EventProducerService::sendEventResponse(bool state, byte evIndex)
{
  VlcbMessage msg = createEventMessage(state, evIndex, {OPC_ARON, OPC_AROF, OPC_ARSON, OPC_ARSOF});
  controller->sendMessage(msg);
  ++diagEventsProduced;
}

void EventProducerService::sendEventResponse(bool state, byte evIndex, byte data1)
{
  VlcbMessage msg = createEventMessage(state, evIndex, {OPC_ARON1, OPC_AROF1, OPC_ARSON1, OPC_ARSOF1});
  msg.addData(data1);
  controller->sendMessage(msg);
  ++diagEventsProduced;
}

void EventProducerService::sendEventResponse(bool state, byte evIndex, byte data1, byte data2)
{
  VlcbMessage msg = createEventMessage(state, evIndex, {OPC_ARON2, OPC_AROF2, OPC_ARSON2, OPC_ARSOF2});
  msg.addData(data1);
  msg.addData(data2);
  controller->sendMessage(msg);
  ++diagEventsProduced;
}

void EventProducerService::sendEventResponse(bool state, byte evIndex, byte data1, byte data2, byte data3)
{
  VlcbMessage msg = createEventMessage(state, evIndex, {OPC_ARON3, OPC_AROF3, OPC_ARSON3, OPC_ARSOF3});
  msg.addData(data1);
  msg.addData(data2);
  msg.addData(data3);
  controller->sendMessage(msg);
  ++diagEventsProduced;
}
}
