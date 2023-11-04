// Copyright (C) David Ellis
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#include <SerialGC.h>
// 3rd party libraries
#include <Streaming.h>
#include <string.h>

//
// Class to transfer CAN messages using the GridConnect protocol over the serial port
//

// there are two basic types of CAN message, standard and extended
//
// Gridconnect format of a CAN message with standard Identifier
// :SBBBBCDDDDDDDDDDDDDDDD;
// 012345678901234567890123 - 24 characters maximum
// where 'S' represents standard
// B is the CAN identifier, 4 hex characters for 11 digit identifier

// Gridconnect format of a CAN message with  extended identier
// :XBBBBBBBBCDDDDDDDDDDDDDDDD;
// 0123456789012345678901234567 - 28 characters maximum
// where 'X' represents extended
// B is the CAN identifier, 8 hex characters for 29 digit identifier

// in both types
// C is 'R' for RTR or 'N' for normal
// DD are databytes, variable length 0 to 16 (in hexadecimal pairs)


namespace VLCB
{

  // Function to convert a pair of hexadecimal characters to a byte value
  //
  int ascii_pair_to_byte(const char *pair)
  {
      unsigned char* data = (unsigned char*)pair;
      int result;
      if (data[1] < 'A') { result = data[1] - '0'; }
      else { result = data[1] - 'A' + 10; }
      if (data[0] < 'A') { result += (data[0] - '0') << 4; }
      else { result += (data[0] - 'A' + 10) << 4; }
      return result;
  }

  // check supplied array is comprised of only hexadecimal characters
  //
  bool checkHexChars(char *charBuff, int count)
  {
    for (int i = 0 ; i< count; i++)
    {
      if (!isxdigit(charBuff[i]))
      {
        return false;
      }
    }
    return true;
  }

  // convert a gridconnect message to CANMessage object
  // see Gridconnect format at beginning of file for byte positions
  //
  bool encodeCANMessage(char * gcBuffer, CANMessage *message) 
  {
    //Serial << " encode gridconnect message "<< gcBuffer << endl;

    int gcIndex = 0;                          // index used to 'walk' gc message
    int gcBufferLength = strlen(gcBuffer);    // save for later use

    // must have start of message character
    if (gcBuffer[gcIndex++] != ':') 
    {
      return false;
    }
    //
    // do CAN Identifier, must be either 'X' or 'S'
    if (gcBuffer[gcIndex] == 'X') 
    {
      message->ext = true;
      // now get ID - convert from hex, but check they are all hex first
      if (checkHexChars(&gcBuffer[2], 8) == false)
      {
        return false;
      }
      message->id = strtol(&gcBuffer[2], NULL, 16);
      gcIndex = 10;
    }
    else if (gcBuffer[gcIndex] == 'S') 
    {
      message->ext = false;
      // now get ID - convert from hex, but check they are all hex first
      if (checkHexChars(&gcBuffer[2], 4) == false)
      {
        return false;
      }
      message->id = strtol(&gcBuffer[2], NULL, 16);
      gcIndex = 6;
    } 
    else 
    {
      return false;
    }
    //
    // do RTR flag
    if (gcBuffer[gcIndex] == 'R') 
    {
      message->rtr = true;
    } 
    else if (gcBuffer[gcIndex] == 'N')
    {
      message->rtr = false;
    } 
    else 
    {
      return false;
    }
    gcIndex++;  // set to next character afert RTR flag
    //
    // Do data segment - convert hex array to byte array
    // find out how many chars in data segment (0 to 16, in multiples of 2) 
    // should be gcBufferLength minus gcIndex as it is now (after RTR flag), minus 1
    int dataLength = gcBufferLength - gcIndex - 1;
    // must be even number of hex characters, and no more than 16
    if ((dataLength % 2 ) || (dataLength > 16 )) 
    {
      return false;
    } 
    else 
    {
      message->len = dataLength/2;
    }
    // now convert hex data into bytes
    for (int i = 0; i < dataLength/2; i++) 
    {
      // check they are hex chars first
      if (checkHexChars(&gcBuffer[gcIndex], 2) == false)
      {
        return false;
      }
      message->data[i] = ascii_pair_to_byte(&gcBuffer[gcIndex]);
      gcIndex += 2;
    }
    //
    // must have end of message character
    if (gcBuffer[gcBufferLength-1] != ';') 
    {
      return false;
    }
    //
    return true; 
  }

  // Function to output a debug CANMessage to Serial
  //
  void debugCANMessage(CANMessage message)
  {
    Serial << endl << "CANMessage:";
    Serial << " id " << message.id << " length " << message.len;
    Serial << " data ";
    for (int i=0; i <message.len; i++) {
      if( i>0 ) Serial << ",";
      Serial << message.data[i];
    }
    Serial << endl;
  }


  bool SerialGC::begin()  
  {
    Serial << F("> ** GridConnect over serial ** ") << endl;
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
      result = encodeCANMessage(rxBuffer, &rxCANMessage);
    }
    //
    return result;
  }


  //
  /// get the available CANMessage
  /// must call available first to ensure there is something to get
  //
  CANMessage SerialGC::getNextCanMessage()
  {
    debugCANMessage(rxCANMessage);
    return rxCANMessage;
  }

  //
  /// send a CANMessage message in GridConnect format
  // see Gridconnect format at beginning of file for byte positions
  //
  bool SerialGC::sendCanMessage(CANMessage *msg)
  {
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
    Serial.print(str);
    return true;
  }

  //
  /// reset
  //
  void SerialGC::reset()
  {
  }


}