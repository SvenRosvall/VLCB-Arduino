// Copyright (C) Martin Da Costa 2023 (martindc.merg@gmail.com)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#include <Streaming.h>

#include "EventConsumerService.h"
#include <Controller.h>
#include <vlcbdefs.hpp>

namespace VLCB {

//
/// register the user handler for learned events
//
void EventConsumerService::setEventHandler(void (*fptr)(byte index, const VlcbMessage *msg)) 
{
  eventhandler = fptr;
}

//
/// for accessory event messages, lookup the event in the event table and call the user's registered event handler function
//
void EventConsumerService::processAccessoryEvent(const VlcbMessage *msg, unsigned int nn, unsigned int en) 
{
  // try to find a matching stored event -- match on nn, en
  byte index = controller->getModuleConfig()->findExistingEvent(nn, en);

  // call any registered event handler

  if (index < controller->getModuleConfig()->EE_MAX_EVENTS)
  {
    if (eventhandler != nullptr)
    {
      (void)(*eventhandler)(index, msg);
    }
  }
}

void EventConsumerService::process(const Action *action)
{
  if (action == nullptr)
  {
    return;
  }

  switch (action->actionType)
  {
    case ACT_MESSAGE_OUT:
      if (!(controller->getParam(PAR_FLAGS) & PF_COE))
      {
        break;
      }
      // else Fall through: A message sent out should also be picked up by the consumer service.

    case ACT_MESSAGE_IN:
      handleConsumedMessage(&action->vlcbMessage);
      break;
      
    default:
      break;
  }
}

void EventConsumerService::handleConsumedMessage(const VlcbMessage *msg)
{
  //DEBUG_SERIAL << ">Handle Message " << endl;
  unsigned int opc = msg->data[0];
  unsigned int nn = Configuration::getTwoBytes(&msg->data[1]);
  unsigned int en = Configuration::getTwoBytes(&msg->data[3]);
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
      if (eventhandler) 
      {
        processAccessoryEvent(msg, nn, en);
      }
      break;

    case OPC_ASON:
    case OPC_ASON1:
    case OPC_ASON2:
    case OPC_ASON3:

    case OPC_ASOF:
    case OPC_ASOF1:
    case OPC_ASOF2:
    case OPC_ASOF3:

      // lookup this accessory event in the event table and call the user's registered callback function
      if (eventhandler) 
      {
        processAccessoryEvent(msg, 0, en);
      }
      break;

    default:
      // unknown or unhandled OPC
      // DEBUG_SERIAL << F("> opcode 0x") << _HEX(opc) << F(" is not currently implemented")  << endl;
      break;
  }
}
}