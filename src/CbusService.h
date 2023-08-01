#pragma once

#include "Service.h"

namespace VLCB {

// Quick and dirty service to migrate from the CBUS library to using a set of VLCB services.
// This is a service that implements all the CBUS op-codes.
// When creating new services, pick code from here.
// In the end there should not be any code left in this service.
class CbusService : public Service
{
public:
  virtual void setController(Controller *controller) override { this->controller = controller; }
  void setEventHandler(void (*fptr)(byte index, CANFrame *msg));
  void setEventHandler(void (*fptr)(byte index, CANFrame *msg, bool ison, byte evval));
  virtual void handleMessage(unsigned int opc, CANFrame *msg, byte remoteCANID) override;

private:
  Controller * controller;
  void (*eventhandler)(byte index, CANFrame *msg);
  void (*eventhandlerex)(byte index, CANFrame *msg, bool evOn, byte evVal);

  void processAccessoryEvent(CANFrame *msg, unsigned int nn, unsigned int en, bool is_on_event);
};

} // VLCB
