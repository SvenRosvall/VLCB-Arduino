//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0
//
//

#include "MockUserInterface.h"
#include "Controller.h"

void MockUserInterface::process(const VLCB::Command *cmd)
{
  if (cmd != nullptr && cmd->commandType == VLCB::CMD_INDICATE_MODE)
  {
    indicatedMode = cmd->mode;
  }
}

VlcbModeParams MockUserInterface::getIndicatedMode()
{
  return indicatedMode;
}
