// Copyright (C) David Ellis
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#include "SerialGC.h"
// 3rd party libraries
#include <Streaming.h>
#include <string.h>

//
// Class to transfer CAN frames using the GridConnect protocol over the serial port
//
// GridConnect is a format to encode a bit orientated CAN frame onto a byte orientated serial stream
// The CBUS developers guide describes a slightly modified form of GridConnect, which has been used to 
// enable communtion between computers and can adapters like the CANUSB4 module
// this module follows this convention

// the GridConnect message syntax for a normal message
// : <S | X> <IDENTIFIER> <N> <DATA-0> <DATA-1> … <DATA-7> ;
// where:
//     ':' and ';' are message start and end characters respectively
//     Identifier and data fields are treated as base-16 digits (hexadecimal).
//     'S' & 'X' characters indicates standard or extended CAN frames
//     'N' character indicates a normal (not RTR) message

// the GridConnect message syntax for a 'Request to Transmit' (RTR) message (there is no data field)
// : <S | X> <IDENTIFIER> <R> <LENGTH>;
// where:
//     ':' and ';' are message start and end characters respectively
//     Identifier field is treated as base-16 digits (hexadecimal).
//     'S' & 'X' characters indicates standard or extended CAN frames
//     'R' character indicates an RTR message
//     Length field is a single ASCII decimal character from ‘0’ to ‘8’ that specifies the length
//     of the message

// the difference in the GridConnect version used here is in the format of the Identifier field:
//
// For a standard can message with an 11 bit identifier, the GridConnect identifier field is 4 hex characters,
// i.e. 16 bits
// The CBUS implementation shifts the 11 bit ID by 5, i.e. fills the top bit, with the lower 5 bits being 0
// (This was done to map directly onto SIDH and SIDL registers in the PIC processor family)
// And the GridConnect Identifier field is also leading zero padded, so always 4 characters for a standard message
// 
// For an extended can message with an 29 bit identifier, the GridConnect identifier field is 8 hex characters,
// i.e. 32 bits
// In this case the mapping is a bit more complex
// IDENTIFIER[0,1] - bits [28:21] of the can identifier
// IDENTIFIER[2] - bits [20:18] of the can identifier, left shifted by 1, bottom bit set to zero
// IDENTIFIER[3] - bits [17:16] of the can identifier, bottom two bits, top two bits set to 0
// IDENTIFIER[4,5,6,7] - bits [15:0] of the can identifier
// (This maps directly onto SIDH, SIDL, EIDH and EIDL registers the PIC processor family)
// And the GridConnect Identifier field is also leading zero padded, so always 8 characters for an extended message
// 


namespace VLCB
{


  bool SerialGC::begin()  
  {
    Serial << F("> ** GridConnect over serial ** ") << endl;
    receivedCount = 0;
    transmitCount = 0;
    return true;
  }


  //
  // parse incoming characters & assemble message
  // return true if valid message assembled & ready
  //
  bool SerialGC::available()
  {
    bool result = false;
    static int rxIndex = 0;
    if (Serial.available())
    {     
      char c = Serial.read();
      if(c >= 'a' && c <= 'z') bitClear(c,5);   // ensure letters are upper case
      //
      // if 'start of message' already seen, save the character, and check for 'end of message'
      if (rxIndex > 0) 
      {
        rxBuffer[rxIndex++] = c;
        // check if end of buffer reached, and restart if so
        if (rxIndex >= RXBUFFERSIZE) 
        {
          rxIndex = 0;
        }
        //
        // check for 'end of message'
        if (c == ';')
        {
          rxBuffer[rxIndex++] = '\0';     // null terminate
          rxIndex = 0;
          result = true;
        }
      }
      //
      // always check for 'start of message'
      if (c == ':') 
      {
        rxIndex = 0;                    // restart at beginning of buffer
        rxBuffer[rxIndex++] = c;
      }
    }
    //
    if (result) 
    {
      // We have received a message between a ':' and a ';', so increment count
      receivedCount++;
      result = decodeGridConnect(rxBuffer, &rxCANMessage);
      if (result == false)
      {
        // must have been an error in the message, so increment error counter
        receiveErrorCount++;
      }
    }
    return result;
  }


  //
  /// get the available CANMessage
  /// must call available first to ensure there is something to get
  //
  CANMessage SerialGC::getNextCanMessage()
  {
    return rxCANMessage;
  }


  //
  /// send a CANMessage message in GridConnect format
  // see Gridconnect format at beginning of file for byte positions
  //
  bool SerialGC::sendCanMessage(CANMessage *msg)
  {
    transmitCount++;
    bool result = encodeGridConnect(txBuffer, msg);
    if (result)
    {
      // output the message
      Serial.print(txBuffer);
    }
    else
    {
      transmitErrorCount++;
    }
   return result;
  }


  //
  /// reset
  //
  void SerialGC::reset()
  {
  }


}