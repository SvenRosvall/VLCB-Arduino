#include <Streaming.h>
#include <Controller.h>

namespace VLCB
{

void Controller::handleMessage(unsigned int opc, CANFrame *msg, byte remoteCANID)
{
  unsigned int nn = (_msg.data[1] << 8) + _msg.data[2];
  unsigned int en = (_msg.data[3] << 8) + _msg.data[4];

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
        processAccessoryEvent(nn, en, (opc % 2 == 0));
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
        processAccessoryEvent(0, en, (opc % 2 == 0));
      }

      break;

    case OPC_RQNP:
      // RQNP message - request for node paramters -- does not contain a NN or EN, so only respond if we
      // are in transition to FLiM
      // DEBUG_SERIAL << F("> RQNP -- request for node params during FLiM transition for NN = ") << nn << endl;

      // only respond if we are in transition to FLiM mode
      if (bModeChanging) {

        // DEBUG_SERIAL << F("> responding to RQNP with PARAMS") << endl;

        // respond with PARAMS message
        _msg.len = 8;
        _msg.data[0] = OPC_PARAMS;    // opcode
        _msg.data[1] = _mparams[1];     // manf code -- MERG
        _msg.data[2] = _mparams[2];     // minor code ver
        _msg.data[3] = _mparams[3];     // module ident
        _msg.data[4] = _mparams[4];     // number of events
        _msg.data[5] = _mparams[5];     // events vars per event
        _msg.data[6] = _mparams[6];     // number of NVs
        _msg.data[7] = _mparams[7];     // major code ver
        // final param[8] = node flags is not sent here as the max message payload is 8 bytes (0-7)
        sendMessage(&_msg);
      }

      break;

    case OPC_RQNPN:
      // RQNPN message -- request parameter by index number
      // index 0 = number of params available;
      // respond with PARAN

      if (nn == module_config->nodeNum) {

        byte paran = _msg.data[3];

        // DEBUG_SERIAL << F("> RQNPN request for parameter # ") << paran << F(", from nn = ") << nn << endl;

        if (paran <= _mparams[0]) {

          paran = _msg.data[3];

          _msg.len = 5;
          _msg.data[0] = OPC_PARAN;
          // _msg.data[1] = highByte(module_config->nodeNum);
          // _msg.data[2] = lowByte(module_config->nodeNum);
          _msg.data[3] = paran;
          _msg.data[4] = _mparams[paran];
          sendMessage(&_msg);

        } else {
          // DEBUG_SERIAL << F("> RQNPN - param #") << paran << F(" is out of range !") << endl;
          sendCMDERR(9);
        }
      }

      break;

    case OPC_SNN:
      // received SNN - set node number
      // DEBUG_SERIAL << F("> received SNN with NN = ") << nn << endl;

      if (bModeChanging) {
        // DEBUG_SERIAL << F("> buf[1] = ") << _msg.data[1] << ", buf[2] = " << _msg.data[2] << endl;

        // save the NN
        // module_config->setNodeNum((_msg.data[1] << 8) + _msg.data[2]);
        module_config->setNodeNum(nn);

        // respond with NNACK
        _msg.len = 3;
        _msg.data[0] = OPC_NNACK;
        // _msg.data[1] = highByte(module_config->nodeNum);
        // _msg.data[2] = lowByte(module_config->nodeNum);

        sendMessage(&_msg);

        // DEBUG_SERIAL << F("> sent NNACK for NN = ") << module_config->nodeNum << endl;

        // we are now in FLiM mode - update the configuration
        bModeChanging = false;
        module_config->setModuleMode(MODE_FLIM);
        indicateMode(MODE_FLIM);

        // enumerate the CAN bus to allocate a free CAN ID
        CANenumeration();

        // DEBUG_SERIAL << F("> current mode = ") << module_config->currentMode << F(", node number = ") << module_config->nodeNum << F(", CANID = ") << module_config->CANID << endl;

      } else {
        // DEBUG_SERIAL << F("> received SNN but not in transition") << endl;
      }

