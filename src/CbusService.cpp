
#include <Streaming.h>

#include "CbusService.h"
#include <Controller.h>

// TODO: Included here during refactoring.
#include <LongMessageService.h>

namespace VLCB {

void CbusService::setEventHandler(void (*fptr)(byte index, CANFrame *msg)) {
  eventhandler = fptr;
}

void CbusService::setEventHandler(void (*fptr)(byte index, CANFrame *msg, bool ison, byte evval)) {
  eventhandlerex = fptr;
}

void CbusService::processAccessoryEvent(CANFrame *msg, unsigned int nn, unsigned int en, bool is_on_event) {

  // try to find a matching stored event -- match on nn, en
  byte index = controller->module_config->findExistingEvent(nn, en);

  // call any registered event handler

  if (index < controller->module_config->EE_MAX_EVENTS) {
    if (eventhandler != NULL) {
      (void)(*eventhandler)(index, msg);
    } else if (eventhandlerex != NULL) {
      (void)(*eventhandlerex)(index, msg, is_on_event, \
                              ((controller->module_config->EE_NUM_EVS > 0) ? controller->module_config->getEventEVval(index, 1) : 0) \
                             );
    }
  }
}

void CbusService::handleMessage(unsigned int opc, CANFrame *msg, byte remoteCANID)
{
  unsigned int nn = (msg->data[1] << 8) + msg->data[2];
  unsigned int en = (msg->data[3] << 8) + msg->data[4];

  switch (opc) {

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
    if (eventhandler || eventhandlerex) {
      processAccessoryEvent(msg, 0, en, (opc % 2 == 0));
    }

    break;

  case OPC_RQNP:
    // RQNP message - request for node paramters -- does not contain a NN or EN, so only respond if we
    // are in transition to FLiM
    // DEBUG_SERIAL << F("> RQNP -- request for node params during FLiM transition for NN = ") << nn << endl;

    // only respond if we are in transition to FLiM mode
    if (controller->bModeChanging) {

      // DEBUG_SERIAL << F("> responding to RQNP with PARAMS") << endl;

      // respond with PARAMS message
      msg->len = 8;
      msg->data[0] = OPC_PARAMS;    // opcode
      msg->data[1] = controller->_mparams[1];     // manf code -- MERG
      msg->data[2] = controller->_mparams[2];     // minor code ver
      msg->data[3] = controller->_mparams[3];     // module ident
      msg->data[4] = controller->_mparams[4];     // number of events
      msg->data[5] = controller->_mparams[5];     // events vars per event
      msg->data[6] = controller->_mparams[6];     // number of NVs
      msg->data[7] = controller->_mparams[7];     // major code ver
      // final param[8] = node flags is not sent here as the max message payload is 8 bytes (0-7)
      controller->sendMessage(msg);
    }

    break;

  case OPC_RQNPN:
    // RQNPN message -- request parameter by index number
    // index 0 = number of params available;
    // respond with PARAN

    if (nn == controller->module_config->nodeNum) {

      byte paran = msg->data[3];

      // DEBUG_SERIAL << F("> RQNPN request for parameter # ") << paran << F(", from nn = ") << nn << endl;

      if (paran <= controller->_mparams[0]) {

        paran = msg->data[3];
        controller->sendMessageWithNN(OPC_PARAN, paran, controller->_mparams[paran]);

      } else {
        // DEBUG_SERIAL << F("> RQNPN - param #") << paran << F(" is out of range !") << endl;
        controller->sendCMDERR(9);
      }
    }

    break;

  case OPC_SNN:
    // received SNN - set node number
    // DEBUG_SERIAL << F("> received SNN with NN = ") << nn << endl;

    if (controller->bModeChanging) {
      // DEBUG_SERIAL << F("> buf[1] = ") << msg->data[1] << ", buf[2] = " << msg->data[2] << endl;

      // save the NN
      // controller->controller->module_config->setNodeNum((msg->data[1] << 8) + msg->data[2]);
      controller->module_config->setNodeNum(nn);

      // respond with NNACK
      controller->sendMessageWithNN(OPC_NNACK);

      // DEBUG_SERIAL << F("> sent NNACK for NN = ") << controller->module_config->nodeNum << endl;

      // we are now in FLiM mode - update the configuration
      controller->setFLiM();

      // DEBUG_SERIAL << F("> current mode = ") << controller->module_config->currentMode << F(", node number = ") << controller->module_config->nodeNum << F(", CANID = ") << controller->module_config->CANID << endl;

    } else {
      // DEBUG_SERIAL << F("> received SNN but not in transition") << endl;
    }

    break;

  case OPC_CANID:
    // CAN -- set CANID
    // DEBUG_SERIAL << F("> CANID for nn = ") << nn << F(" with new CANID = ") << msg->data[3] << endl;

    if (nn == controller->module_config->nodeNum) {
      // DEBUG_SERIAL << F("> setting my CANID to ") << msg->data[3] << endl;
      if (msg->data[3] < 1 || msg->data[3] > 99) {
        controller->sendCMDERR(7);
      } else {
        controller->module_config->setCANID(msg->data[3]);
      }
    }

    break;

  case OPC_ENUM:
    // received ENUM -- start CAN bus self-enumeration
    // DEBUG_SERIAL << F("> ENUM message for nn = ") << nn << F(" from CANID = ") << remoteCANID << endl;
    // DEBUG_SERIAL << F("> my nn = ") << controller->module_config->nodeNum << endl;

    if (nn == controller->module_config->nodeNum && remoteCANID != controller->module_config->CANID && !controller->bCANenum) {
      // DEBUG_SERIAL << F("> initiating enumeration") << endl;
      controller->startCANenumeration();
    }

    break;

  case OPC_NVRD:
    // received NVRD -- read NV by index
    if (nn == controller->module_config->nodeNum) {

      byte nvindex = msg->data[3];
      if (nvindex > controller->module_config->EE_NUM_NVS) {
        controller->sendCMDERR(10);
      } else {
        // respond with NVANS
        controller->sendMessageWithNN(OPC_NVANS, nvindex, controller->module_config->readNV(nvindex));
      }
    }

    break;

  case OPC_NVSET:
    // received NVSET -- set NV by index
    // DEBUG_SERIAL << F("> received NVSET for nn = ") << nn << endl;

    if (nn == controller->module_config->nodeNum) {

      if (msg->data[3] > controller->module_config->EE_NUM_NVS) {
        controller->sendCMDERR(10);
      } else {
        // update EEPROM for this NV -- NVs are indexed from 1, not zero
        controller->module_config->writeNV(msg->data[3], msg->data[4]);
        // respond with WRACK
        controller->sendWRACK();
        // DEBUG_SERIAL << F("> set NV ok") << endl;
      }
    }

    break;

  case OPC_NNLRN:
    // received NNLRN -- place into learn mode
    // DEBUG_SERIAL << F("> NNLRN for node = ") << nn << F(", learn mode on") << endl;

    if (nn == controller->module_config->nodeNum) {
      controller->bLearn = true;
      // DEBUG_SERIAL << F("> set lean mode ok") << endl;
      // set bit 5 in parameter 8
      bitSet(controller->_mparams[8], 5);
    }

    break;

  case OPC_EVULN:
    // received EVULN -- unlearn an event, by event number
    // en = (msg->data[3] << 8) + msg->data[4];
    // DEBUG_SERIAL << F("> EVULN for nn = ") << nn << F(", en = ") << en << endl;

    // we must be in learn mode
    if (controller->bLearn == true) {

      // DEBUG_SERIAL << F("> searching for existing event to unlearn") << endl;

      // search for this NN and EN pair
      byte index = controller->module_config->findExistingEvent(nn, en);

      if (index < controller->module_config->EE_MAX_EVENTS) {

        // TODO: Review this. j is always 0.
        // DEBUG_SERIAL << F("> deleting event at index = ") << index << F(", evs ") << endl;
        controller->module_config->cleareventEEPROM(index);

        // update hash table
        controller->module_config->updateEvHashEntry(index);

        // respond with WRACK
        controller->sendWRACK();

      } else {
        // DEBUG_SERIAL << F("> did not find event to unlearn") << endl;
        // respond with CMDERR
        controller->sendCMDERR(10);
      }

    } // if in learn mode

    break;

  case OPC_NNULN:
    // received NNULN -- exit from learn mode

    if (nn == controller->module_config->nodeNum) {
      controller->bLearn = false;
      // DEBUG_SERIAL << F("> NNULN for node = ") << nn << F(", learn mode off") << endl;
      // clear bit 5 in parameter 8
      bitClear(controller->_mparams[8], 5);
    }

    break;

  case OPC_RQEVN:
    // received RQEVN -- request for number of stored events
    // DEBUG_SERIAL << F("> RQEVN -- number of stored events for nn = ") << nn << endl;

    if (nn == controller->module_config->nodeNum) {

      // respond with 0x74 NUMEV

      controller->sendMessageWithNN(OPC_NUMEV, controller->module_config->numEvents());
    }

    break;

  case OPC_NERD:
    // request for all stored events
    // DEBUG_SERIAL << F("> NERD : request all stored events for nn = ") << nn << endl;

    if (nn == controller->module_config->nodeNum) {
      msg->len = 8;
      msg->data[0] = OPC_ENRSP;                       // response opcode
      msg->data[1] = highByte(controller->module_config->nodeNum);        // my NN hi
      msg->data[2] = lowByte(controller->module_config->nodeNum);         // my NN lo

      for (byte i = 0; i < controller->module_config->EE_MAX_EVENTS; i++) {

        if (controller->module_config->getEvTableEntry(i) != 0) {
          // it's a valid stored event

          // read the event data from EEPROM
          // construct and send a ENRSP message
          controller->module_config->readEvent(i, &msg->data[3]);
          msg->data[7] = i;                           // event table index

          // DEBUG_SERIAL << F("> sending ENRSP reply for event index = ") << i << endl;
          controller->sendMessage(msg);
          delay(10);

        } // valid stored ev
      } // loop each ev
    } // for me

    break;

  case OPC_REVAL:
    // received REVAL -- request read of an event variable by event index and ev num
    // respond with NEVAL

    if (nn == controller->module_config->nodeNum) {

      if (controller->module_config->getEvTableEntry(msg->data[3]) != 0) {

        byte value = controller->module_config->getEventEVval(msg->data[3], msg->data[4]);
        controller->sendMessageWithNN(OPC_NEVAL, msg->data[3], msg->data[4], value);
      } else {

        // DEBUG_SERIAL << F("> request for invalid event index") << endl;
        controller->sendCMDERR(6);
      }

    }

    break;

  case OPC_NNCLR:
    // NNCLR -- clear all stored events

    if (controller->bLearn == true && nn == controller->module_config->nodeNum) {

      // DEBUG_SERIAL << F("> NNCLR -- clear all events") << endl;

      for (byte e = 0; e < controller->module_config->EE_MAX_EVENTS; e++) {
        controller->module_config->cleareventEEPROM(e);
      }

      // recreate the hash table
      controller->module_config->clearEvHashTable();
      // DEBUG_SERIAL << F("> cleared all events") << endl;

      controller->sendWRACK();
    }

    break;

  case OPC_NNEVN:
    // request for number of free event slots

    if (controller->module_config->nodeNum == nn) {

      byte free_slots = 0;

      // count free slots using the event hash table
      for (byte i = 0; i < controller->module_config->EE_MAX_EVENTS; i++) {
        if (controller->module_config->getEvTableEntry(i) == 0) {
          ++free_slots;
        }
      }

      // DEBUG_SERIAL << F("> responding to to NNEVN with EVNLF, free event table slots = ") << free_slots << endl;
      controller->sendMessageWithNN(OPC_EVNLF, free_slots);
    }

    break;

  case OPC_QNN:
    // this is probably a config recreate -- respond with PNN if we have a node number
    // DEBUG_SERIAL << F("> QNN received, my node number = ") << controller->module_config->nodeNum << endl;

    if (controller->module_config->nodeNum > 0) {
      // DEBUG_SERIAL << ("> responding with PNN message") << endl;
      controller->sendMessageWithNN(OPC_PNN, controller->_mparams[1], controller->_mparams[3], controller->_mparams[8]);
    }

    break;

  case OPC_RQMN:
    // request for node module name, excluding "CAN" prefix
    // sent during module transition, so no node number check
    // DEBUG_SERIAL << F("> RQMN received") << endl;

    // only respond if in transition to FLiM

    // respond with NAME
    if (controller->bModeChanging) {
      msg->len = 8;
      msg->data[0] = OPC_NAME;
      memcpy(msg->data + 1, controller->_mname, 7);
      controller->sendMessage(msg);
    }

    break;

  case OPC_EVLRN:
  {
    // received EVLRN -- learn an event
    byte evindex = msg->data[5];
    byte evval = msg->data[6];

    // DEBUG_SERIAL << endl << F("> EVLRN for source nn = ") << nn << F(", en = ") << en << F(", evindex = ") << evindex << F(", evval = ") << evval << endl;

    // we must be in learn mode
    if (controller->bLearn) {

      // search for this NN, EN as we may just be adding an EV to an existing learned event
      // DEBUG_SERIAL << F("> searching for existing event to update") << endl;
      byte index = controller->module_config->findExistingEvent(nn, en);

      // not found - it's a new event
      if (index >= controller->module_config->EE_MAX_EVENTS) {
        // DEBUG_SERIAL << F("> existing event not found - creating a new one if space available") << endl;
        index = controller->module_config->findEventSpace();
      }

      // if existing or new event space found, write the event data

      if (index < controller->module_config->EE_MAX_EVENTS) {

        // write the event to EEPROM at this location -- EVs are indexed from 1 but storage offsets start at zero !!
        // DEBUG_SERIAL << F("> writing EV = ") << evindex << F(", at index = ") << index << F(", offset = ") << (controller->module_config->EE_EVENTS_START + (index * controller->module_config->EE_BYTES_PER_EVENT)) << endl;

        // don't repeat this for subsequent EVs
        if (evindex < 2) {
          controller->module_config->writeEvent(index, &msg->data[1]);
        }

        controller->module_config->writeEventEV(index, evindex, evval);

        // recreate event hash table entry
        // DEBUG_SERIAL << F("> updating hash table entry for idx = ") << index << endl;
        controller->module_config->updateEvHashEntry(index);

        // respond with WRACK
        controller->sendWRACK();

      } else {
        // DEBUG_SERIAL << F("> no free event storage, index = ") << index << endl;
        // respond with CMDERR
        controller->sendCMDERR(10);
      }

    } else { // bLearn == true
      // DEBUG_SERIAL << F("> error -- not in learn mode") << endl;
    }

    break;
  }

  case OPC_AREQ:
    // AREQ message - request for node state, only producer nodes

    if ((msg->data[1] == highByte(controller->module_config->nodeNum)) && (msg->data[2] == lowByte(controller->module_config->nodeNum))) {
      (void)(*eventhandler)(0, msg);
    }

    break;

  case OPC_BOOT:
    // boot mode
    break;

  case OPC_RSTAT:
    // command station status -- not applicable to accessory modules
    break;

  // case OPC_ARST:
  // system reset ... this is not what I thought it meant !
  // controller->module_config->reboot();
  // break;

  case OPC_DTXC:
    // Controller long message
    if (controller->longMessageHandler != NULL) {
      controller->longMessageHandler->processReceivedMessageFragment(msg);
    }
    break;

  default:
    // unknown or unhandled OPC
    // DEBUG_SERIAL << F("> opcode 0x") << _HEX(opc) << F(" is not currently implemented")  << endl;
    break;
  }
}

}