// Copyright (C) Martin Da Costa 2023 (martindc.merg@gmail.com)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#include <Streaming.h>

#include "OpCodeConsumerService.h"
#include <Controller.h>
#include <vlcbdefs.hpp>

namespace VLCB {

void OpCodeConsumerService::setController(Controller *cntrl) 
{
  this->controller = cntrl;
  this->module_config = cntrl->getModuleConfig();
}

//
/// register the user handler for Op-Codes
//
void OpCodeConsumerService::setOpCodeHandler(void (*fptr)(const VlcbMessage *msg), byte opcodes[], byte num_opcodes)
{
  opcodehandler = fptr;
  _opcodes = opcodes;
  _num_opcodes = num_opcodes;
}

void OpCodeConsumerService::process(const Action *action)
{
  if (action != nullptr && action->actionType == ACT_MESSAGE_IN)
  {
    handleOpCodeMessage(&action->vlcbMessage);
  }
}
  
void OpCodeConsumerService::handleOpCodeMessage(const VlcbMessage *msg)
{
  // check if incoming opcode is in the user list, if list length > 0
  byte opc = msg->data[0];
  
  if (_num_opcodes > 0)
  {
    for (byte i = 0; i < _num_opcodes; i++)
    {
      if (opc == _opcodes[i])
      {
        (void)(*opcodehandler)(msg);
        break;
      }
    }
  }
}
}
  