      break;

    case OPC_CANID:
      // CAN -- set CANID
      // DEBUG_SERIAL << F("> CANID for nn = ") << nn << F(" with new CANID = ") << _msg.data[3] << endl;

      if (nn == module_config->nodeNum) {
        // DEBUG_SERIAL << F("> setting my CANID to ") << _msg.data[3] << endl;
        if (_msg.data[3] < 1 || _msg.data[3] > 99) {
          sendCMDERR(7);
        } else {
          module_config->setCANID(_msg.data[3]);
        }
      }

      break;

    case OPC_ENUM:
      // received ENUM -- start CAN bus self-enumeration
      // DEBUG_SERIAL << F("> ENUM message for nn = ") << nn << F(" from CANID = ") << remoteCANID << endl;
      // DEBUG_SERIAL << F("> my nn = ") << module_config->nodeNum << endl;

      if (nn == module_config->nodeNum && remoteCANID != module_config->CANID && !bCANenum) {
        // DEBUG_SERIAL << F("> initiating enumeration") << endl;
        CANenumeration();
      }

      break;

    case OPC_NVRD:
      // received NVRD -- read NV by index
      if (nn == module_config->nodeNum) {

        byte nvindex = _msg.data[3];
        if (nvindex > module_config->EE_NUM_NVS) {
          sendCMDERR(10);
        } else {
          // respond with NVANS
          _msg.len = 5;
          _msg.data[0] = OPC_NVANS;
          // _msg.data[1] = highByte(module_config->nodeNum);
          // _msg.data[2] = lowByte(module_config->nodeNum);
          _msg.data[4] = module_config->readNV(nvindex);
          sendMessage(&_msg);
        }
      }

      break;

    case OPC_NVSET:
      // received NVSET -- set NV by index
      // DEBUG_SERIAL << F("> received NVSET for nn = ") << nn << endl;

      if (nn == module_config->nodeNum) {

        if (_msg.data[3] > module_config->EE_NUM_NVS) {
          sendCMDERR(10);
        } else {
          // update EEPROM for this NV -- NVs are indexed from 1, not zero
          module_config->writeNV(_msg.data[3], _msg.data[4]);
          // respond with WRACK
          sendWRACK();
          // DEBUG_SERIAL << F("> set NV ok") << endl;
        }
      }

      break;

    case OPC_NNLRN:
      // received NNLRN -- place into learn mode
      // DEBUG_SERIAL << F("> NNLRN for node = ") << nn << F(", learn mode on") << endl;

      if (nn == module_config->nodeNum) {
        bLearn = true;
        // DEBUG_SERIAL << F("> set lean mode ok") << endl;
        // set bit 5 in parameter 8
        bitSet(_mparams[8], 5);
      }

      break;

    case OPC_EVULN:
      // received EVULN -- unlearn an event, by event number
      // en = (_msg.data[3] << 8) + _msg.data[4];
      // DEBUG_SERIAL << F("> EVULN for nn = ") << nn << F(", en = ") << en << endl;

      // we must be in learn mode
      if (bLearn == true) {

        // DEBUG_SERIAL << F("> searching for existing event to unlearn") << endl;

        // search for this NN and EN pair
        byte index = module_config->findExistingEvent(nn, en);

        if (index < module_config->EE_MAX_EVENTS) {

          // TODO: Review this. j is always 0.
          // DEBUG_SERIAL << F("> deleting event at index = ") << index << F(", evs ") << endl;
          module_config->cleareventEEPROM(index);

          // update hash table
          module_config->updateEvHashEntry(index);

          // respond with WRACK
          sendWRACK();

        } else {
          // DEBUG_SERIAL << F("> did not find event to unlearn") << endl;
          // respond with CMDERR
          sendCMDERR(10);
        }

      } // if in learn mode

      break;

    case OPC_NNULN:
      // received NNULN -- exit from learn mode

