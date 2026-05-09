//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

#pragma once

#include <Arduino.h>
#include "Transport.h"

namespace VLCB
{

/// @cond LIBRARY
/// @brief Represents a CAN frame sent on the CAN bus.
/// 
/// Contains all the information used on the actual CAN bus.
/// Different implementations use their own representations of this data
/// which must be converted to/from CANFrame
struct CANFrame
{
  uint32_t id;
  bool ext;
  bool rtr;
  uint8_t len;
  uint8_t data[8];
};
/// @endcond 

/// @brief Interface base class for CAN transport implementations.
/// 
/// A concrete class derived from CanTransport must be used with CanService.
class CanTransport : public Transport
{
public:
  /// @cond LIBRARY
  virtual bool available() = 0; ///< Check if an incoming CAN frame is available on the CAN bus.
  virtual CANFrame getNextCanFrame() = 0; ///< Get the next CAN frame from the CAN bus.
  virtual bool sendCanFrame(CANFrame *msg) = 0; ///< Send a CAN frame to the CAN bus. 

  inline virtual byte getHardwareType() { return 0; } ///< Get the hardware type of the concrete transport class.
  /// @endcond 
};

}