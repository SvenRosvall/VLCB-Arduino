// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#include <Streaming.h>

#include "EventTeachingService.h"
#include "Configuration.h"
#include <Controller.h>
#include <vlcbdefs.hpp>

namespace VLCB {

void EventTeachingService::enableLearn() 
{
  bLearn = true;
  //DEBUG_SERIAL << F("ets> set learn mode ok") << endl;
  controller->setParamFlag(PF_LRN, true);
}

void EventTeachingService::inhibitLearn() 
{
  bLearn = false;
  //DEBUG_SERIAL << F("ets> learn mode off") << endl;
  controller->setParamFlag(PF_LRN, false);
}

void EventTeachingService::process(const Action *action)
{
  if (action != nullptr && action->actionType == ACT_MESSAGE_IN)
  {
    handleMessage(&action->vlcbMessage);
  }
}

void EventTeachingService::handleMessage(const VlcbMessage *msg) 
{
  unsigned int opc = msg->data[0];
  unsigned int nn = Configuration::getTwoBytes(&msg->data[1]);
  unsigned int en = Configuration::getTwoBytes(&msg->data[3]);
  //DEBUG_SERIAL << "ets>VlcbSvc handling message op=" << _HEX(opc) << " nn=" << nn << " en" << en << endl;

  switch (opc)
  {
  case OPC_MODE:
    // 76 - Set Operating Mode
    handleLearnMode(msg);
    break;

  case OPC_NNLRN:
    // 53 - place into learn mode
    handleLearn(nn);
    break;

  case OPC_EVULN:
    // 95 - unlearn an event, by event number
    handleUnlearnEvent(msg, nn, en);
    break;

  case OPC_NNULN:
    // 54 - exit from learn mode
    handleUnlearn(nn);
    break;

  case OPC_RQEVN:
    // 58 - request for number of stored events
    handleRequestEventCount(nn);
    break;

  case OPC_NERD:
    // 57 - request for all stored events
    handleReadEvents(nn);
    break;

  case OPC_NENRD:
    // 72 - request read stored event by event index
    handleReadEventIndex(nn, msg->data[3]);
    break;

  case OPC_REVAL:
    // 9C - request read of an event variable by event index and ev num
    // respond with NEVAL
    handleReadEventVariable(msg, nn);
    break;

  case OPC_NNCLR:
    // 55 - clear all stored events
    handleClearEvents(nn);
    break;

  case OPC_NNEVN:
    // 56 - request for number of free event slots
    handleGetFreeEventSlots(nn);
    break;

  case OPC_EVLRN:
    // D2 - learn an event
    handleLearnEvent(msg, nn, en);
    break;

  case OPC_EVLRNI:
    // F5 - learn an event using event index
    handleLearnEventIndex(msg);
    break;

  case OPC_REQEV:
    // B2 - Read event variable in learn mode
    handleRequestEventVariable(msg, nn, en);
    break;
  }
}

void EventTeachingService::handleLearnMode(const VlcbMessage *msg)
{
  //DEBUG_SERIAL << F("ets> MODE -- request op-code received for NN = ") << nn << endl;
  byte requestedMode = msg->data[3];
  switch (requestedMode)
  {
    case MODE_LEARN_ON:
      // Turn on Learn Mode
      enableLearn();
      break;

    case MODE_LEARN_OFF:
      // Turn off Learn Mode
      inhibitLearn();
      break;
  }
}

void EventTeachingService::handleLearn(unsigned int nn)
{
  //DEBUG_SERIAL << F("> NNLRN for node = ") << nn << F(", learn mode on") << endl;

  if (isThisNodeNumber(nn))
  {
    enableLearn();
  }
  else  //if we are learning and another node is put in learn mode, stop learn.
  {
    if (bLearn)
    {
      inhibitLearn();
    }
  }
}

void EventTeachingService::handleUnlearnEvent(const VlcbMessage *msg, unsigned int nn, unsigned int en)
{  
  // DEBUG_SERIAL << F("ets> EVULN for nn = ") << nn << F(", en = ") << en << endl;

  // we must be in learn mode
  if (bLearn)
  {
    if (msg->len < 5)
    {
      controller->sendGRSP(OPC_EVULN, getServiceID(), CMDERR_INV_CMD);
    }
    else
    {
      // DEBUG_SERIAL << F("ets> searching for existing event to unlearn") << endl;

      // search for this NN and EN pair
      Configuration *module_config = controller->getModuleConfig();
      byte index = module_config->findExistingEvent(nn, en);

      if (index < module_config->EE_MAX_EVENTS)
      {
        // DEBUG_SERIAL << F("ets> deleting event at index = ") << index << F(", evs ") << endl;
        module_config->cleareventEEPROM(index);

        // update hash table
        module_config->updateEvHashEntry(index);

        // respond with WRACK
        controller->sendWRACK();
        controller->sendGRSP(OPC_EVULN, getServiceID(), GRSP_OK);
      }
      else
      {
        // DEBUG_SERIAL << F("ets> did not find event to unlearn") << endl;
        // respond with CMDERR
        controller->sendCMDERR(CMDERR_INVALID_EVENT);
        controller->sendGRSP(OPC_EVULN, getServiceID(), CMDERR_INVALID_EVENT);
      }
    }
  }  // if in learn mode
}

void EventTeachingService::handleUnlearn(unsigned int nn)
{
  //DEBUG_SERIAL << F("ets> NNULN for nn = ") << nn << endl;
  if (isThisNodeNumber(nn))
  {
    inhibitLearn();
  }
}

void EventTeachingService::handleRequestEventCount(unsigned int nn)
{
  // DEBUG_SERIAL << F("ets> RQEVN -- number of stored events for nn = ") << nn << endl;

  if (isThisNodeNumber(nn))
  {
    // respond with 0x74 NUMEV
    controller->sendMessageWithNN(OPC_NUMEV, controller->getModuleConfig()->numEvents());
  }
}

void EventTeachingService::handleReadEvents(unsigned int nn)
{
  //DEBUG_SERIAL << F("ets> NERD : request all stored events for nn = ") << nn << endl;

  if (isThisNodeNumber(nn))
  {
    VlcbMessage msg;
    msg.len = 8;
    msg.data[0] = OPC_ENRSP;     // response opcode
    msg.data[1] = highByte(nn);  // my NN hi
    msg.data[2] = lowByte(nn);   // my NN lo

    Configuration *module_config = controller->getModuleConfig();
    for (byte i = 0; i < module_config->EE_MAX_EVENTS; i++)
    {
      if (module_config->getEvTableEntry(i) != 0)
      {
        // it's a valid stored event
        // read the event data from EEPROM
        // construct and send a ENRSP message
        module_config->readEvent(i, &msg.data[3]);
        msg.data[7] = i;  // event table index

        //DEBUG_SERIAL << F("> sending ENRSP reply for event index = ") << i << endl;
        controller->sendMessage(&msg);
      }  // valid stored ev
    }    // loop each ev
  }      // for me
}

void EventTeachingService::handleReadEventIndex(unsigned int nn, byte eventIndex)
{
  // DEBUG_SERIAL << F("ets> NERD : request all stored events for nn = ") << nn << endl;

  if (isThisNodeNumber(nn))
  {
    Configuration *module_config = controller->getModuleConfig();
    if ((eventIndex >= module_config->EE_MAX_EVENTS) && (module_config->getEvTableEntry(eventIndex) == 0))
    {
      controller->sendCMDERR(CMDERR_INV_EN_IDX);
      controller->sendGRSP(OPC_NENRD, getServiceID(), CMDERR_INV_EN_IDX);
    }
    else
    {
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
  }  // for me
}

void EventTeachingService::handleReadEventVariable(const VlcbMessage *msg, unsigned int nn)
{
  if (!isThisNodeNumber(nn))
  {
    return;
  }

  if (msg->len < 5)
  {
    controller->sendGRSP(OPC_REVAL, getServiceID(), CMDERR_INV_CMD);
    return;
  }

  uint8_t eventIndex = msg->data[3];
  uint8_t evnum = msg->data[4];

  Configuration *module_config = controller->getModuleConfig();
  if (eventIndex >= module_config->EE_MAX_EVENTS)
  {
    // DEBUG_SERIAL << F("ets> request for invalid event index") << endl;
    controller->sendCMDERR(CMDERR_INV_EN_IDX);
    controller->sendGRSP(OPC_REVAL, getServiceID(), CMDERR_INV_EN_IDX);
    return;
  }

  if (module_config->getEvTableEntry(eventIndex) == 0)
  {
    // DEBUG_SERIAL << F("ets> request for non-existing event") << endl;
    controller->sendCMDERR(CMDERR_INV_EN_IDX);
    controller->sendGRSP(OPC_REVAL, getServiceID(), CMDERR_INV_EN_IDX);
    return;
  }
  
  if (evnum > module_config->EE_NUM_EVS)
  {
    // DEBUG_SERIAL << F("ets> request for invalid event variable") << endl;
    controller->sendCMDERR(CMDERR_INV_EV_IDX);
    controller->sendGRSP(OPC_REVAL, getServiceID(), CMDERR_INV_EV_IDX);
    return;
  }

  if (evnum == 0)
  {
    // Return number of EVs. This may be dynamic in other implementations.
    controller->sendMessageWithNN(OPC_NEVAL, eventIndex, evnum, module_config->EE_NUM_EVS);
    for (byte i = 1; i <= module_config->EE_NUM_EVS; i++)
    {
      byte value = module_config->getEventEVval(eventIndex, i);
      controller->sendMessageWithNN(OPC_NEVAL, eventIndex, i, value);
    }
  }
  else
  {
    byte value = module_config->getEventEVval(eventIndex, evnum);
    controller->sendMessageWithNN(OPC_NEVAL, eventIndex, evnum, value);
  }
}

void EventTeachingService::handleClearEvents(unsigned int nn)
{
  if (isThisNodeNumber(nn))
  {
    if (bLearn)
    {
      // DEBUG_SERIAL << F("ets> NNCLR -- clear all events") << endl;

      Configuration *module_config = controller->getModuleConfig();
      for (byte e = 0; e < module_config->EE_MAX_EVENTS; e++)
      {
        module_config->cleareventEEPROM(e);
      }

      // recreate the hash table
      module_config->clearEvHashTable();
      // DEBUG_SERIAL << F("ets> cleared all events") << endl;
      
      if (controller->getParam(PAR_FLAGS) & PF_PRODUCER)
      {
        module_config->setResetFlag();
      }

      controller->sendWRACK();
      controller->sendGRSP(OPC_NNCLR, getServiceID(), GRSP_OK);
    }
    else
    {
      controller->sendCMDERR(CMDERR_NOT_LRN);
      controller->sendGRSP(OPC_NNCLR, getServiceID(), CMDERR_NOT_LRN);
    }
  }
}

void EventTeachingService::handleGetFreeEventSlots(unsigned int nn)
{
  if (isThisNodeNumber(nn))
  {
    byte free_slots = 0;

    // count free slots using the event hash table
    Configuration *module_config = controller->getModuleConfig();
    for (byte i = 0; i < module_config->EE_MAX_EVENTS; i++)
    {
      if (module_config->getEvTableEntry(i) == 0)
      {
        ++free_slots;
      }
    }

    // DEBUG_SERIAL << F("ets> responding to to NNEVN with EVNLF, free event table slots = ") << free_slots << endl;
    controller->sendMessageWithNN(OPC_EVNLF, free_slots);
  }
}

void EventTeachingService::handleLearnEvent(const VlcbMessage *msg, unsigned int nn, unsigned int en)
{
  // DEBUG_SERIAL << endl << F("ets> EVLRN for source nn = ") << nn << endl;

  // we must be in learn mode
  if (!bLearn)
  {
    return;
  }

  if (msg->len < 7)
  {
    controller->sendGRSP(OPC_EVLRN, getServiceID(), CMDERR_INV_CMD);
    return;
  }

  byte evnum = msg->data[5];
  byte evval = msg->data[6];
  Configuration *module_config = controller->getModuleConfig();
  if ((evnum == 0) || (evnum > module_config->EE_NUM_EVS))
  {
    controller->sendCMDERR(CMDERR_INV_EV_IDX);
    controller->sendGRSP(OPC_EVLRN, getServiceID(), CMDERR_INV_EV_IDX);
    return;
  }
  // Is this a produced event that we know about?
  // Search the events table by evnum = 1 for a value match with evval.
  if ((evnum == 1) && (evval > 0))
  {
    byte index = module_config->findExistingEventByEv(evnum, evval);
    if (index < module_config->EE_MAX_EVENTS)
    {
      module_config->writeEvent(index, &msg->data[1]);
      // no need to write eventEV as, by definition, it hasn't changed
      // recreate event hash table entry
      module_config->updateEvHashEntry(index);
      
      // respond with WRACK
      controller->sendWRACK();  // Deprecated in favour of GRSP_OK
      // DEBUG_SERIAL <<F("ets> WRACK sent") << endl;

      // Note that the op-code spec only lists WRACK as successful response.
      controller->sendGRSP(OPC_EVLRN, getServiceID(), GRSP_OK);
      ++diagEventsTaught;
      return;
    }
  }     
      
  // search for this NN, EN as we may just be adding an EV to an existing learned event 
  //DEBUG_SERIAL << F("ets> searching for existing event to update") << endl;
  byte index = module_config->findExistingEvent(nn, en);

  // not found - it's a new event
  if (index >= module_config->EE_MAX_EVENTS)
  {
    // DEBUG_SERIAL << F("ets> existing event not found - creating a new one if space available") << endl;
    index = module_config->findEventSpace();
  }

  // if existing or new event space found, write the event data
  if (index >= module_config->EE_MAX_EVENTS)
  {
    // DEBUG_SERIAL << F("ets> no free event storage, index = ") << index << endl;
    // respond with CMDERR & GRSP
    controller->sendCMDERR(CMDERR_TOO_MANY_EVENTS);
    controller->sendGRSP(OPC_EVLRN, getServiceID(), CMDERR_TOO_MANY_EVENTS);
    return;
  }

  // write the event to EEPROM at this location -- EVs are indexed from 1 but storage offsets start at zero !!
  // DEBUG_SERIAL << F("ets> writing EV = ") << evnum << F(", at index = ") << index << F(", offset = ") << (module_config->EE_EVENTS_START + (index * module_config->EE_BYTES_PER_EVENT)) << endl;

  // don't repeat this for subsequent EVs
  if (evnum <= 1)
  {
    module_config->writeEvent(index, &msg->data[1]);
  }

  module_config->writeEventEV(index, evnum, evval);

  // recreate event hash table entry
  // DEBUG_SERIAL << F("ets> updating hash table entry for idx = ") << index << endl;
  module_config->updateEvHashEntry(index);

  // respond with WRACK
  controller->sendWRACK();  // Deprecated in favour of GRSP_OK
  // DEBUG_SERIAL <<F("ets> WRACK sent") << endl;
  
  // Note that the op-code spec only lists WRACK as successful response.
  controller->sendGRSP(OPC_EVLRN, getServiceID(), GRSP_OK);
  ++diagEventsTaught;
}

void EventTeachingService::handleLearnEventIndex(const VlcbMessage *msg)
{
  //DEBUG_SERIAL << endl << F("ets> EVLRNI for source nn = ") << nn << endl;

  // we must be in learn mode
  if (bLearn)
  {
    if (msg->len < 8)
    {
      controller->sendGRSP(OPC_EVLRNI, getServiceID(), CMDERR_INV_CMD);
    }
    else
    {
      byte index = msg->data[5];
      byte evIndex = msg->data[6];
      byte evVal = msg->data[7];

      // invalid index
      Configuration *module_config = controller->getModuleConfig();
      if (index >= module_config->EE_MAX_EVENTS)
      {
        //DEBUG_SERIAL << F("> invalid index") << endl;
        controller->sendGRSP(OPC_EVLRNI, getServiceID(), CMDERR_INV_EN_IDX);
      }
      else if ((evIndex == 0) || (evIndex > module_config->EE_NUM_EVS))  // Not a valid evIndex
      {
        controller->sendCMDERR(CMDERR_INV_EV_IDX);
        controller->sendGRSP(OPC_EVLRNI, getServiceID(), CMDERR_INV_EV_IDX);
      }
      else
      {
        // write the event to EEPROM at this location -- EVs are indexed from 1 but storage offsets start at zero !!
        //DEBUG_SERIAL << F("ets> writing EV = ") << evIndex << F(", at index = ") << index << F(", offset = ") << (module_config->EE_EVENTS_START + (index * module_config->EE_BYTES_PER_EVENT)) << endl;

        // Writes the first four bytes NN & EN only if they have changed.
        byte eventTableNNEN[4];
        module_config->readEvent(index, eventTableNNEN);
        if (!Configuration::nnenEquals(eventTableNNEN, &msg->data[1]))
        {
          module_config->writeEvent(index, &msg->data[1]);
          //DEBUG_SERIAL << F("ets> Writing EV Index = ") << index << F(" Node Number ") << (msg->data[1] << 8) + msg->data[2] << F(" Event Number ") << (msg->data[3] << 8) + msg->data[4] <<endl;
        }

        module_config->writeEventEV(index, evIndex, evVal);

        // recreate event hash table entry
        //DEBUG_SERIAL << F("ets> updating hash table entry for idx = ") << index << endl;
        module_config->updateEvHashEntry(index);

        // respond with WRACK
        controller->sendWRACK();  // Deprecated in favour of GRSP_OK
        controller->sendGRSP(OPC_EVLRNI, getServiceID(), GRSP_OK);
        ++diagEventsTaught;
      }
    }
  }
}

void EventTeachingService::handleRequestEventVariable(const VlcbMessage *msg, unsigned int nn, unsigned int en)
{
  if (!bLearn)
  {
    return;
  }

  if (msg->len < 6)
  {
    controller->sendGRSP(OPC_REQEV, getServiceID(), CMDERR_INV_CMD);
    return;
  }

  Configuration *module_config = controller->getModuleConfig();
  byte index = module_config->findExistingEvent(nn, en);
  byte evnum = msg->data[5];

  if (index >= module_config->EE_MAX_EVENTS)
  {
    //DEBUG_SERIAL << F("ets> event not found") << endl;
    controller->sendCMDERR(CMDERR_INVALID_EVENT);
    controller->sendGRSP(OPC_REQEV, getServiceID(), CMDERR_INVALID_EVENT);
    return;
  }

  if (evnum > module_config->EE_NUM_EVS)
  {
    controller->sendCMDERR(CMDERR_INV_EV_IDX);
    controller->sendGRSP(OPC_REQEV, getServiceID(), CMDERR_INV_EV_IDX);
    return;
  }

  // event found
  //DEBUG_SERIAL << F("ets> event found. evnum = ") << evnum << endl;
  VlcbMessage response = *msg;
  response.len = 7;
  response.data[0] = OPC_EVANS;
  if (evnum == 0)
  {
    //send all event variables one after the other starting with the number of variables
    // Reuse the incoming message as it contains the event NN/EN and event index.
    response.data[5] = 0;
    response.data[6] = module_config->EE_NUM_EVS;
    controller->sendMessage(&response);
    for (byte i = 1; i <= module_config->EE_NUM_EVS; i++)
    {
      response.data[5] = i;
      response.data[6] = module_config->getEventEVval(index, i);
      controller->sendMessage(&response);
    }
  }
  else
  {
    // Reuse the incoming message as it contains the event NN/EN and event index.
    response.data[6] = module_config->getEventEVval(index, evnum);    
    controller->sendMessage(&response);
  }
}

}