      if (nn == module_config->nodeNum) {
        bLearn = false;
        // DEBUG_SERIAL << F("> NNULN for node = ") << nn << F(", learn mode off") << endl;
        // clear bit 5 in parameter 8
        bitClear(_mparams[8], 5);
      }

      break;

    case OPC_RQEVN:
      // received RQEVN -- request for number of stored events
      // DEBUG_SERIAL << F("> RQEVN -- number of stored events for nn = ") << nn << endl;

      if (nn == module_config->nodeNum) {

        // respond with 0x74 NUMEV
        _msg.len = 4;
        _msg.data[0] = OPC_NUMEV;
        // _msg.data[1] = highByte(module_config->nodeNum);
        // _msg.data[2] = lowByte(module_config->nodeNum);
        _msg.data[3] = module_config->numEvents();

        sendMessage(&_msg);
      }

      break;

    case OPC_NERD:
      // request for all stored events
      // DEBUG_SERIAL << F("> NERD : request all stored events for nn = ") << nn << endl;

      if (nn == module_config->nodeNum) {
        _msg.len = 8;
        _msg.data[0] = OPC_ENRSP;                       // response opcode
        _msg.data[1] = highByte(module_config->nodeNum);        // my NN hi
        _msg.data[2] = lowByte(module_config->nodeNum);         // my NN lo

        for (byte i = 0; i < module_config->EE_MAX_EVENTS; i++) {

          if (module_config->getEvTableEntry(i) != 0) {
            // it's a valid stored event

            // read the event data from EEPROM
            // construct and send a ENRSP message
            module_config->readEvent(i, &_msg.data[3]);
            _msg.data[7] = i;                           // event table index

            // DEBUG_SERIAL << F("> sending ENRSP reply for event index = ") << i << endl;
            sendMessage(&_msg);
            delay(10);

          } // valid stored ev
        } // loop each ev
      } // for me

      break;

    case OPC_REVAL:
      // received REVAL -- request read of an event variable by event index and ev num
      // respond with NEVAL

      if (nn == module_config->nodeNum) {

        if (module_config->getEvTableEntry(_msg.data[3]) != 0) {

          _msg.len = 6;
          _msg.data[0] = OPC_NEVAL;
          // _msg.data[1] = highByte(module_config->nodeNum);
          // _msg.data[2] = lowByte(module_config->nodeNum);
          _msg.data[5] = module_config->getEventEVval(_msg.data[3], _msg.data[4]);
          sendMessage(&_msg);
        } else {

          // DEBUG_SERIAL << F("> request for invalid event index") << endl;
          sendCMDERR(6);
        }

      }

      break;

    case OPC_NNCLR:
      // NNCLR -- clear all stored events

      if (bLearn == true && nn == module_config->nodeNum) {

        // DEBUG_SERIAL << F("> NNCLR -- clear all events") << endl;

        for (byte e = 0; e < module_config->EE_MAX_EVENTS; e++) {
          module_config->cleareventEEPROM(e);
        }

        // recreate the hash table
        module_config->clearEvHashTable();
        // DEBUG_SERIAL << F("> cleared all events") << endl;

        sendWRACK();
      }

      break;

    case OPC_NNEVN:
      // request for number of free event slots

      if (module_config->nodeNum == nn) {

        byte free_slots = 0;

        // count free slots using the event hash table
        for (byte i = 0; i < module_config->EE_MAX_EVENTS; i++) {
          if (module_config->getEvTableEntry(i) == 0) {
            ++free_slots;
          }
        }

        // DEBUG_SERIAL << F("> responding to to NNEVN with EVNLF, free event table slots = ") << free_slots << endl;
        // memset(&_msg, 0, sizeof(_msg));
        _msg.len = 4;
        _msg.data[0] = OPC_EVNLF;
        // _msg.data[1] = highByte(module_config->nodeNum);
        // _msg.data[2] = lowByte(module_config->nodeNum);
        _msg.data[3] = free_slots;
        sendMessage(&_msg);
      }

