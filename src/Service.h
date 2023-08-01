#pragma once

#include <Arduino.h>

namespace VLCB
{

class Controller;
class CANFrame;

enum Processed {
  NOT_PROCESSED=0,
  PROCESSED=1
};

class Service
{
public:
  virtual void setController(Controller *controller) {}
  virtual Processed handleMessage(unsigned int opc, CANFrame *msg, byte remoteCANID) = 0;
};

}

