//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

#include "CombinedUserInterface.h"
#include "Controller.h"

namespace VLCB
{

CombinedUserInterface::CombinedUserInterface(UserInterface *ui1, UserInterface *ui2)
  : ui1(ui1), ui2(ui2)
{}

void CombinedUserInterface::setController(Controller *ctrl)
{
  ui1->setController(ctrl);
  ui2->setController(ctrl);
}

void CombinedUserInterface::process(const Command *cmd)
{
  ui1->process(cmd);
  ui2->process(cmd);
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

}