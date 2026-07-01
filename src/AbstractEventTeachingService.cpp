// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#include <Streaming.h>

#include "AbstractEventTeachingService.h"
#include "Configuration.h"
#include <Controller.h>
#include <vlcbdefs.hpp>

namespace VLCB {

Service::Data AbstractEventTeachingService::getServiceData()
{
  Configuration *module_config = controller->getModuleConfig();
  return { module_config->getNumEvents(), module_config->getNumEVs(), 0 };
}

void AbstractEventTeachingService::enableLearn() 
{
  bLearn = true;
  //DEBUG_SERIAL << F("ets> set learn mode ok") << endl;
  controller->setParamFlag(PF_LRN, true);
}

void AbstractEventTeachingService::inhibitLearn() 
{
  bLearn = false;
  //DEBUG_SERIAL << F("ets> learn mode off") << endl;
  controller->setParamFlag(PF_LRN, false);
}

void AbstractEventTeachingService::handleMessage(const VlcbMessage *msg) 
{
  unsigned int opc = msg->data[0];
  unsigned int nn = Configuration::getTwoBytes(&msg->data[1]);
  //DEBUG_SERIAL << "ets>VlcbSvc handling message op=" << _HEX(opc) << " nn=" << nn << " en" << en << endl;

  switch (opc)
  {
  case OPC_MODE:
    // 76 - Set Operating Mode
    handleLearnMode(msg, nn);
    break;

  case OPC_NNLRN:
    // 53 - place into learn mode
    handleLearn(nn);
    break;

  case OPC_EVULN:
    // 95 - unlearn an event, by event number
    handleUnlearnEvent(msg, nn);
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
  }
}

void AbstractEventTeachingService::handleLearnMode(const VlcbMessage *msg, unsigned int nn)
{
  //DEBUG_SERIAL << F("ets> MODE -- request op-code received for NN = ") << nn << endl;
  if (!isThisNodeNumber(nn))
  {
    return;
  }

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
      
    default:
      return;
  }
  controller->messageActedOn();
}

void AbstractEventTeachingService::handleLearn(unsigned int nn)
{
  //DEBUG_SERIAL << F("> NNLRN for node = ") << nn << F(", learn mode on") << endl;

  if (isThisNodeNumber(nn))
  {
    enableLearn();
    controller->messageActedOn();
  }
  else  //if we are learning and another node is put in learn mode, stop learn.
  {
    if (bLearn)
    {
      inhibitLearn();
      controller->messageActedOn();
    }
  }
}

void AbstractEventTeachingService::handleUnlearnEvent(const VlcbMessage *msg, unsigned int nn)
{  
  // DEBUG_SERIAL << F("ets> EVULN for nn = ") << nn << F(", en = ") << en << endl;

  // we must be in learn mode
  if (!bLearn)
  {
    return;
  }

  controller->messageActedOn();

  if (msg->len < 5)
  {
    controller->sendGRSP(OPC_EVULN, getServiceID(), CMDERR_INV_CMD);
    return;
  }

  // DEBUG_SERIAL << F("ets> searching for existing event to unlearn") << endl;

  // search for this NN and EN pair
  Configuration *module_config = controller->getModuleConfig();
  unsigned int en = Configuration::getTwoBytes(&msg->data[3]);
  byte index = module_config->findExistingEvent(nn, en);

  if (index >= module_config->getNumEvents())
  {
    // DEBUG_SERIAL << F("ets> did not find event to unlearn") << endl;
    // respond with CMDERR
    controller->sendCMDERR(CMDERR_INVALID_EVENT);
    controller->sendGRSP(OPC_EVULN, getServiceID(), CMDERR_INVALID_EVENT);
    return;
  }

  do
  {
    // DEBUG_SERIAL << F("ets> deleting event at index = ") << index << F(", evs ") << endl;
    module_config->cleareventEEPROM(index);

    // update hash table
    module_config->updateEvHashEntry(index);
    
    // Find next event. (for event slots)
    index = module_config->findExistingEvent(nn, en);
  } while (index < module_config->getNumEvents());

  // respond with WRACK
  controller->sendWRACK();
  controller->sendGRSP(OPC_EVULN, getServiceID(), GRSP_OK);
}

void AbstractEventTeachingService::handleUnlearn(unsigned int nn)
{
  //DEBUG_SERIAL << F("ets> NNULN for nn = ") << nn << endl;
  if (!isThisNodeNumber(nn))
  {
    return;
  }

  controller->messageActedOn();

  inhibitLearn();
}

void AbstractEventTeachingService::handleRequestEventCount(unsigned int nn)
{
  // DEBUG_SERIAL << F("ets> RQEVN -- number of stored events for nn = ") << nn << endl;

  if (!isThisNodeNumber(nn))
  {
    return;
  }

  controller->messageActedOn();

  // respond with 0x74 NUMEV
  controller->sendMessageWithNN(OPC_NUMEV, controller->getModuleConfig()->numEvents());
}

