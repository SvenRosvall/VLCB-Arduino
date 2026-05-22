//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0
//
//

#include "InternalDiagnosticsService.h"
#include "Controller.h"

namespace VLCB
{

void InternalDiagnosticsService::reportDiagnostics(byte serviceIndex, byte diagnosticsCode)
{
  unsigned int diagnosticsValue;
  switch (diagnosticsCode)
  {
    case 0x00:
      reportAllDiagnostics(serviceIndex);
      return;
    case 0x01: // Free memory.
      diagnosticsValue = controller->getModuleConfig()->freeSRAM();
      break;
    case 0x02: // Action queue: current size
      diagnosticsValue = controller->getActionQueue().bufUse();
      break;
    case 0x03: // Action queue: high water mark
      diagnosticsValue = controller->getActionQueue().getHighWaterMark();
      break;
    case 0x04: // Action queue: number of overflows
      diagnosticsValue = controller->getActionQueue().getOverflows();
      break;

    default:
      controller->sendGRSP(OPC_RDGN, serviceIndex, GRSP_INVALID_DIAGNOSTIC);
      return;
  }
  
  controller->sendDGN(serviceIndex, diagnosticsCode, diagnosticsValue);
}

void InternalDiagnosticsService::reportAllDiagnostics(byte serviceIndex)
{
  byte diagCount = 4;
  controller->sendDGN(serviceIndex, 0, diagCount);
  for (byte i = 1; i <= diagCount ; ++i)
  {
    reportDiagnostics(serviceIndex, i);
  }
}

}
