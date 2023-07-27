
/*

  Copyright (C) Duncan Greenwood 2017 (duncan_greenwood@hotmail.com)

  This work is licensed under the:
      Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
   To view a copy of this license, visit:
      http://creativecommons.org/licenses/by-nc-sa/4.0/
   or send a letter to Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.

   License summary:
    You are free to:
      Share, copy and redistribute the material in any medium or format
      Adapt, remix, transform, and build upon the material

    The licensor cannot revoke these freedoms as long as you follow the license terms.

    Attribution : You must give appropriate credit, provide a link to the license,
                  and indicate if changes were made. You may do so in any reasonable manner,
                  but not in any way that suggests the licensor endorses you or your use.

    NonCommercial : You may not use the material for commercial purposes. **(see note below)

    ShareAlike : If you remix, transform, or build upon the material, you must distribute
                 your contributions under the same license as the original.

    No additional restrictions : You may not apply legal terms or technological measures that
                                 legally restrict others from doing anything the license permits.

   ** For commercial use, please contact the original copyright holder(s) to agree licensing terms

    This software is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE

*/

// 3rd party libraries
#include <Streaming.h>

// Controller library
#include <Controller.h>

//
/// construct a Controller object with an external Configuration object named "config" that is defined
/// in user code
//

namespace VLCB
{

Controller::Controller() {
  extern Configuration config;
  module_config = &config;

  eventhandler = NULL;
  eventhandlerex = NULL;
  framehandler = NULL;
}

//
/// construct a Controller object with a Configuration object that the user provides.
/// note that this Configuration object must have a lifetime longer than the Controller object.
//

Controller::Controller(Configuration *the_config) {
  module_config = the_config;

  eventhandler = NULL;
  eventhandlerex = NULL;
  framehandler = NULL;
}

//
/// register the user handler for learned events
//

void Controller::setEventHandler(void (*fptr)(byte index, CANFrame *msg)) {
  eventhandler = fptr;
}

// overloaded form which receives the opcode on/off state and the first event variable

void Controller::setEventHandler(void (*fptr)(byte index, CANFrame *msg, bool ison, byte evval)) {
  eventhandlerex = fptr;
}

//
/// register the user handler for CAN frames
/// default args in .h declaration for opcodes array (NULL) and size (0)
//

void Controller::setFrameHandler(void (*fptr)(CANFrame *msg), byte opcodes[], byte num_opcodes) {
  framehandler = fptr;
  _opcodes = opcodes;
  _num_opcodes = num_opcodes;
}

//
/// assign the module parameter set
//

void Controller::setParams(unsigned char *mparams) {
  _mparams = mparams;
}

//
/// assign the module name
//

void Controller::setName(unsigned char *mname) {
  _mname = mname;
}

//
/// set module to SLiM mode
//

void Controller::setSLiM(void) {

  bModeChanging = false;
  module_config->setNodeNum(0);
  module_config->setFLiM(false);
  module_config->setCANID(0);

  indicateMode(module_config->FLiM);
}

//
/// extract CANID from CAN frame header
//

inline byte Controller::getCANID(unsigned long header) {

  return header & 0x7f;
}

//
/// send a WRACK (write acknowledge) message
//

bool Controller::sendWRACK(void) {

  // send a write acknowledgement response

  _msg.len = 3;
  _msg.data[0] = OPC_WRACK;
  _msg.data[1] = highByte(module_config->nodeNum);
  _msg.data[2] = lowByte(module_config->nodeNum);

  return sendMessage(&_msg);
}

//
/// send a CMDERR (command error) message
//

bool Controller::sendCMDERR(byte cerrno) {

  // send a command error response

  _msg.len = 4;
  _msg.data[0] = OPC_CMDERR;
  _msg.data[1] = highByte(module_config->nodeNum);
  _msg.data[2] = lowByte(module_config->nodeNum);
  _msg.data[3] = cerrno;

  return sendMessage(&_msg);
}

//
/// is this an Extended CAN frame ?
//

bool Controller::isExt(CANFrame *amsg) {

  return (amsg->ext);
}

//
/// is this a Remote frame ?
//

bool Controller::isRTR(CANFrame *amsg) {

  return (amsg->rtr);
}

//
/// if in FLiM mode, initiate a CAN ID enumeration cycle
//

void Controller::CANenumeration(void) {

  // initiate CAN bus enumeration cycle, either due to ENUM opcode, ID clash, or user button press

  // DEBUG_SERIAL << F("> beginning self-enumeration cycle") << endl;

  // set global variables
  bCANenum = true;                  // we are enumerating
  CANenumTime = millis();           // the cycle start time
  memset(enum_responses, 0, sizeof(enum_responses));

  // send zero-length RTR frame
  _msg.len = 0;
  sendMessage(&_msg, true, false);          // fixed arg order in v 1.1.4, RTR - true, ext = false

  // DEBUG_SERIAL << F("> enumeration cycle initiated") << endl;
}

//
/// initiate the transition from SLiM to FLiM mode
//

void Controller::initFLiM(void) {

  // DEBUG_SERIAL << F("> initiating FLiM negotation") << endl;

  indicateMode(MODE_CHANGING);

  bModeChanging = true;
  timeOutTimer = millis();

  // send RQNN message with current NN, which may be zero if a virgin/SLiM node
  _msg.len = 3;
  _msg.data[0] = OPC_RQNN;
  _msg.data[1] = highByte(module_config->nodeNum);
  _msg.data[2] = lowByte(module_config->nodeNum);
  sendMessage(&_msg);

  // DEBUG_SERIAL << F("> requesting NN with RQNN message for NN = ") << module_config->nodeNum << endl;
}

//
/// revert from FLiM to SLiM mode
//

void Controller::revertSLiM(void) {

  // DEBUG_SERIAL << F("> reverting to SLiM mode") << endl;

  // send NNREL message
  _msg.len = 3;
  _msg.data[0] = OPC_NNREL;
  _msg.data[1] = highByte(module_config->nodeNum);
  _msg.data[2] = lowByte(module_config->nodeNum);

  sendMessage(&_msg);
  setSLiM();
}

//
/// change or re-confirm node number
//

void Controller::renegotiate(void) {

  initFLiM();
}

void Controller::setUI(UserInterface *ui)
{
  _ui = ui;
}

//
/// set the Controller LEDs to indicate the current mode
//

void Controller::indicateMode(byte mode) {

  // DEBUG_SERIAL << F("> indicating mode = ") << mode << endl;
  if (_ui) {
    _ui->indicateMode(mode);
  }
}

void Controller::indicateActivity()
{
  if (_ui) {
    _ui->indicateActivity();
  }
}

/// main Controller message processing procedure

void Controller::process(byte num_messages) {

  unsigned int j = 0;

  // start bus enumeration if required
  if (enumeration_required) {
    enumeration_required = false;
    CANenumeration();
  }

  // process switch operations if the module is configured with one

  if (_ui) {

    // allow LEDs and switch to update
    _ui->run();

    //
    /// use LEDs to indicate that the user can release the switch
    //

    if (_ui->resetRequested()) {
      //DEBUG_SERIAL << "> Button is pressed for mode change" << endl;
      indicateMode(MODE_CHANGING);
    }

    //
    /// handle switch state changes
    //

    UserInterface::RequestedAction requestedAction = _ui->checkRequestedAction();
    switch (requestedAction)
    {
      case UserInterface::CHANGE_MODE:
        // initiate mode change
        //Serial << "Controller::process() - changing mode, current isFlim=" << module_config->FLiM << endl;
        if (!module_config->FLiM) {
          initFLiM();
        } else {
          revertSLiM();
        }
        break;

      case UserInterface::RENEGOTIATE:
        //Serial << "Controller::process() - renegotiate" << endl;
        renegotiate();
        break;

      case UserInterface::ENUMERATION:
        //Serial << "Controller::process() - enumerate" << endl;
        if (module_config->FLiM)
        {
          CANenumeration();
        }
        break;

      case UserInterface::NONE:
        //Serial << "Controller::process() - no action" << endl;
        break;
    }
  }

  // get received CAN frames from buffer
  // process by default 3 messages per run so the user's application code doesn't appear unresponsive under load

  byte mcount = 0;

  while (available() && mcount < num_messages) {

    ++mcount;

    // at least one CAN frame is available in the reception buffer
    // retrieve the next one

    // memset(&_msg, 0, sizeof(CANFrame));
    _msg = transport->getNextMessage();

    // extract OPC, NN, EN
    unsigned int opc = _msg.data[0];
    unsigned int nn = (_msg.data[1] << 8) + _msg.data[2];
    unsigned int en = (_msg.data[3] << 8) + _msg.data[4];

    //
    /// extract the CANID of the sending module
    //

    byte remoteCANID = getCANID(_msg.id);

    //
    /// if registered, call the user handler with this new frame
    //

    if (framehandler != NULL) {

      // check if incoming opcode is in the user list, if list length > 0
      if (_num_opcodes > 0) {
        for (byte i = 0; i < _num_opcodes; i++) {
          if (opc == _opcodes[i]) {
            (void)(*framehandler)(&_msg);
            break;
          }
        }
      } else {
        (void)(*framehandler)(&_msg);
      }
    }

    //
    /// pulse the green LED
    //

    indicateActivity();

    // is this a CANID enumeration request from another node (RTR set) ?
    if (_msg.rtr) {
      // DEBUG_SERIAL << F("> CANID enumeration RTR from CANID = ") << remoteCANID << endl;
      // send an empty message to show our CANID
      _msg.len = 0;
      sendMessage(&_msg);
      continue;
    }

    //
    /// set flag if we find a CANID conflict with the frame's producer
    /// doesn't apply to RTR or zero-length frames, so as not to trigger an enumeration loop
    //

    if (remoteCANID == module_config->CANID && _msg.len > 0) {
      // DEBUG_SERIAL << F("> CAN id clash, enumeration required") << endl;
      enumeration_required = true;
    }

    // is this an extended frame ? we currently ignore these as bootloader, etc data may confuse us !
    if (_msg.ext) {
      // DEBUG_SERIAL << F("> extended frame ignored, from CANID = ") << remoteCANID << endl;
      continue;
    }

    // are we enumerating CANIDs ?
    if (bCANenum && _msg.len == 0) {

      // store this response in the responses array
      if (remoteCANID > 0) {
        // fix to correctly record the received CANID
        bitWrite(enum_responses[(remoteCANID / 16)], remoteCANID % 8, 1);
        // DEBUG_SERIAL << F("> stored CANID ") << remoteCANID << F(" at index = ") << (remoteCANID / 8) << F(", bit = ") << (remoteCANID % 8) << endl;
      }

      continue;
    }

    //
    /// process the message opcode
    /// if we got this far, it's a standard CAN frame (not extended, not RTR) with a data payload length > 0
    //

    if (_msg.len > 0) {

      byte index;

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
          module_config->setFLiM(true);
          indicateMode(module_config->FLiM);

          // enumerate the CAN bus to allocate a free CAN ID
          CANenumeration();

          // DEBUG_SERIAL << F("> FLiM mode = ") << module_config->FLiM << F(", node number = ") << module_config->nodeNum << F(", CANID = ") << module_config->CANID << endl;

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
            module_config->writeNV( _msg.data[3], _msg.data[4]);
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
          index = module_config->findExistingEvent(nn, en);

          if (index < module_config->EE_MAX_EVENTS) {

            // TODO: Review this. j is always 0.
            // DEBUG_SERIAL << F("> deleting event at index = ") << index << F(", evs ") << endl;
            module_config->cleareventEEPROM(j);

            // update hash table
            module_config->updateEvHashEntry(j);

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

      case OPC_EVLRN:
      {
        // received EVLRN -- learn an event
        byte evindex = _msg.data[5];
        byte evval = _msg.data[6];

        // DEBUG_SERIAL << endl << F("> EVLRN for source nn = ") << nn << F(", en = ") << en << F(", evindex = ") << evindex << F(", evval = ") << evval << endl;

        // we must be in learn mode
        if (bLearn) {

          // search for this NN, EN as we may just be adding an EV to an existing learned event
          // DEBUG_SERIAL << F("> searching for existing event to update") << endl;
          index = module_config->findExistingEvent(nn, en);

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
          (void)(*eventhandler)(0, &_msg);
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
    } else {
      // DEBUG_SERIAL << F("> oops ... zero - length frame ?? ") << endl;
    }
  }  // while messages available

  // check CAN bus enumeration timer
  checkCANenum();

  //
  /// check 30 sec timeout for SLiM/FLiM negotiation with FCU
  //

  if (bModeChanging && ((millis() - timeOutTimer) >= 30000)) {

    // DEBUG_SERIAL << F("> timeout expired, FLiM = ") << FLiM << F(", mode change = ") << bModeChanging << endl;
    indicateMode(module_config->FLiM);
    bModeChanging = false;
  }

  // DEBUG_SERIAL << F("> end of opcode processing, time = ") << (micros() - mtime) << "us" << endl;

  //
  /// end of Controller message processing
  //
}

void Controller::checkCANenum(void) {

  //
  /// check the 100ms CAN enumeration cycle timer
  //

  byte selected_id = 1;     // default if no responses from other modules

  // if (bCANenum && !bCANenumComplete && (millis() - CANenumTime) >= 100) {
  if (bCANenum && (millis() - CANenumTime) >= 100) {

    // enumeration timer has expired -- stop enumeration and process the responses

    // DEBUG_SERIAL << F("> enum cycle complete at ") << millis() << F(", start = ") << CANenumTime << F(", duration = ") << (millis() - CANenumTime) << endl;
    // DEBUG_SERIAL << F("> processing received responses") << endl;

    // iterate through the 128 bit field
    for (byte i = 0; i < 16; i++) {

      // ignore if this byte is all 1's -> there are no unused IDs in this group of numbers
      if (enum_responses[i] == 0xff) {
        continue;
      }

      // for each bit in the byte
      for (byte b = 0; b < 8; b++) {

        // ignore first bit of first byte -- CAN ID zero is not used for nodes
        if (i == 0 && b == 0) {
          continue;
        }

        // if the bit is not set
        if (bitRead(enum_responses[i], b) == 0) {
          selected_id = ((i * 16) + b);
          // DEBUG_SERIAL << F("> bit ") << b << F(" of byte ") << i << F(" is not set, first free CAN ID = ") << selected_id << endl;
          // i = 16; // ugh ... but probably better than a goto :)
          // but using a goto saves 4 bytes of program size ;)
          goto check_done;
        }
      }
    }

check_done:

    // DEBUG_SERIAL << F("> enumeration responses = ") << enums << F(", lowest available CAN id = ") << selected_id << endl;

    // bCANenumComplete = true;
    bCANenum = false;
    CANenumTime = 0UL;

    // store the new CAN ID
    module_config->setCANID(selected_id);

    // send NNACK
    _msg.len = 3;
    _msg.data[0] = OPC_NNACK;
    _msg.data[1] = highByte(module_config->nodeNum);
    _msg.data[2] = lowByte(module_config->nodeNum);
    sendMessage(&_msg);
  }
}

//
/// for accessory event messages, lookup the event in the event table and call the user's registered event handler function
//

void Controller::processAccessoryEvent(unsigned int nn, unsigned int en, bool is_on_event) {

  // try to find a matching stored event -- match on nn, en
  byte index = module_config->findExistingEvent(nn, en);

  // call any registered event handler

  if (index < module_config->EE_MAX_EVENTS) {
    if (eventhandler != NULL) {
      (void)(*eventhandler)(index, & _msg);
    } else if (eventhandlerex != NULL) {
      (void)(*eventhandlerex)(index, & _msg, is_on_event, \
                              ((module_config->EE_NUM_EVS > 0) ? module_config->getEventEVval(index, 1) : 0) \
                             );
    }
  }
}

//
/// set the long message handler object to receive long message frames
//

void Controller::setLongMessageHandler(LongMessageController *handler) {
  longMessageHandler = handler;
}

}