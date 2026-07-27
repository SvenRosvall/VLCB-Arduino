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

/// @brief Transport implementation for the MCP2515/25625 CAN controllers
/// 
/// This implementation is available in https://github.com/SvenRosvall/VCAN2515
class CAN2515 : public CanTransport
{
};

}
