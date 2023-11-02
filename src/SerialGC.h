// Copyright (C) David Ellis
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#pragma once

// header files

#include <Controller.h>
#include <CanTransport.h>

namespace VLCB
{

  // constants


  //
  /// an implementation of the Transport interface class
  /// to support the gridconnect protocol over serial
  //

  class SerialGC : public CanTransport
  {
  public:

    SerialGC();

    void begin();

    bool available() override;
    CANMessage getNextCanMessage() override;
    bool sendCanMessage(CANMessage *msg) override;
    void reset() override;


  private:

    char rxBuffer[100]; // Define a byte array to store the incoming data
    bool encodeCANMessage(char *gcBuffer, CANMessage *message);
    CANMessage rxCANMessage;
    int ascii_pair_to_byte(const char *pair);

  };

}