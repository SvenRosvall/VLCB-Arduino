#pragma once

//#include "CBUS.h"

namespace VLCB
{

const int DEFAULT_PRIORITY = 0xB;     // default CBUS messages priority. 1011 = 2|3 = normal/low

class CANFrame;

// Interface for sending and receiving CBUS messages.
class Transport
{
public:
#ifdef ARDUINO_ARCH_RP2040
  virtual bool begin(bool poll = false, SPIClassRP2040 spi = SPI) = 0;
#else
  virtual bool begin(bool poll = false, SPIClass spi = SPI) = 0;
#endif
  virtual bool available(void) = 0;
  virtual CANFrame getNextMessage(void) = 0;
  virtual bool sendMessage(CANFrame *msg, bool rtr = false, bool ext = false, byte priority = DEFAULT_PRIORITY) = 0;
  virtual void reset(void) = 0;
};

}
