//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

#include <Streaming.h>
#include "CanService.h"
#include "Controller.h"
#include <cbusdefs.h>

namespace VLCB
{

enum CanOpCodes
{
  CAN_OP_ENUM = OPC_ENUM,
  CAN_OP_CANID = OPC_CANID
};

void CanService::setController(Controller *cntrl)
{
  this->controller = cntrl;
  this->module_config = cntrl->module_config;
}

//
/// extract CANID from CAN frame header
//
byte CanService::getCANID(unsigned long header)
{
  return header & 0x7f;
}

//
/// if in FLiM mode, initiate a CAN ID enumeration cycle
//
void CanService::startCANenumeration()
{
  // initiate CAN bus enumeration cycle, either due to ENUM opcode, ID clash, or user button press

  // DEBUG_SERIAL << F("> beginning self-enumeration cycle") << endl;

  // set global variables
  bCANenum = true;                  // we are enumerating
  CANenumTime = millis();           // the cycle start time
  memset(enum_responses, 0, sizeof(enum_responses));

  // send zero-length RTR frame
  CANFrame msg;
  msg.len = 0;
  controller->sendMessage(&msg, true, false);          // fixed arg order in v 1.1.4, RTR - true, ext = false

  // DEBUG_SERIAL << F("> enumeration cycle initiated") << endl;
}

void CanService::checkCANenumTimout()
{
  //
  /// check the 100ms CAN enumeration cycle timer
  //
  if (bCANenum && (millis() - CANenumTime) >= 100)
  {
    // enumeration timer has expired -- stop enumeration and process the responses

    // DEBUG_SERIAL << F("> enum cycle complete at ") << millis() << F(", start = ") << CANenumTime << F(", duration = ") << (millis() - CANenumTime) << endl;
    // DEBUG_SERIAL << F("> processing received responses") << endl;

    byte selected_id = findFreeCanId();

    // DEBUG_SERIAL << F("> lowest available CAN id = ") << selected_id << endl;

    bCANenum = false;
    CANenumTime = 0UL;

    // store the new CAN ID
    module_config->setCANID(selected_id);

    // send NNACK
    controller->sendMessageWithNN(OPC_NNACK);
  }
}

byte CanService::findFreeCanId()
{
  // iterate through the 128 bit field
  for (byte i = 0; i < 16; i++)
  {
    // ignore if this byte is all 1's -> there are no unused IDs in this group of numbers
    if (enum_responses[i] == 0xff)
    {
      continue;
    }

    // for each bit in the byte
    for (byte b = 0; b < 8; b++)
    {
      // ignore first bit of first byte -- CAN ID zero is not used for nodes
      if (i == 0 && b == 0) {
        continue;
      }

      // if the bit is not set
      if (bitRead(enum_responses[i], b) == 0)
      {
        byte selected_id = ((i * 16) + b);
        // DEBUG_SERIAL << F("> bit ") << b << F(" of byte ") << i << F(" is not set, first free CAN ID = ") << selected_id << endl;
        return selected_id;
      }
    }
  }

  return 1;     // default if no responses from other modules
}

void CanService::process(UserInterface::RequestedAction requestedAction)
{
  if (enumeration_required)
  {
    // DEBUG_SERIAL << "> enumeration flag set" << endl;
    enumeration_required = false;
    startCANenumeration();
  }

  if (requestedAction == UserInterface::ENUMERATION)
  {
    // DEBUG_SERIAL << "> User request - enumerate" << endl;
    if (module_config->currentMode != MODE_SLIM)
    {
      startCANenumeration();
    }
  }

  checkCANenumTimout();
}

Processed CanService::handleRawMessage(CANFrame *msg)
{
  byte remoteCANID = getCANID(msg->id);

  // is this a CANID enumeration request from another node (RTR set) ?
  if (msg->rtr)
  {
    // DEBUG_SERIAL << F("> CANID enumeration RTR from CANID = ") << remoteCANID << endl;
    // send an empty message to show our CANID
    msg->len = 0;
    controller->sendMessage(msg);
    return PROCESSED;
  }

  /// set flag if we find a CANID conflict with the frame's producer
  /// doesn't apply to RTR or zero-length frames, so as not to trigger an enumeration loop
  if (remoteCANID == module_config->CANID && msg->len > 0)
  {
    // DEBUG_SERIAL << F("> CAN id clash, enumeration required") << endl;
    enumeration_required = true;
  }

  // are we enumerating CANIDs ?
  if (bCANenum && msg->len == 0)
  {

    // store this response in the responses array
    if (remoteCANID > 0)
    {
      // fix to correctly record the received CANID
      bitWrite(enum_responses[(remoteCANID / 16)], remoteCANID % 8, 1);
      // DEBUG_SERIAL << F("> stored CANID ") << remoteCANID << F(" at index = ") << (remoteCANID / 8) << F(", bit = ") << (remoteCANID % 8) << endl;
    }

    return PROCESSED;
  }

  return NOT_PROCESSED;
}

Processed CanService::handleMessage(unsigned int opc, CANFrame *msg)
{
  unsigned int nn = (msg->data[1] << 8) + msg->data[2];

  switch (opc)
  {

    case CAN_OP_CANID:
      // CAN -- set CANID
      // DEBUG_SERIAL << F("> CANID for nn = ") << nn << F(" with new CANID = ") << msg->data[3] << endl;

      if (nn == module_config->nodeNum)
      {
        // DEBUG_SERIAL << F("> setting my CANID to ") << msg->data[3] << endl;
        byte newCANID = msg->data[3];
        if (newCANID < 1 || newCANID > 99)
        {
          controller->sendCMDERR(GRSP_INVALID_PARAMETER);
          controller->sendGRSP(CAN_OP_CANID, getServiceID(), GRSP_INVALID_PARAMETER);
        }
        else
        {
          module_config->setCANID(newCANID);
          controller->sendWRACK();
          controller->sendGRSP(CAN_OP_CANID, getServiceID(), GRSP_OK);
        }
      }

      return PROCESSED;

    case CAN_OP_ENUM:
      // received ENUM -- start CAN bus self-enumeration
      {
        byte remoteCANID = getCANID(msg->id);

        // DEBUG_SERIAL << F("> ENUM message for nn = ") << nn << F(" from CANID = ") << remoteCANID << endl;
        // DEBUG_SERIAL << F("> my nn = ") << module_config->nodeNum << endl;

        if (nn == module_config->nodeNum && remoteCANID != module_config->CANID && !bCANenum)
        {
          // DEBUG_SERIAL << F("> initiating enumeration") << endl;
          startCANenumeration();
        }
      }

      return PROCESSED;

    default:
      return NOT_PROCESSED;
  }
}

}