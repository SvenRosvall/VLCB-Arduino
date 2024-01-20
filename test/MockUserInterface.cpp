//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0
//
//

#include "MockUserInterface.h"

void MockUserInterface::run()
{

}

void MockUserInterface::indicateActivity()
{

}

void MockUserInterface::indicateMode(VlcbModeParams mode)
{
  indicatedMode = mode;
}

VLCB::UserInterface::RequestedAction MockUserInterface::checkRequestedAction()
{
  return requestedAction;
}

void MockUserInterface::setRequestedAction(VLCB::UserInterface::RequestedAction action)
{
  requestedAction = action;
}

VlcbModeParams MockUserInterface::getIndicatedMode()
{
  return indicatedMode;
}
