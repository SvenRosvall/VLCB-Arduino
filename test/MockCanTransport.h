//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0
//
//

#pragma once

#include <deque>
#include <vector>
#include "Transport.h"
#include "Controller.h"
#include "CanTransport.h"


struct MockCanFrame
{

};

template <>
struct VLCB::CANFrame<MockCanFrame>
{
  uint32_t id;
  bool ext;
  bool rtr;
  uint8_t len;
  uint8_t data[8];
};

// This is to replace the hardware layer. It uses the CanTransport class for CAN processing.
class MockCanTransport : public VLCB::CanTransport<MockCanFrame>
{
public:
  virtual bool available() override;
  virtual VLCB::CANFrame<MockCanFrame> getNextCanFrame() override;
  virtual bool sendCanFrame(VLCB::CANFrame<MockCanFrame> *frame) override;
  
  virtual void reset() override;

  virtual unsigned int receiveCounter() override { return 0; }
  virtual unsigned int transmitCounter() override { return 0; }
  virtual unsigned int receiveErrorCounter() override { return 0; }
  virtual unsigned int transmitErrorCounter() override { return 0; }
  virtual unsigned int errorStatus() override { return 0; }

  void setNextMessage(VLCB::CANFrame<MockCanFrame> frame);
  void clearMessages();

  std::deque<VLCB::CANFrame<MockCanFrame>> incoming_frames;
  std::vector<VLCB::CANFrame<MockCanFrame>> sent_frames;
};
