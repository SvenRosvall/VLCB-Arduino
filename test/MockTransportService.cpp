//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

#include "MockTransportService.h"
#include "Controller.h"

void MockTransportService::setController(VLCB::Controller *cntrl)
{
  canTransport->setController(cntrl);
}

void MockTransportService::process(const VLCB::Command *cmd)
{
  canTransport->process();

  if (cmd == nullptr)
  {
    return;
  }

  switch (cmd->commandType)
  {
    case VLCB::CMD_MESSAGE_OUT:
      canTransport->sendMessage(&cmd->vlcbMessage);
      break;
  }
}
