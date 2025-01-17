// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#pragma once

// header files

#include <Controller.h>
#include <TransportDiagnostics.h>
#include <CAN2515.h>
#include <ACAN2515.h>           // ACAN2515 library

namespace VLCB
{

class CAN2515Diagnostics : public TransportDiagnostics
{
public:
  CAN2515Diagnostics(CAN2515 * can2515)
    : can2515(can2515)
    , canp(can2515->canp)
  {}

  virtual unsigned int receiveCounter() override { return can2515->_numMsgsRcvd; }
  virtual unsigned int transmitCounter() override { return can2515->_numMsgsSent; }
  virtual unsigned int receiveErrorCounter() override { return canp->receiveErrorCounter(); }
  virtual unsigned int transmitErrorCounter() override { return canp->transmitErrorCounter(); }
  virtual unsigned int receiveBufferUsage() override { return canp->receiveBufferCount(); };
  virtual unsigned int transmitBufferUsage() override { return canp->transmitBufferCount(0); };
  virtual unsigned int receiveBufferPeak() override { return canp->receiveBufferPeakCount(); };
  virtual unsigned int transmitBufferPeak() override { return canp->transmitBufferPeakCount(0); };
  virtual unsigned int errorStatus() override { return canp->errorFlagRegister(); }

private:
  CAN2515 *can2515;
  ACAN2515 *canp;   // pointer to CAN object so user code can access its members
};

}