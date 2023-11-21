
#pragma once

// header files
#include <Arduino.h>
#include <Controller.h>
#include <CanTransport.h>

namespace VLCB
{
  bool decodeGridConnect(char * gcBuffer, CANMessage *message);
  bool encodeGridConnect(char * txBuffer, CANMessage *msg);
}
