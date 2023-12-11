// Copyright (C) Martin Da Costa 2023 (martindc.merg@gmail.com)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#pragma once

#include "Service.h"
#include "CircularBuffer.h"
#include <Controller.h>
#include <vlcbdefs.hpp>

namespace VLCB {

class Configuration;

class ConsumeOwnEventsService : public Service
{
public:
  ConsumeOwnEventsService(byte bufferCapacity = 4)
    : buffer(bufferCapacity) {}

  virtual Processed handleMessage(unsigned int opc, VlcbMessage *msg) override {return NOT_PROCESSED;}

  virtual byte getServiceID() override
  {
    return SERVICE_ID_CONSUME_OWN_EVENTS;
  }
  virtual byte getServiceVersionID() override
  {
    return 1;
  }

  bool available() { return buffer.available(); }
  VlcbMessage *peek() { return buffer.peek(); }
  VlcbMessage *get() { return buffer.get(); }
  void put(const VlcbMessage *msg) { buffer.put(msg); }
  void clear() { buffer.clear(); }

  // Diagnostic metrics access
  unsigned int getNumberOfPuts() { return buffer.getNumberOfPuts(); }
  unsigned int getNumberofGets() { return buffer.getNumberOfGets(); }
  unsigned int getOverflows() { return buffer.getOverflows(); }
  unsigned int getHighWaterMark() { return buffer.getHighWaterMark(); }

private:

  CircularBuffer<VlcbMessage> buffer;
};

}  // VLCB
