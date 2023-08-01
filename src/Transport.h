#pragma once

#include <SPI.h>

namespace VLCB
{

const int DEFAULT_PRIORITY = 0xB;     // default Controller messages priority. 1011 = 2|3 = normal/low

class Controller;
class CANFrame;

// Interface for sending and receiving Controller messages.
class Transport
{
public:
  virtual void setController(Controller * ctrl) { }
#ifdef ARDUINO_ARCH_RP2040
  virtual bool begin(bool poll = false, SPIClassRP2040 spi = SPI) = 0;
#else
  virtual bool begin(bool poll = false, SPIClass spi = SPI) = 0;
#endif
  virtual bool available() = 0;
  virtual CANFrame getNextMessage() = 0;
  virtual bool sendMessage(CANFrame *msg, bool rtr = false, bool ext = false, byte priority = DEFAULT_PRIORITY) = 0;
  virtual void reset() = 0;
};

}
