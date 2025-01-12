// Copyright (C) Martin Da Costa 2024 (martindc.merg@gmail.com)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#include "EventConsumerServiceWithDiagnostics.h"
#include <Controller.h>

namespace VLCB {

void EventConsumerServiceWithDiagnostics::reportDiagnostics(byte serviceIndex, byte diagnosticsCode)
{
  switch (diagnosticsCode)
  {
    case 0x00:
      reportAllDiagnostics(serviceIndex);
      break;
    case 0x01: 
      controller->sendDGN(serviceIndex, diagnosticsCode, diagEventsConsumed);
      break;
    default:
      controller->sendGRSP(OPC_RDGN, serviceIndex, GRSP_INVALID_DIAGNOSTIC);
      return;
  }
}

void EventConsumerServiceWithDiagnostics::reportAllDiagnostics(byte serviceIndex)
{
  byte diagCount = 1;
  controller->sendDGN(serviceIndex, 0, diagCount);
  for (byte i = 1; i <= diagCount ; ++i)
  {
    reportDiagnostics(serviceIndex, i);
  }
}      
}