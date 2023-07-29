#pragma once

#include <Arduino.h>

namespace VLCB
{

class Controller;
class CANFrame;

class Service
{
public:
  virtual void setController(Controller *controller) {}
  virtual void handleMessage(unsigned int opc, CANFrame *msg, byte remoteCANID) = 0;
};

}