class RespondEvents : public TimedResponse::Task
{
private:
  Configuration *module_config;
  VlcbMessage msg;

public:
  RespondEvents(Controller * controller, Configuration *module_config, int nodeNumber)
    : Task(controller), module_config(module_config)
  {
    msg.len = 8;
    msg.data[0] = OPC_ENRSP;     // response opcode
    msg.data[1] = highByte(nodeNumber);  // my NN hi
    msg.data[2] = lowByte(nodeNumber);   // my NN lo
  }
  
  TimedResponse::Result runStep() override
  {
    if (sequence >= module_config->getNumEvents())
    {
      return TimedResponse::FINISHED;
    }
    if (module_config->getEvTableEntry(sequence) != 0)
    {
      // it's a valid stored event
      // read the event data from EEPROM
      // construct and send a ENRSP message
      module_config->readEvent(sequence, &msg.data[3]);
      msg.data[7] = sequence;  // event table index
      controller->sendMessage(&msg);
    }
    return TimedResponse::PROGRESS;
  }
};

void AbstractEventTeachingService::handleReadEvents(unsigned int nn)
{
  //DEBUG_SERIAL << F("ets> NERD : request all stored events for nn = ") << nn << endl;

  if (!isThisNodeNumber(nn))
  {
    return;
  }

  controller->messageActedOn();

  controller->addTimedResponseTask(new RespondEvents(controller, controller->getModuleConfig(), nn));
}

class RespondEventVar : public TimedResponse::Task
{
  byte eventIndex;
  byte numEVs;
public:
  RespondEventVar(Controller * controller, byte eventIndex, byte numEVs)
    : Task(controller), eventIndex(eventIndex), numEVs(numEVs) {}
  TimedResponse::Result runStep() override
  {
    if (sequence >= numEVs)
    {
      return TimedResponse::FINISHED;
    }
    
    byte value = controller->getModuleConfig()->getEventEVval(eventIndex, sequence + 1);
    controller->sendMessageWithNN(OPC_NEVAL, eventIndex, sequence + 1, value);

    return TimedResponse::PROGRESS;
  }
};

void AbstractEventTeachingService::handleReadEventVariable(const VlcbMessage *msg, unsigned int nn)
{
  if (!isThisNodeNumber(nn))
  {
    return;
  }

  controller->messageActedOn();

  if (msg->len < 5)
  {
    controller->sendGRSP(OPC_REVAL, getServiceID(), CMDERR_INV_CMD);
    return;
  }

  uint8_t eventIndex = msg->data[3];
  uint8_t evnum = msg->data[4];

  Configuration *module_config = controller->getModuleConfig();
  if (eventIndex >= module_config->getNumEvents())
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
  
  if (evnum > module_config->getNumEVs())
  {
    // DEBUG_SERIAL << F("ets> request for invalid event variable") << endl;
    controller->sendCMDERR(CMDERR_INV_EV_IDX);
    controller->sendGRSP(OPC_REVAL, getServiceID(), CMDERR_INV_EV_IDX);
    return;
  }

  if (evnum == 0)
  {
    // Return number of EVs. This may be dynamic in other implementations.
    controller->sendMessageWithNN(OPC_NEVAL, eventIndex, evnum, module_config->getNumEVs());
    if (!module_config->fcuCompatible)
    {
      controller->addTimedResponseTask(new RespondEventVar(controller, eventIndex, module_config->getNumEVs()));
    }
  }
  else
  {
    byte value = module_config->getEventEVval(eventIndex, evnum);
    controller->sendMessageWithNN(OPC_NEVAL, eventIndex, evnum, value);
  }
}

void AbstractEventTeachingService::handleClearEvents(unsigned int nn)
{
  if (!isThisNodeNumber(nn))
  {
    return;
  }

  controller->messageActedOn();

  if (!bLearn)
  {
    controller->sendCMDERR(CMDERR_NOT_LRN);
    controller->sendGRSP(OPC_NNCLR, getServiceID(), CMDERR_NOT_LRN);
    return;
  }

  // DEBUG_SERIAL << F("ets> NNCLR -- clear all events") << endl;

  Configuration *module_config = controller->getModuleConfig();
  for (byte e = 0; e < module_config->getNumEvents(); e++)
  {
    module_config->cleareventEEPROM(e);
  }

  // recreate the hash table
  module_config->clearEvHashTable();
  // DEBUG_SERIAL << F("ets> cleared all events") << endl;
  
  if (module_config->getFlag(PF_PRODUCER))
  {
    module_config->setResetFlag();
  }

  controller->sendWRACK();
  controller->sendGRSP(OPC_NNCLR, getServiceID(), GRSP_OK);
}

void AbstractEventTeachingService::handleGetFreeEventSlots(unsigned int nn)
{
  if (!isThisNodeNumber(nn))
  {
    return;
  }

  controller->messageActedOn();

  byte free_slots = 0;

  // count free slots using the event hash table
  Configuration *module_config = controller->getModuleConfig();
  for (byte i = 0; i < module_config->getNumEvents(); i++)
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
