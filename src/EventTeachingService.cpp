//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

#include "EventTeachingService.h"
#include "Configuration.h"
#include <Controller.h>
#include <vlcbdefs.hpp>

namespace VLCB
{

void EventTeachingService::processAction(const Action &action)
{
  if (action.actionType == ACT_MESSAGE_IN)
  {
    handleMessage(&action.vlcbMessage);
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
    case OPC_REQEV:
      // B2 - Read event variable in learn mode
      handleRequestEventVariable(msg, nn, en);
      break;
      
    case OPC_EVLRN:
      // D2 - learn an event
      handleLearnEvent(msg, nn, en);
      break;

    default:
      AbstractEventTeachingService::handleMessage(msg);
      break;
  }
}

class RespondEV : public TimedResponse::Task
{
private:
  Configuration *module_config;
  VlcbMessage response; // A prepopulated response message.
  byte eventIndex;

public:
  RespondEV(Controller * controller, Configuration *module_config, const VlcbMessage & response, byte eventIndex)
    : Task(controller), module_config(module_config), response(response), eventIndex(eventIndex)
  {}
  
  virtual TimedResponse::Result runStep() override
  {
    if (sequence >= module_config->getNumEVs())
    {
      return TimedResponse::FINISHED;
    }
    response.data[5] = sequence + 1;
    response.data[6] = module_config->getEventEVval(eventIndex, sequence + 1);
    controller->sendMessage(&response);
    return TimedResponse::PROGRESS;
  }
};

void EventTeachingService::handleRequestEventVariable(const VlcbMessage *msg, unsigned int nn, unsigned int en)
{
  if (!bLearn)
  {
    return;
  }

  controller->messageActedOn();

  if (msg->len < 6)
  {
    controller->sendGRSP(OPC_REQEV, getServiceID(), CMDERR_INV_CMD);
    return;
  }

  Configuration *module_config = controller->getModuleConfig();
  byte eventIndex = module_config->findExistingEvent(nn, en);
  byte evnum = msg->data[5];

  if (eventIndex >= module_config->getNumEvents())
  {
    //DEBUG_SERIAL << F("ets> event not found") << endl;
    controller->sendCMDERR(CMDERR_INVALID_EVENT);
    controller->sendGRSP(OPC_REQEV, getServiceID(), CMDERR_INVALID_EVENT);
    return;
  }

  if (evnum > module_config->getNumEVs())
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
    response.data[6] = module_config->getNumEVs();
    controller->sendMessage(&response);
    if (!module_config->fcuCompatible)
    {
      controller->addTimedResponseTask(new RespondEV(controller, module_config, response, eventIndex));
    }
  }
  else
  {
    // Reuse the incoming message as it contains the event NN/EN and event index.
    response.data[6] = module_config->getEventEVval(eventIndex, evnum);    
    controller->sendMessage(&response);
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

  controller->messageActedOn();

  if (msg->len < 7)
  {
    controller->sendGRSP(OPC_EVLRN, getServiceID(), CMDERR_INV_CMD);
    return;
  }

  byte evnum = msg->data[5];
  byte evval = msg->data[6];
  Configuration *module_config = controller->getModuleConfig();
  if ((evnum == 0) || (evnum > module_config->getNumEVs()))
  {
    controller->sendCMDERR(CMDERR_INV_EV_IDX);
    controller->sendGRSP(OPC_EVLRN, getServiceID(), CMDERR_INV_EV_IDX);
    return;
  }
  
  if (validatorFunc != nullptr)
  {
    byte errorCode = validatorFunc(nn, en, evnum, evval);
    if (errorCode != GRSP_OK)
    {
      // User defined validation failed.
      controller->sendCMDERR(errorCode);
      controller->sendGRSP(OPC_EVLRN, getServiceID(), errorCode);
      return;
    }
  }
  
  byte index = module_config->findExistingEvent(nn, en);
  //DEBUG_SERIAL << F("> IndexNNEN: ") << index << endl;

  // search for this NN, EN as we may just be adding an EV to an existing learned event 
  //DEBUG_SERIAL << F("ets> searching for existing event to update") << endl;
  // not found - it's a new event
  if (index >= module_config->getNumEvents())
  {
    // DEBUG_SERIAL << F("ets> existing event not found - creating a new one if space available") << endl;
    index = module_config->findEventSpace();

    // if existing or new event space found, write the event data
    if (index >= module_config->getNumEvents())
    {
      // DEBUG_SERIAL << F("ets> no free event storage, index = ") << index << endl;
      // respond with CMDERR & GRSP
      controller->sendCMDERR(CMDERR_TOO_MANY_EVENTS);
      controller->sendGRSP(OPC_EVLRN, getServiceID(), CMDERR_TOO_MANY_EVENTS);
      return;
    }

    // write the event to EEPROM at this location
    module_config->writeEvent(index, &msg->data[1]);

    // recreate event hash table entry
    // DEBUG_SERIAL << F("ets> updating hash table entry for idx = ") << index << endl;
    module_config->updateEvHashEntry(index);
  }

  // DEBUG_SERIAL << F("ets> writing EV = ") << evnum << F(", at index = ") << index << F(", offset = ") << (module_config->EE_EVENTS_START + (index * module_config->EE_BYTES_PER_EVENT)) << endl;
  module_config->writeEventEV(index, evnum, evval);

  // respond with WRACK
  controller->sendWRACK();  // Deprecated in favour of GRSP_OK
  // DEBUG_SERIAL <<F("ets> WRACK sent") << endl;
  
  // Note that the op-code spec only lists WRACK as successful response.
  controller->sendGRSP(OPC_EVLRN, getServiceID(), GRSP_OK);
  ++diagEventsTaught;
}

} // VLCB
