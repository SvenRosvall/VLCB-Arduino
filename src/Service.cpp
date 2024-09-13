//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

#include "Service.h"
#include "Controller.h"

namespace VLCB
{

bool Service::isThisNodeNumber(unsigned int nn)
{
  return nn == controller->getModuleConfig()->nodeNum;
}

}