      break;

    case OPC_QNN:
      // this is probably a config recreate -- respond with PNN if we have a node number
      // DEBUG_SERIAL << F("> QNN received, my node number = ") << module_config->nodeNum << endl;

      if (module_config->nodeNum > 0) {
        // DEBUG_SERIAL << ("> responding with PNN message") << endl;
        _msg.len = 6;
        _msg.data[0] = OPC_PNN;
        _msg.data[1] = highByte(module_config->nodeNum);
        _msg.data[2] = lowByte(module_config->nodeNum);
        _msg.data[3] = _mparams[1];
        _msg.data[4] = _mparams[3];
        _msg.data[5] = _mparams[8];
        sendMessage(&_msg);
      }

      break;

    case OPC_RQMN:
      // request for node module name, excluding "CAN" prefix
      // sent during module transition, so no node number check
      // DEBUG_SERIAL << F("> RQMN received") << endl;

      // only respond if in transition to FLiM

      // respond with NAME
      if (bModeChanging) {
        _msg.len = 8;
        _msg.data[0] = OPC_NAME;
        memcpy(_msg.data + 1, _mname, 7);
        sendMessage(&_msg);
      }

      break;

    case OPC_EVLRN: {
      // received EVLRN -- learn an event
      byte evindex = _msg.data[5];
      byte evval = _msg.data[6];

      // DEBUG_SERIAL << endl << F("> EVLRN for source nn = ") << nn << F(", en = ") << en << F(", evindex = ") << evindex << F(", evval = ") << evval << endl;

      // we must be in learn mode
      if (bLearn) {

        // search for this NN, EN as we may just be adding an EV to an existing learned event
        // DEBUG_SERIAL << F("> searching for existing event to update") << endl;
        byte index = module_config->findExistingEvent(nn, en);

        // not found - it's a new event
        if (index >= module_config->EE_MAX_EVENTS) {
          // DEBUG_SERIAL << F("> existing event not found - creating a new one if space available") << endl;
          index = module_config->findEventSpace();
        }

        // if existing or new event space found, write the event data

        if (index < module_config->EE_MAX_EVENTS) {

          // write the event to EEPROM at this location -- EVs are indexed from 1 but storage offsets start at zero !!
          // DEBUG_SERIAL << F("> writing EV = ") << evindex << F(", at index = ") << index << F(", offset = ") << (module_config->EE_EVENTS_START + (index * module_config->EE_BYTES_PER_EVENT)) << endl;

          // don't repeat this for subsequent EVs
          if (evindex < 2) {
            module_config->writeEvent(index, &_msg.data[1]);
          }

          module_config->writeEventEV(index, evindex, evval);

          // recreate event hash table entry
          // DEBUG_SERIAL << F("> updating hash table entry for idx = ") << index << endl;
          module_config->updateEvHashEntry(index);

          // respond with WRACK
          sendWRACK();

        } else {
          // DEBUG_SERIAL << F("> no free event storage, index = ") << index << endl;
          // respond with CMDERR
          sendCMDERR(10);
        }

      } else { // bLearn == true
        // DEBUG_SERIAL << F("> error -- not in learn mode") << endl;
      }

      break;
    }

    case OPC_AREQ:
      // AREQ message - request for node state, only producer nodes

      if ((_msg.data[1] == highByte(module_config->nodeNum)) && (_msg.data[2] == lowByte(module_config->nodeNum))) {
        (void) (*eventhandler)(0, &_msg);
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
      // module_config->reboot();
      // break;

    case OPC_DTXC:
      // Controller long message
      if (longMessageHandler != NULL) {
        longMessageHandler->processReceivedMessageFragment(&_msg);
      }
      break;

    default:
      // unknown or unhandled OPC
      // DEBUG_SERIAL << F("> opcode 0x") << _HEX(opc) << F(" is not currently implemented")  << endl;
      break;
  }
}

}