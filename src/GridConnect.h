
#pragma once

// header files
#include <Arduino.h>
#include <Controller.h>
#include <CanTransport.h>

namespace VLCB
{
  bool decodeGridConnect(const char * gcBuffer, CANFrame *frame);
  bool encodeGridConnect(char * txBuffer, CANFrame *frame);
}
