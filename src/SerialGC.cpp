// Copyright (C) David Ellis
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#include <SerialGC.h>
// 3rd party libraries
#include <Streaming.h>
#include <string.h>

namespace VLCB
{

  //
  /// constructor
  //
  SerialGC::SerialGC()  { }


   void SerialGC::begin()  {
    Serial << F("> ** GridConnect over serial ** ") << endl;
 }

  //
  /// check for unprocessed messages in the buffer
  //
  bool SerialGC::available()
  {
    bool result = false;
    static int rxIndex = 0;
    if (Serial.available()){
      char c = Serial.read();
      //
      // if 'start of message' already seen, save the character, and check for 'end of message'
      if (rxIndex > 0) {
        rxBuffer[rxIndex++] = c;
        //
        // check for 'end of message'
        if (c == ';'){
          rxBuffer[rxIndex++] = '\0';     // null terminate
          Serial << " rxIndex " << rxIndex << "  ";
          Serial << rxBuffer << endl;
          rxIndex = 0;
          result = true;
        }
      }
      //
      // always check for 'start of message'
      if (c == ':') {
        rxIndex = 0;                    // restart at beginning of buffer
        rxBuffer[rxIndex++] = c;
      }
    }
    //
    return result;
  }



  //
  /// get next unprocessed message from the buffer
  /// must call available first to ensure there is something to get
  //
  CANMessage SerialGC::getNextCanMessage()
  {
    CANMessage message;       // CAN frame class
      Serial << " encode can frame ";
/*
    char * gcMSG;
      gcMSG = strtok (rxBuffer,";");
      Serial << gcMSG << endl;
*/
    //message.ext

    return message;
  }

  //
  /// send a VLCB message
  //
  bool SerialGC::sendCanMessage(CANMessage *msg)
  {
    // Gridconnect format of a CAN message with standard Identifier, in hexadecimal
    // :SBBBBCDDDDDDDDDDDDDDDD;
    // 012345678901234567890123 - 24 characters maximum
    // where 'S' represents standard
    // B is the CAN identifier, 4 hex characters for 11 digit identifier
    // and for extended identier
    // :XBBBBBBBBCDDDDDDDDDDDDDDDD;
    // 0123456789012345678901234567 - 28 characters maximum
    // where 'X' represents extended
    // B is the CAN identifier, 8 hex characters for 29 digit identifier
    // in both, 
    // C is 'R' for RTR or 'N' for normal
    // DD are databytes, variable length 0 to 16 (in hexadecimal pairs)

    // start char array for output string
    char str[30];
    byte offset = 0;
    // set starting character & standard or extended CAN identifier
    if (msg->ext) {
      strcpy (str,":X");
      // extended 29 bit CAN idenfier in bytes 2 to 9
      sprintf(str + 2, "%08X", msg->id);
      offset = 10;
    } else {
      strcpy (str,":S");
      // standard 11 bit CAN idenfier in bytes 2 to 5
      sprintf(str + 2, "%04X", msg->id);
      offset = 6;
    }
    // set RTR or normal - byte 6 or 10
    if (msg->rtr) {
      strcpy (str + offset++,"R");
    } else {
      strcpy (str + offset++,"N");
    }
    // add terminator in case len = 0, will be overwritten if len >0
    strcpy (str + offset,";");
    //now data from byte 7 if len > 0
    for (int i=0; i<msg->len; i++){
      sprintf(str + offset + i*2, "%02X", msg->data[i]);
      // append terminator after every data byte - will be overwritten except for last one
      strcpy (str + offset + 2 + i*2,";");
    }
    // output the message
    Serial << str << endl;
    return true;
  }

  //
  /// reset
  //
  void SerialGC::reset()
  {
  }

}