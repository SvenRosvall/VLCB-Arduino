
#pragma once

// header files
#include <Arduino.h>
#include <Controller.h>
#include <CanTransport.h>

namespace VLCB
{
  struct GCFrame
  {
  };
  template <>
  struct CANFrame<GCFrame>
  {
    uint32_t id;
    bool ext;
    bool rtr;
    uint8_t len;
    uint8_t data[8];    
  };

  bool decodeGridConnect(const char * gcBuffer, CANFrame<GCFrame> *frame);
  bool encodeGridConnect(char * txBuffer, CANFrame<GCFrame> *frame);
}
