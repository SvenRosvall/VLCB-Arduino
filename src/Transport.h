// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#pragma once

namespace VLCB
{

// Interface for statistics on the transport layer.
class Transport
{
public:
  virtual void reset() = 0;

  virtual unsigned int receiveCounter() = 0;
  virtual unsigned int transmitCounter() = 0;
  virtual unsigned int receiveErrorCounter() = 0;
  virtual unsigned int transmitErrorCounter() = 0;
  virtual unsigned int receiveBufferUsage() = 0;
  virtual unsigned int transmitBufferUsage() = 0;
  virtual unsigned int receiveBufferPeak() = 0;
  virtual unsigned int transmitBufferPeak() = 0;
  virtual unsigned int errorStatus() = 0;
};

}
