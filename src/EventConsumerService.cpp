// Copyright (C) Martin Da Costa 2023 (martindc.merg@gmail.com)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#include <Streaming.h>

#include "EventConsumerService.h"
#include <Controller.h>
#include <vlcbdefs.hpp>

namespace VLCB {

void EventConsumerService::setController(Controller *cntrl) 
{
  this->controller = cntrl;
  this->module_config = cntrl->getModuleConfig();
}

//
/// register the user handler for learned events
//
void EventConsumerService::setEventHandler(void (*fptr)(byte index, VlcbMessage *msg)) 
{
  eventhandler = fptr;
}

// overloaded form which receives the opcode on/off state and the first event variable
void EventConsumerService::setEventHandler(void (*fptr)(byte index, VlcbMessage *msg, bool ison, byte evval)) 
{
  eventhandlerex = fptr;
}

//
/// for accessory event messages, lookup the event in the event table and call the user's registered event handler function
//
void EventConsumerService::processAccessoryEvent(VlcbMessage *msg, unsigned int nn, unsigned int en, bool is_on_event) 
{
  // try to find a matching stored event -- match on nn, en
  byte index = module_config->findExistingEvent(nn, en);

  // call any registered event handler

  if (index < module_config->EE_MAX_EVENTS) {
    if (eventhandler != NULL) {
      (void)(*eventhandler)(index, msg);
    } else if (eventhandlerex != NULL) {
      byte evVal = (module_config->EE_NUM_EVS > 0) ? module_config->getEventEVval(index, 1) : 0;
      (void)(*eventhandlerex)(index, msg, is_on_event, evVal);
    }
  }
}

void EventConsumerService::process(UserInterface::RequestedAction requestedAction)
{
  if (coeService && (coeService->available()))
  {
    // DEBUG_SERIAL << ">Getting COE Message " << endl;
    VlcbMessage *msg = coeService->get();
    bool done = handleEventMessage(msg);
    // DEBUG_SERIAL << ">COE Message handled " << endl;
  }
}

Processed EventConsumerService::handleEventMessage(VlcbMessage *msg)
{
  return handleEventMessage(msg);
}

Processed EventConsumerService::handleMessage(unsigned int opc, VlcbMessage *msg)
{
  //DEBUG_SERIAL << ">Handle Message " << endl;
  unsigned int nn = (msg->data[1] << 8) + msg->data[2];
  unsigned int en = (msg->data[3] << 8) + msg->data[4];
  // DEBUG_SERIAL << ">ECService handling message op=" << _HEX(opc) << " nn=" << nn << " en" << en << endl;

  switch (opc) 
  {
    case OPC_ACON:
    case OPC_ACON1:
    case OPC_ACON2:
    case OPC_ACON3:

    case OPC_ACOF:
    case OPC_ACOF1:
    case OPC_ACOF2:
    case OPC_ACOF3:

    case OPC_ARON:
    case OPC_AROF:

      // lookup this accessory event in the event table and call the user's registered callback function
      if (eventhandler || eventhandlerex) {
        processAccessoryEvent(msg, nn, en, (opc % 2 == 0));
      }

      return PROCESSED;

    case OPC_ASON:
    case OPC_ASON1:
    case OPC_ASON2:
    case OPC_ASON3:

    case OPC_ASOF:
    case OPC_ASOF1:
    case OPC_ASOF2:
    case OPC_ASOF3:

      // lookup this accessory event in the event table and call the user's registered callback function
      if (eventhandler || eventhandlerex) 
      {
        processAccessoryEvent(msg, 0, en, (opc % 2 == 0));
      }

      return PROCESSED;

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