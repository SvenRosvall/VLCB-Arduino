
#pragma once

// header files
#include <Arduino.h>
#include <Controller.h>
#include <CanTransport.h>

namespace VLCB
{
  struct GCFrame
  {
    uint32_t id;
    bool ext;
    bool rtr;
    uint8_t len;
    uint8_t data[8];
  };
  
  template <>
  struct CANFrame<GCFrame>
  {
    GCFrame &getMessage() { return frame; }

    uint32_t id() const { return frame.id; }
    bool ext() const { return frame.ext; }
    bool rtr() const { return frame.rtr; }
    uint8_t len() const { return frame.len; }
    const void *data() const { return frame.data; }

    void id(uint32_t id) { frame.id = id; }
    void ext(bool ext) { frame.ext = ext; }
    void rtr(bool rtr) { frame.rtr = rtr; }
    void len(byte len) { frame.len = len; }
    byte *data() { return frame.data; }

  private:
    GCFrame frame;
  };

  bool decodeGridConnect(const char * gcBuffer, CANFrame<GCFrame> *frame);
  bool encodeGridConnect(char * txBuffer, CANFrame<GCFrame> *frame);
}
