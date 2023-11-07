//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

#include "CombinedUserInterface.h"

namespace VLCB
{

CombinedUserInterface::CombinedUserInterface(UserInterface *ui1, UserInterface *ui2)
  : ui1(ui1), ui2(ui2)
{}

void CombinedUserInterface::run()
{
  ui1->run();
  ui2->run();
}

void CombinedUserInterface::indicateResetting()
{
  ui1->indicateResetting();
  ui2->indicateResetting();
}

void CombinedUserInterface::indicateResetDone()
{
  ui1->indicateResetDone();
  ui2->indicateResetDone();
}

bool CombinedUserInterface::resetRequested()
{
  return ui1->resetRequested() || ui2->resetRequested();
}

void CombinedUserInterface::indicateActivity()
{
  ui1->indicateActivity();
  ui2->indicateActivity();
}

void CombinedUserInterface::indicateMode(byte mode)
{
  ui1->indicateMode(mode);
  ui2->indicateMode(mode);
}

UserInterface::RequestedAction CombinedUserInterface::checkRequestedAction()
{
  auto ret = ui1->checkRequestedAction();
  if (ret == UserInterface::NONE)
  {
    ret = ui2->checkRequestedAction();
  }
  return ret;
}

}