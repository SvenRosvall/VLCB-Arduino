//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

#include "CanTransport.h"
#include "Controller.h"

namespace VLCB
{

//
/// extract CANID from CAN frame header
//
inline byte getCANID(unsigned long header)
{
  return header & 0x7f;
}

//
/// if in Normal mode, initiate a CAN ID enumeration cycle
//
void CanTransport::startCANenumeration(bool fromENUM)
{
  if (bCANenum)
  {
    // already enumerating.
    return;
  }
  // initiate CAN bus enumeration cycle, either due to ENUM opcode, ID clash, or user button press

  // DEBUG_SERIAL << F("> beginning self-enumeration cycle") << endl;

  // set global variables
  bCANenum = true;                  // we are enumerating
  CANenumTime = millis();           // the cycle start time
  memset(enum_responses, 0, sizeof(enum_responses));
  startedFromEnumMessage = fromENUM;

  sendRtrMessage();

  // DEBUG_SERIAL << F("> enumeration cycle initiated") << endl;
}

void CanTransport::checkIncomingMessage()
{
  // Check concrete transport for messages and put on controller command queue.
  if (!available())
  {
    return;
  }
  CANMessage canMsg = getNextCanMessage();

  // is this an extended frame ? we currently ignore these as bootloader, etc data may confuse us !
  if (canMsg.ext)
  {
    return;
  }

  // is this a CANID enumeration request from another node (RTR set) ?
  if (canMsg.rtr)
  {
    // DEBUG_SERIAL << F("> CANID enumeration RTR from CANID = ") << remoteCANID << endl;
    // send an empty canMsg to show our CANID

    // TODO: introduce a sendMessage() with zero length. Doesn't need a VlcbMessage.
    VlcbMessage message;
    message.len = 0;
    sendMessage(&message);

    return;
  }

  controller->indicateActivity();

  byte remoteCANID = getCANID(canMsg.id);

  /// set flag if we find a CANID conflict with the frame's producer
  /// doesn't apply to RTR or zero-length frames, so as not to trigger an enumeration loop
  if (remoteCANID == controller->getModuleCANID() && canMsg.len > 0)
  {
    // DEBUG_SERIAL << F("> CAN id clash, enumeration required") << endl;
    enumeration_required = true;
  }

  // are we enumerating CANIDs ?
  if (bCANenum && canMsg.len == 0)
  {
    // store this response in the responses array
    if (remoteCANID > 0)
    {
      // fix to correctly record the received CANID
      bitWrite(enum_responses[(remoteCANID / 8)], remoteCANID % 8, 1);
      // DEBUG_SERIAL << F("> stored CANID ") << remoteCANID << F(" at index = ") << (remoteCANID / 8) << F(", bit = ") << (remoteCANID % 8) << endl;
    }

    return;
  }

  // The message is a real message.
  Command cmd = {CMD_MESSAGE_IN, { canMsg.len}};
  memcpy(cmd.vlcbMessage.data, canMsg.data, canMsg.len);

  controller->putCommand(cmd);
}

/// actual implementation of the makeHeader method
/// so it can be called directly or as a Controller class method
/// the 11 bit ID of a standard CAN frame is comprised of: (4 bits of CAN priority) + (7 bits of CAN ID)
/// priority = 1011 (0xB hex, 11 dec) as default argument, which translates to medium/low
inline uint32_t makeHeader_impl(byte id, byte priority)
{
  return (priority << 7) + (id & 0x7f);
}

bool CanTransport::sendMessage(const VlcbMessage *msg)
{
  // caller must populate the message data
  // this method will create the correct frame header (CAN ID and priority bits)
  // rtr and ext default to false unless arguments are supplied - see method definition in .h
  // priority defaults to 1011 low/medium

  CANMessage message;       // ACAN2515 frame class
  message.id = makeHeader_impl(controller->getModuleCANID(), DEFAULT_PRIORITY);
  message.len = msg->len;
  message.rtr = false;
  message.ext = false;
  memcpy(message.data, msg->data, msg->len);

  controller->indicateActivity();
  return sendCanMessage(&message);
}

bool CanTransport::sendRtrMessage()
{
  CANMessage message;       // ACAN2515 frame class
  message.id = makeHeader_impl(controller->getModuleCANID(), DEFAULT_PRIORITY);
  message.rtr = true;
  message.ext = false;
  message.len = 0;

  return sendCanMessage(&message);
}

void CanTransport::checkCANenumTimout()
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

    // store the new CAN ID
    controller->getModuleConfig()->setCANID(selected_id);

    // send NNACK if initiated by ENUM request.
    if (startedFromEnumMessage)
    {
      controller->sendMessageWithNN(OPC_NNACK);
    }
  }
}

byte CanTransport::findFreeCanId()
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
      if (i == 0 && b == 0) 
      {
        continue;
      }

      // if the bit is not set
      if (bitRead(enum_responses[i], b) == 0)
      {
        byte selected_id = ((i * 8) + b);
        // DEBUG_SERIAL << F("> bit ") << b << F(" of byte ") << i << F(" is not set, first free CAN ID = ") << selected_id << endl;
        return selected_id;
      }
    }
  }

  return 1;     // default if no responses from other modules
}

void CanTransport::process()
{
  checkIncomingMessage();

  if (enumeration_required)
  {
    // DEBUG_SERIAL << "> enumeration flag set" << endl;
    enumeration_required = false;
    startCANenumeration();
  }

  checkCANenumTimout();
}

}
