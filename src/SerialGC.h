// Copyright (C) David Ellis
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#pragma once

// header files
#include <Arduino.h>
#include <Controller.h>
#include <CanTransport.h>
#include <GridConnect.h>

namespace VLCB
{

  // constants

  // grid connect should be 28 characters maximum
  static const int RXBUFFERSIZE = 30;

  //
  /// an implementation of the Transport interface class
  /// to support the gridconnect protocol over serial
  //

  class SerialGC : public CanTransport<GCFrame>
  {
  public:

    bool begin();

    bool available() override;
    CANFrame<GCFrame> getNextCanFrame() override;
    bool sendCanFrame(CANFrame<GCFrame> *frame) override;
    void reset() override;

    unsigned int receiveCounter() override { return receivedCount; }
    unsigned int transmitCounter() override { return transmitCount; }
    unsigned int receiveErrorCounter() override { return receiveErrorCount; }
    unsigned int transmitErrorCounter() override { return transmitErrorCount; }
    unsigned int errorStatus() override { return 0; }


  private:

    char rxBuffer[RXBUFFERSIZE]; // Define a byte array to store the incoming data
    char txBuffer[RXBUFFERSIZE]; // Define a byte array to store the outgoing data
    CANFrame<GCFrame> rxCANFrame;

    unsigned int receivedCount = 0;
    unsigned int transmitCount = 0;
    unsigned int receiveErrorCount = 0;
    unsigned int transmitErrorCount = 0;

    void debugCANMessage(CANFrame<GCFrame> frame);

  };

}