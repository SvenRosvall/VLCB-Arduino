//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0
//
//

#include "MockUserInterface.h"

void MockUserInterface::process(const VLCB::Command *cmd)
{

}

void MockUserInterface::indicateResetting()
{

}

void MockUserInterface::indicateResetDone()
{

}

void MockUserInterface::indicateMode(VlcbModeParams mode)
{
  indicatedMode = mode;
}

bool MockUserInterface::resetRequested()
{
  return false;
}

VlcbModeParams MockUserInterface::getIndicatedMode()
{
  return indicatedMode;
}
