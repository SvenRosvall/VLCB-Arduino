//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

#include "EventSlotTeachingService.h"
#include "Configuration.h"
#include <Controller.h>

namespace VLCB
{

void EventSlotTeachingService::process(const Action &action)
{
  if (action.actionType == ACT_MESSAGE_IN)
  {
    handleMessage(&action.vlcbMessage);
  }
}

void EventSlotTeachingService::handleMessage(const VlcbMessage *msg) 
{
  unsigned int opc = msg->data[0];
  unsigned int nn = Configuration::getTwoBytes(&msg->data[1]);
  //DEBUG_SERIAL << "ets>VlcbSvc handling message op=" << _HEX(opc) << " nn=" << nn << " en" << en << endl;

  switch (opc)
  {
    case OPC_EVLRNI:
      // F5 - learn an event using event index
      handleLearnEventIndex(msg);
      break;

    case OPC_NENRD:
      // 72 - request read stored event by event index
      handleReadEventIndex(nn, msg->data[3]);
      break;

    default:
      AbstractEventTeachingService::handleMessage(msg);
      break;
  }
}

void EventSlotTeachingService::handleLearnEventIndex(const VlcbMessage *msg)
{
  //DEBUG_SERIAL << endl << F("ets> EVLRNI for source nn = ") << nn << endl;

  // we must be in learn mode
  if (!bLearn)
  {
    return;
  }

  controller->messageActedOn();

  if (msg->len < 8)
  {
    controller->sendGRSP(OPC_EVLRNI, getServiceID(), CMDERR_INV_CMD);
    return;
  }

  byte index = msg->data[5];
  byte evIndex = msg->data[6];
  byte evVal = msg->data[7];

  // invalid index
  Configuration *module_config = controller->getModuleConfig();
  if (index >= module_config->getNumEvents())
  {
    //DEBUG_SERIAL << F("> invalid index") << endl;
    controller->sendGRSP(OPC_EVLRNI, getServiceID(), CMDERR_INV_EN_IDX);
    return;
  }
  
  if (evIndex > module_config->getNumEVs())  // Not a valid evIndex
  {
    controller->sendCMDERR(CMDERR_INV_EV_IDX);
    controller->sendGRSP(OPC_EVLRNI, getServiceID(), CMDERR_INV_EV_IDX);
    return;
  }
  
  byte emptyNNEN[] = {0, 0, 0, 0};
  if (Configuration::nnenEquals(emptyNNEN,&msg->data[1]) && evIndex == 0 && evVal == 0)
  {
    // DEBUG_SERIAL << F("ets> deleting event at index = ") << index << F(", evs ") << endl;
    module_config->cleareventEEPROM(index);

    // update hash table
    module_config->updateEvHashEntry(index);

    controller->sendWRACK();
    controller->sendGRSP(OPC_EVLRNI, getServiceID(), GRSP_OK);

    return;
  } 

  // write the event to EEPROM at this location -- EVs are indexed from 1 but storage offsets start at zero !!
  //DEBUG_SERIAL << F("ets> writing EV = ") << evIndex << F(", at index = ") << index << F(", offset = ") << (module_config->EE_EVENTS_START + (index * module_config->EE_BYTES_PER_EVENT)) << endl;

  // Writes the first four bytes NN & EN only if they have changed.
  byte eventTableNNEN[EE_HASH_BYTES];
  module_config->readEvent(index, eventTableNNEN);

  if (!Configuration::nnenEquals(eventTableNNEN, &msg->data[1])
      && !Configuration::nnenEquals(emptyNNEN, &msg->data[1]))
  {
    module_config->writeEvent(index, &msg->data[1]);
    //DEBUG_SERIAL << F("ets> Writing EV Index = ") << index << F(" Node Number ") << (msg->data[1] << 8) + msg->data[2] << F(" Event Number ") << (msg->data[3] << 8) + msg->data[4] <<endl;

    // recreate event hash table entry
    //DEBUG_SERIAL << F("ets> updating hash table entry for idx = ") << index << endl;
    module_config->updateEvHashEntry(index);
  }

  if ( evIndex != 0 )
  {
    module_config->writeEventEV(index, evIndex, evVal);
  }

  // respond with WRACK
  controller->sendWRACK();  // Deprecated in favour of GRSP_OK
  controller->sendGRSP(OPC_EVLRNI, getServiceID(), GRSP_OK);
  ++diagEventsTaught;
}

void EventSlotTeachingService::handleReadEventIndex(unsigned int nn, byte eventIndex)
{
  // DEBUG_SERIAL << F("ets> NERD : request all stored events for nn = ") << nn << endl;

  if (!isThisNodeNumber(nn))
  {
    return;
  }

  controller->messageActedOn();

  Configuration *module_config = controller->getModuleConfig();
  if ((eventIndex >= module_config->getNumEvents()) || (module_config->getEvTableEntry(eventIndex) == 0))
  {
    controller->sendCMDERR(CMDERR_INV_EN_IDX);
    controller->sendGRSP(OPC_NENRD, getServiceID(), CMDERR_INV_EN_IDX);
    return;
  }

  // it's a valid stored event
  // read the event data from EEPROM
  // construct and send a ENRSP message
  VlcbMessage response;
  response.len = 8;
  response.data[0] = OPC_ENRSP;     // response opcode
  response.data[1] = highByte(nn);  // my NN hi
  response.data[2] = lowByte(nn);   // my NN lo
  module_config->readEvent(eventIndex, &response.data[3]);
  response.data[7] = eventIndex;  // event table index

  // DEBUG_SERIAL << F("ets> sending ENRSP reply for event index = ") << eventIndex << endl;
  controller->sendMessage(&response);
}

} // VLCB
