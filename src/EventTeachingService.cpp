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
  this->module_config = cntrl->module_config;
}

Processed EventTeachingService::handleMessage(unsigned int opc, CANFrame *msg)
{
  unsigned int nn = (msg->data[1] << 8) + msg->data[2];
  unsigned int en = (msg->data[3] << 8) + msg->data[4];
  // DEBUG_SERIAL << ">CbusSvc handling message op=" << _HEX(opc) << " nn=" << nn << " en" << en << endl;

  switch (opc)
  {
    
  case OPC_NNLRN:
    // received NNLRN -- place into learn mode
     DEBUG_SERIAL << F("> NNLRN for node = ") << nn << F(", learn mode on") << endl;

    if (nn == module_config->nodeNum)
    {
      controller->bLearn = true;
      // DEBUG_SERIAL << F("> set lean mode ok") << endl;
      // set bit 5 in parameter 8
      bitSet(controller->_mparams[8], 5);
    }

    return PROCESSED;

  case OPC_EVULN:
    // received EVULN -- unlearn an event, by event number
    // en = (msg->data[3] << 8) + msg->data[4];
    // DEBUG_SERIAL << F("> EVULN for nn = ") << nn << F(", en = ") << en << endl;

    // we must be in learn mode
    if (controller->bLearn == true)
    {
      // DEBUG_SERIAL << F("> searching for existing event to unlearn") << endl;

      // search for this NN and EN pair
      byte index = module_config->findExistingEvent(nn, en);

      if (index < module_config->EE_MAX_EVENTS)
      {
        // TODO: Review this. j is always 0.
        // DEBUG_SERIAL << F("> deleting event at index = ") << index << F(", evs ") << endl;
        module_config->cleareventEEPROM(index);

        // update hash table
        module_config->updateEvHashEntry(index);

        // respond with WRACK
        controller->sendWRACK();
      } else {
        // DEBUG_SERIAL << F("> did not find event to unlearn") << endl;
        // respond with CMDERR
        controller->sendCMDERR(10);
      }
    } // if in learn mode

    return PROCESSED;

  case OPC_NNULN:
    // received NNULN -- exit from learn mode

    if (nn == module_config->nodeNum)
    {
      controller->bLearn = false;
      // DEBUG_SERIAL << F("> NNULN for node = ") << nn << F(", learn mode off") << endl;
      // clear bit 5 in parameter 8
      bitClear(controller->_mparams[8], 5);
    }

    return PROCESSED;

  case OPC_RQEVN:
    // received RQEVN -- request for number of stored events
    // DEBUG_SERIAL << F("> RQEVN -- number of stored events for nn = ") << nn << endl;

    if (nn == module_config->nodeNum)
    {
      // respond with 0x74 NUMEV

      controller->sendMessageWithNN(OPC_NUMEV, module_config->numEvents());
    }

    return PROCESSED;

  case OPC_NERD:
    // request for all stored events
    // DEBUG_SERIAL << F("> NERD : request all stored events for nn = ") << nn << endl;

    if (nn == module_config->nodeNum)
    {
      msg->len = 8;
      msg->data[0] = OPC_ENRSP;                       // response opcode
      msg->data[1] = highByte(module_config->nodeNum);        // my NN hi
      msg->data[2] = lowByte(module_config->nodeNum);         // my NN lo

      for (byte i = 0; i < module_config->EE_MAX_EVENTS; i++)
      {
        if (module_config->getEvTableEntry(i) != 0) {
          // it's a valid stored event

          // read the event data from EEPROM
          // construct and send a ENRSP message
          module_config->readEvent(i, &msg->data[3]);
          msg->data[7] = i;                           // event table index

          // DEBUG_SERIAL << F("> sending ENRSP reply for event index = ") << i << endl;
          controller->sendMessage(msg);
          delay(10);

        } // valid stored ev
      } // loop each ev
    } // for me

    return PROCESSED;

  case OPC_REVAL:
    // received REVAL -- request read of an event variable by event index and ev num
    // respond with NEVAL

    if (nn == module_config->nodeNum)
    {
      if (module_config->getEvTableEntry(msg->data[3]) != 0)
      {
        byte value = module_config->getEventEVval(msg->data[3], msg->data[4]);
        controller->sendMessageWithNN(OPC_NEVAL, msg->data[3], msg->data[4], value);
      }
      else
      {
        // DEBUG_SERIAL << F("> request for invalid event index") << endl;
        controller->sendCMDERR(6);
      }
    }

    return PROCESSED;

  case OPC_NNCLR:
    // NNCLR -- clear all stored events

    if (controller->bLearn == true && nn == module_config->nodeNum)
    {
      // DEBUG_SERIAL << F("> NNCLR -- clear all events") << endl;

      for (byte e = 0; e < module_config->EE_MAX_EVENTS; e++)
      {
        module_config->cleareventEEPROM(e);
      }

      // recreate the hash table
      module_config->clearEvHashTable();
      // DEBUG_SERIAL << F("> cleared all events") << endl;

      controller->sendWRACK();
    }

    return PROCESSED;

  case OPC_NNEVN:
    // request for number of free event slots

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

      // DEBUG_SERIAL << F("> responding to to NNEVN with EVNLF, free event table slots = ") << free_slots << endl;
      controller->sendMessageWithNN(OPC_EVNLF, free_slots);
    }

    return PROCESSED;

  case OPC_EVLRN:
  {
    // received EVLRN -- learn an event
    byte evindex = msg->data[5];
    byte evval = msg->data[6];

    // DEBUG_SERIAL << endl << F("> EVLRN for source nn = ") << nn << F(", en = ") << en << F(", evindex = ") << evindex << F(", evval = ") << evval << endl;

    // we must be in learn mode
    if (controller->bLearn)
    {
      // search for this NN, EN as we may just be adding an EV to an existing learned event
      // DEBUG_SERIAL << F("> searching for existing event to update") << endl;
      byte index = module_config->findExistingEvent(nn, en);

      // not found - it's a new event
      if (index >= module_config->EE_MAX_EVENTS)
      {
        // DEBUG_SERIAL << F("> existing event not found - creating a new one if space available") << endl;
        index = module_config->findEventSpace();
      }

      // if existing or new event space found, write the event data
      if (index < module_config->EE_MAX_EVENTS)
      {
        // write the event to EEPROM at this location -- EVs are indexed from 1 but storage offsets start at zero !!
        // DEBUG_SERIAL << F("> writing EV = ") << evindex << F(", at index = ") << index << F(", offset = ") << (module_config->EE_EVENTS_START + (index * module_config->EE_BYTES_PER_EVENT)) << endl;

        // don't repeat this for subsequent EVs
        if (evindex < 2)
        {
          module_config->writeEvent(index, &msg->data[1]);
        }

        module_config->writeEventEV(index, evindex, evval);

        // recreate event hash table entry
        // DEBUG_SERIAL << F("> updating hash table entry for idx = ") << index << endl;
        module_config->updateEvHashEntry(index);

        // respond with WRACK
        controller->sendWRACK();
      }
      else
      {
        // DEBUG_SERIAL << F("> no free event storage, index = ") << index << endl;
        // respond with CMDERR
        controller->sendCMDERR(10);
      }
    }
    else
    { // bLearn == true
      // DEBUG_SERIAL << F("> error -- not in learn mode") << endl;
    }

    return PROCESSED;
  }

  default:
    // unknown or unhandled OPC
    // DEBUG_SERIAL << F("> opcode 0x") << _HEX(opc) << F(" is not currently implemented")  << endl;
    return NOT_PROCESSED;
  }
}

}