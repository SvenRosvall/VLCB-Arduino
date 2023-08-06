//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

#pragma once

namespace VLCB
{

// Op-codes that are not available in cbusdefs.h
enum OpCodes
{
  OPC_GRSP = 0xAF,
};

enum GrspCodes
{
  // These first set of codes are common codes with CMDERR
  GRSP_OK = 0,
  GRSP_INVALID_PARAMETER = 9,

  // The codes below are new for VLCB
  GRSP_INVALID_SERVICE = 252
};

}