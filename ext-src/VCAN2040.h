//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0
//
//

#pragma once

#include <CanTransport.h>

namespace VLCB
{

/// @brief Transport implementation for the software CAN controller on a RP2040 / RP2350
/// 
/// This implementation is available in https://github.com/MartinDaCosta53/VCAN2040
class VCAN2040 : public CanTransport
{
};

}
