// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#include <Streaming.h>

#include "EventTeachingService.h"
#include <Controller.h>
#include <vlcbdefs.hpp>

namespace VLCB {

void EventTeachingService::setController(Controller *cntrl) 
{
  this->controller = cntrl;
  this->module_config = cntrl->getModuleConfig();
}

// Register the user handler for produced event checking
void EventTeachingService::setcheckInputProduced(byte (*fptr)())
{
  checkInputProduced = fptr;
}

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

Processed EventTeachingService::handleMessage(unsigned int opc, VlcbMessage *msg) 
{
  unsigned int nn = (msg->data[1] << 8) + msg->data[2];
  unsigned int en = (msg->data[3] << 8) + msg->data[4];
  //DEBUG_SERIAL << "ets>VlcbSvc handling message op=" << _HEX(opc) << " nn=" << nn << " en" << en << endl;

  switch (opc)
  {
  case OPC_MODE:
    // 76 - Set Operating Mode
    return handleLearnMode(msg);

  case OPC_NNLRN:
    // 53 - place into learn mode
    return handleLearn(nn);

  case OPC_EVULN:
    // 95 - unlearn an event, by event number
    return handleUnlearnEvent(msg, nn, en);

  case OPC_NNULN:
    // 54 - exit from learn mode
    return handleUnlearn(nn);

  case OPC_RQEVN:
    // 58 - request for number of stored events
    return handleRequestEventCount(nn);

  case OPC_NERD:
    // 57 - request for all stored events
    return handleReadEvents(msg, nn);

  case OPC_NENRD:
    // 72 - request read stored event by event index
    return handleReadEventIndex(msg, nn);

  case OPC_REVAL:
    // 9C - request read of an event variable by event index and ev num
    // respond with NEVAL
    return handleReadEventVariable(msg, nn);

  case OPC_NNCLR:
    // 55 - clear all stored events
    return handleClearEvents(nn);

  case OPC_NNEVN:
    // 56 - request for number of free event slots
    return handleGetFreeEventSlots(nn);

  case OPC_EVLRN:
    // D2 - learn an event
    return handleLearnEvent(msg, nn, en);

  case OPC_EVLRNI:
    // F5 - learn an event using event index
    return handleLearnEventIndex(msg);

  case OPC_REQEV:
    // B2 - Read event variable in learn mode
    return handleRequestEventVariable(msg, nn, en);
    
  default:
    // unknown or unhandled OPC
    //DEBUG_SERIAL << F("ets> opcode 0x") << _HEX(opc) << F(" is not currently implemented")  << endl;
    return NOT_PROCESSED;
  }
}

Processed EventTeachingService::handleLearnMode(const VlcbMessage *msg)
{
  //DEBUG_SERIAL << F("ets> MODE -- request op-code received for NN = ") << nn << endl;
  byte requestedMode = msg->data[3];
  switch (requestedMode)
  {
    case MODE_LEARN_ON:
      // Turn on Learn Mode
      enableLearn();
      return PROCESSED;

    case MODE_LEARN_OFF:
      // Turn off Learn Mode
      inhibitLearn();
      return PROCESSED;
    default:
      // if none of the above commands, let another service see message
      return NOT_PROCESSED;
  }
}

Processed EventTeachingService::handleLearn(unsigned int nn)
{
  //DEBUG_SERIAL << F("> NNLRN for node = ") << nn << F(", learn mode on") << endl;

  if (nn == module_config->nodeNum)
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

  return PROCESSED;
}

Processed EventTeachingService::handleUnlearnEvent(const VlcbMessage *msg, unsigned int nn, unsigned int en)
{  
  // DEBUG_SERIAL << F("ets> EVULN for nn = ") << nn << F(", en = ") << en << endl;

  // we must be in learn mode
  if (bLearn == true)
  {
    if (msg->len < 5)
    {
      controller->sendGRSP(OPC_EVULN, getServiceID(), CMDERR_INV_CMD);
      controller->sendCMDERR(CMDERR_INV_CMD);

    }
    else
    {
      // DEBUG_SERIAL << F("ets> searching for existing event to unlearn") << endl;

      // search for this NN and EN pair
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

  return PROCESSED;
}

Processed EventTeachingService::handleUnlearn(unsigned int nn)
{
  //DEBUG_SERIAL << F("ets> NNULN for nn = ") << nn << endl;
  if (nn == module_config->nodeNum)
  {
    inhibitLearn();
  }

  return PROCESSED;
}

Processed EventTeachingService::handleRequestEventCount(unsigned int nn)
{
  // DEBUG_SERIAL << F("ets> RQEVN -- number of stored events for nn = ") << nn << endl;

  if (nn == module_config->nodeNum)
  {
    // respond with 0x74 NUMEV
    controller->sendMessageWithNN(OPC_NUMEV, module_config->numEvents());
  }

  return PROCESSED;
}

Processed EventTeachingService::handleReadEvents(VlcbMessage *msg, unsigned int nn)
{
  //DEBUG_SERIAL << F("ets> NERD : request all stored events for nn = ") << nn << endl;
  
  if (nn == module_config->nodeNum)
  {
    msg->len = 8;
    msg->data[0] = OPC_ENRSP;     // response opcode
    msg->data[1] = highByte(nn);  // my NN hi
    msg->data[2] = lowByte(nn);   // my NN lo

    for (byte i = 0; i < module_config->EE_MAX_EVENTS; i++)
    {
      if (module_config->getEvTableEntry(i) != 0)
      {
        // it's a valid stored event
        // read the event data from EEPROM
        // construct and send a ENRSP message
        module_config->readEvent(i, &msg->data[3]);
        msg->data[7] = i;  // event table index

        //DEBUG_SERIAL << F("> sending ENRSP reply for event index = ") << i << endl;
        controller->sendMessage(msg);
      }  // valid stored ev
    }    // loop each ev
  }      // for me

  return PROCESSED;
}

Processed EventTeachingService::handleReadEventIndex(VlcbMessage *msg, unsigned int nn)
{
  // DEBUG_SERIAL << F("ets> NERD : request all stored events for nn = ") << nn << endl;

  if (nn == module_config->nodeNum)
  {
    byte i = msg->data[3];
    if ((i >= module_config->EE_MAX_EVENTS) && (module_config->getEvTableEntry(i) == 0))
    {
      controller->sendGRSP(OPC_REVAL, getServiceID(), CMDERR_INV_EN_IDX);
      controller->sendCMDERR(CMDERR_INV_EN_IDX);
    }
    else
    {
      // it's a valid stored event
      // read the event data from EEPROM
      // construct and send a ENRSP message
      module_config->readEvent(i, &msg->data[3]);
      msg->len = 8;
      msg->data[0] = OPC_ENRSP;     // response opcode
      msg->data[1] = highByte(nn);  // my NN hi
      msg->data[2] = lowByte(nn);   // my NN lo
      msg->data[7] = i;  // event table index

      // DEBUG_SERIAL << F("ets> sending ENRSP reply for event index = ") << i << endl;
      controller->sendMessage(msg);
    }
  }  // for me

  return PROCESSED;
}

Processed EventTeachingService::handleReadEventVariable(const VlcbMessage *msg, unsigned int nn)
{
  if (nn == module_config->nodeNum)
  {
    if (msg->len < 5)
    {
      controller->sendGRSP(OPC_REVAL, getServiceID(), CMDERR_INV_CMD);
      controller->sendCMDERR(CMDERR_INV_CMD);
    }
    else
    {
      if (module_config->getEvTableEntry(msg->data[3]) != 0)
      {
        byte value = module_config->getEventEVval(msg->data[3], msg->data[4]);
        controller->sendMessageWithNN(OPC_NEVAL, msg->data[3], msg->data[4], value);
      }
      else
      {
        // DEBUG_SERIAL << F("ets> request for invalid event index") << endl;
        controller->sendCMDERR(CMDERR_INV_EV_IDX);
        controller->sendGRSP(OPC_REVAL, getServiceID(), CMDERR_INV_EV_IDX);
      }
    }
  }

  return PROCESSED;
}

Processed EventTeachingService::handleClearEvents(unsigned int nn)
{
  if (nn == module_config->nodeNum)
  {
    if (bLearn == true)
    {
      // DEBUG_SERIAL << F("ets> NNCLR -- clear all events") << endl;

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

  return PROCESSED;
}

Processed EventTeachingService::handleGetFreeEventSlots(unsigned int nn)
{
  if (module_config->nodeNum == nn)
  {
    byte free_slots = 0;

    // count free slots using the event hash table
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

  return PROCESSED;
}

Processed EventTeachingService::handleLearnEvent(VlcbMessage *msg, unsigned int nn, unsigned int en)
{
  // DEBUG_SERIAL << endl << F("ets> EVLRN for source nn = ") << nn << endl;

  // we must be in learn mode
  if (!bLearn)
  {
    return PROCESSED;
  }

  if (msg->len < 7)
  {
    controller->sendGRSP(OPC_EVLRN, getServiceID(), CMDERR_INV_CMD);
    return PROCESSED;
  }

  byte evindex = msg->data[5];
  byte evval = msg->data[6];
  if ((evindex == 0) || (evindex > module_config->EE_NUM_EVS))
  {
    controller->sendCMDERR(CMDERR_INV_EV_IDX);
    controller->sendGRSP(OPC_EVLRN, getServiceID(), CMDERR_INV_EV_IDX);
    return PROCESSED;
  }

  // Is this an existing event? 
  //DEBUG_SERIAL << F("ets> searching for existing event to update") << endl;
  byte index = module_config->findExistingEvent(nn, en);

  // not found in event table
  if (index >= module_config->EE_MAX_EVENTS)
  {
    // Are we a producer and is this a produced event to update?
    if ((controller->getParam(PAR_FLAGS) & PF_PRODUCER) && (evindex == 1) && (checkInputProduced != nullptr))
    {
      // Wait TIME_OUT seconds to see if producer button pressed.
      //DEBUG_SERIAL << F("ets> Update candidate") << endl;
      unsigned long timeStart = millis();
      while (TIME_OUT > millis() - timeStart)
      {
        index = (byte)(*checkInputProduced)();
        if (index < module_config->EE_PRODUCED_EVENTS)
        {
          // DEBUG_SERIAL << F("ets> Update index ") << index << endl;
          break;
        }
      }
    }
          
    if (index >= module_config->EE_PRODUCED_EVENTS)
    {
      //DEBUG_SERIAL << F("ets> Not for update. Create a new event if space available") << endl;
      index = module_config->findEventSpace();
    }
  }

  // if existing or new event space found, write the event data
  if (index >= module_config->EE_MAX_EVENTS)
  {
    // DEBUG_SERIAL << F("ets> no free event storage, index = ") << index << endl;
    // respond with CMDERR & GRSP
    controller->sendCMDERR(CMDERR_TOO_MANY_EVENTS);
    controller->sendGRSP(OPC_EVLRN, getServiceID(), CMDERR_TOO_MANY_EVENTS);
    return PROCESSED;
  }

  // write the event to EEPROM at this location -- EVs are indexed from 1 but storage offsets start at zero !!
  // DEBUG_SERIAL << F("ets> writing EV = ") << evindex << F(", at index = ") << index << F(", offset = ") << (module_config->EE_EVENTS_START + (index * module_config->EE_BYTES_PER_EVENT)) << endl;

  // don't repeat this for subsequent EVs
  if (evindex <= 1)
  {
    module_config->writeEvent(index, &msg->data[1]);
  }

  module_config->writeEventEV(index, evindex, evval);

  // recreate event hash table entry
  // DEBUG_SERIAL << F("ets> updating hash table entry for idx = ") << index << endl;
  module_config->updateEvHashEntry(index);

  // respond with WRACK
  controller->sendWRACK();  // Deprecated in favour of GRSP_OK
  // DEBUG_SERIAL <<F("ets> WRACK sent") << endl;
  
  // Note that the op-code spec only lists WRACK as successful response.
  controller->sendGRSP(OPC_EVLRN, getServiceID(), GRSP_OK);
  return PROCESSED;
}

Processed EventTeachingService::handleLearnEventIndex(VlcbMessage *msg)
{
  //DEBUG_SERIAL << endl << F("ets> EVLRNI for source nn = ") << nn << endl;

  // we must be in learn mode
  if (bLearn)
  {
    if (msg->len < 8)
    {
      controller->sendGRSP(OPC_EVLRN, getServiceID(), CMDERR_INV_CMD);
      controller->sendCMDERR(CMDERR_INV_CMD);
    }
    else
    {
      byte index = msg->data[5];
      byte evIndex = msg->data[6];
      byte evVal = msg->data[7];

      // invalid index
      if (index >= module_config->EE_MAX_EVENTS)
      {
        //DEBUG_SERIAL << F("> invalid index") << endl;
        controller->sendGRSP(OPC_EVLRN, getServiceID(), CMDERR_INV_EV_IDX);
      }
      else if (evIndex == 0)  // Not a valid evIndex
      {
        controller->sendGRSP(OPC_EVLRN, getServiceID(), GRSP_INVALID_COMMAND_PARAMETER);
      }
      else
      {
        // write the event to EEPROM at this location -- EVs are indexed from 1 but storage offsets start at zero !!
        //DEBUG_SERIAL << F("ets> writing EV = ") << evIndex << F(", at index = ") << index << F(", offset = ") << (module_config->EE_EVENTS_START + (index * module_config->EE_BYTES_PER_EVENT)) << endl;

        // Writes the first four bytes NN & EN only if they have changed.
        byte eventTableNNEN[4];
        module_config->readEvent(index, eventTableNNEN);
                if (memcmp(eventTableNNEN, &msg->data[1], 4) != 0)
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
        controller->sendGRSP(OPC_EVLRN, getServiceID(), GRSP_OK);
      }
    }
  }

  return PROCESSED;
}

Processed EventTeachingService::handleRequestEventVariable(VlcbMessage *msg, unsigned int nn, unsigned int en)
{
  if (!bLearn)
  {
    return PROCESSED;
  }

  if (msg->len < 6)
  {
    controller->sendGRSP(OPC_REQEV, getServiceID(), CMDERR_INV_CMD);
    controller->sendCMDERR(CMDERR_INV_CMD);
    return PROCESSED;
  }

  byte index = module_config->findExistingEvent(nn, en);
  byte evIndex = msg->data[5];

  if (index >= module_config->EE_MAX_EVENTS)
  {
    //DEBUG_SERIAL << F("ets> event not found") << endl;
    controller->sendGRSP(OPC_REQEV, getServiceID(), CMDERR_INVALID_EVENT);
    controller->sendCMDERR(CMDERR_INVALID_EVENT);
    return PROCESSED;
  }

  if (evIndex > module_config->EE_NUM_EVS)
  {
    controller->sendCMDERR(CMDERR_INV_EV_IDX);
    controller->sendGRSP(OPC_REQEV, getServiceID(), CMDERR_INV_EV_IDX);
    return PROCESSED;
  }

  // event found
  //DEBUG_SERIAL << F("ets> event found. evIndex = ") << evIndex << endl;
  if (evIndex == 0)
  {
    //send all event variables one after the other starting with the number of variables
    // Reuse the incoming message as it contains the event NN/EN and event index.
    msg->len = 7;
    msg->data[0] = OPC_EVANS;
    msg->data[5] = 0;
    msg->data[6] = module_config->EE_NUM_EVS;
    controller->sendMessage(msg);
    for (byte i = 1; i <= module_config->EE_NUM_EVS; i++)
    {
      msg->data[5] = i;
      msg->data[6] = module_config->getEventEVval(index, i);
      controller->sendMessage(msg);
    }
  }
  else
  {
    // Reuse the incoming message as it contains the event NN/EN and event index.
    msg->len = 7;
    msg->data[0] = OPC_EVANS;
    msg->data[6] = module_config->getEventEVval(index, evIndex);    
    controller->sendMessage(msg);
  }
  return PROCESSED;
}

}
