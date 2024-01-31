// Copyright (C) David Ellis
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#include "SerialGC.h"
// 3rd party libraries
#include <Streaming.h>
#include <string.h>
#include <ctype.h>

//
// Class to transfer CAN frames using the GridConnect protocol over the serial port
//
// see GridConnect.cpp for more details on this protocol

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
      c = toupper(c);
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
      result = decodeGridConnect(rxBuffer, &rxCANFrame);
      if (!result)
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
  CANFrame SerialGC::getNextCanFrame()
  {
    return rxCANFrame;
  }


  //
  /// send a CANMessage message in GridConnect format
  // see Gridconnect format at beginning of file for byte positions
  //
  bool SerialGC::sendCanFrame(CANFrame *frame)
  {
    transmitCount++;
    bool result = encodeGridConnect(txBuffer, frame);
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