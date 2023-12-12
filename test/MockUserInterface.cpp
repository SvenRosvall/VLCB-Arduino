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

void MockUserInterface::indicateResetting()
{

}

void MockUserInterface::indicateResetDone()
{

}

void MockUserInterface::indicateActivity()
{

}

void MockUserInterface::indicateMode(byte mode)
{
  indicatedMode = mode;
}

bool MockUserInterface::resetRequested()
{
  return false;
}

VLCB::UserInterface::RequestedAction MockUserInterface::checkRequestedAction()
{
  RequestedAction ret = requestedAction;
  requestedAction = RequestedAction::NONE;
  return ret;
}

void MockUserInterface::setRequestedAction(VLCB::UserInterface::RequestedAction action)
{
  requestedAction = action;
}

byte MockUserInterface::getIndicatedMode()
{
  return indicatedMode;
}
