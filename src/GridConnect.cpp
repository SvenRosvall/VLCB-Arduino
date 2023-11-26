// Copyright (C) David Ellis
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/


//
// Functions to convert between GridConnect format messages and CANMessage objects
//


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

#include <ctype.h>
#include "GridConnect.h"

namespace VLCB
{

  bool encodeGridConnect(char * gcBuffer, CANMessage *msg){
      byte offset = 0;
      gcBuffer[0] = 0;  // null terminate buffer to start with
      // set starting character & standard or extended CAN identifier
      if (msg->ext) {
        if (msg->id > 0x1FFFFFFF)
        {
          // id is greater than 29 bits, so fail the encoding
          return false;
        }
        // mark as extended message
        strcpy (gcBuffer,":X");
        // extended 29 bit CAN idenfier in bytes 2 to 9
        // chars 2 & 3 are ID bits 21 to 28
        sprintf(gcBuffer + 2, "%02X", (msg->id) >> 21);
        // char 4 -  bits 1 to 3 are ID bits 18 to 20
        sprintf(gcBuffer + 4, "%01X", ((msg->id) >> 17) & 0xE);
        // char 5 -  bits 0 to 1 are ID bits 16 & 17
        sprintf(gcBuffer + 5, "%01X", ((msg->id) >> 16) & 0x3);
        // chars 6 to 9 are ID bits 0 to 15
        sprintf(gcBuffer + 6, "%04X", msg->id & 0xFFFF);
        offset = 10;
      } else {// mark sas standard message
        if (msg->id > 0x7FF)
        {
          // id is greater than 11 bits, so fail the encoding
          return false;
        }
        strcpy (gcBuffer,":S");
        // standard 11 bit CAN idenfier in bytes 2 to 5, left shifted 5 to occupy highest bits
        sprintf(gcBuffer + 2, "%04X", msg->id << 5);
        offset = 6;
      }
      // set RTR or normal - byte 6 or 10
      strcpy(gcBuffer + offset++, msg->rtr ? "R" : "N");
      if (msg->len > 8){  // if greater than 8 then faulty msg
        gcBuffer[0] = 0;
        return false;
      }
      //now add hex data from byte 7 if len > 0
      for (int i=0; i<msg->len; i++){
        sprintf(gcBuffer + offset, "%02X", msg->data[i]);
        offset += 2;
      }
      // add terminator
      strcpy (gcBuffer + offset,";");
      return true;
  }

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
  bool checkHexChars(const char *charBuff, int count)
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
  bool decodeGridConnect(const char * gcBuffer, CANMessage *message) 
  {
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
      // now get 29 bit ID - convert from hex, but check they are all hex first
      if (checkHexChars(&gcBuffer[2], 8) == false)
      {
        return false;
      }
      // ok, all hex, so build up id from characters 2 to 9
      // chars 2 & 3 are bits 21 to 28
      message->id = uint32_t(ascii_pair_to_byte(&gcBuffer[2])) << 21;
      // chars 4 & 5 -  bits 5 to 7 are bits 18 to 20
      message->id += uint32_t(ascii_pair_to_byte(&gcBuffer[4]) & 0xE0) << 13;
      // chars 4 & 5 -  bits 0 to 1 are bits 16 & 17
      message->id += uint32_t(ascii_pair_to_byte(&gcBuffer[4]) & 0x3) << 16;
      // chars 6 & 7 are bits 8 to 15
      message->id += uint32_t(ascii_pair_to_byte(&gcBuffer[6])) << 8;
      // chars 8 & 9 are bits 0 to 7 
      message->id += ascii_pair_to_byte(&gcBuffer[8]);
      gcIndex = 10;
    }
    else if (gcBuffer[gcIndex] == 'S') 
    {
      message->ext = false;
      // now get 11 bit ID - convert from hex, but check they are all hex first
      if (checkHexChars(&gcBuffer[2], 4) == false)
      {
        return false;
      }
      // 11 bit identifier needs to be shifted right by 5
      message->id = strtol(&gcBuffer[2], NULL, 16) >> 5;
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
    // set length of data segment
    message->len = dataLength/2;
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





}