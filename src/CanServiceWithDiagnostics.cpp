//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

#include "CanServiceWithDiagnostics.h"
#include "Controller.h"

namespace VLCB
{


void CanServiceWithDiagnostics::reportDiagnostics(byte serviceIndex, byte diagnosticsCode)
{
  unsigned int diagnosticsValue;
  switch (diagnosticsCode)
  {
    case 0x00:
      reportAllDiagnostics(serviceIndex);
      return;
    case 0x01: // CAN RX error counter
      diagnosticsValue = tptDiag->receiveErrorCounter();
      break;
    case 0x02: // CAN TX error counter
      diagnosticsValue = tptDiag->transmitErrorCounter();
      break;
    case 0x03: // CAN status byte, this is hardware dependent. The meaning of each bitfield must be documented.
      diagnosticsValue = tptDiag->errorStatus();
      break;
    case 0x04: // Tx buffer current usage count
      diagnosticsValue = tptDiag->transmitBufferUsage();
      break;
    case 0x06: // TX message count
      diagnosticsValue = tptDiag->transmitCounter();
      break;
    case 0x07: // RX buffer current usage count
      diagnosticsValue = tptDiag->receiveBufferUsage();
      break;
    case 0x09: // RX message counter
      diagnosticsValue = tptDiag->receiveCounter();
      break;
    case 0x11: // Transmit buffers used high watermark - Added in service version 2
      diagnosticsValue = tptDiag->transmitBufferPeak();
      break;
    case 0x12: // Receive buffers used high watermark - Added in service version 2
      diagnosticsValue = tptDiag->receiveBufferPeak();
      break;

    // Diagnostics codes not yet implemented
    case 0x05: // Tx buffer overrun count
    case 0x08: // RX buffer overrun count
    case 0x0A: // CAN error frames detected
    case 0x0B: // CAN error frames generated (both active and passive ?)
    case 0x0C: // number of times CAN arbitration was lost
    case 0x0D: // number of CANID enumerations
    case 0x0E: // number of CANID conflicts detected
    case 0x0F: // the number of CANID changes
    case 0x10: // the number of CANID enumeration failures
      diagnosticsValue = 0;
      break;

    default:
      controller->sendGRSP(OPC_RDGN, serviceIndex, GRSP_INVALID_DIAGNOSTIC);
      return;
  }

  controller->sendDGN(serviceIndex, diagnosticsCode, diagnosticsValue);
}

void CanServiceWithDiagnostics::reportAllDiagnostics(byte serviceIndex)
{
  byte diagCount = 18;
  controller->sendDGN(serviceIndex, 0, diagCount);
  for (byte i = 1; i <= diagCount ; ++i)
  {
    reportDiagnostics(serviceIndex, i);
  }
}

} // VLCB