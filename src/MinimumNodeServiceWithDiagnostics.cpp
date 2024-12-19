//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

#include "MinimumNodeServiceWithDiagnostics.h"
#include "Controller.h"

namespace VLCB
{

void MinimumNodeServiceWithDiagnostics::handleMessage(const VlcbMessage *msg)
{
  unsigned int opc = msg->data[0];
  unsigned int nn = Configuration::getTwoBytes(&msg->data[1]);

  switch (opc)
  {
    case OPC_RDGN:
      // 87 - Request Diagnostic Data
      handleRequestDiagnostics(msg, nn);
      break;

    default:
      // Revert back to the main class for the remaining op-codes.
      MinimumNodeService::handleMessage(msg);
      break;
  }
}

void MinimumNodeServiceWithDiagnostics::handleRequestDiagnostics(const VlcbMessage *msg, unsigned int nn)
{
  if (!isThisNodeNumber(nn))
  {
    // Not for this module.
    return;
  }

  if (msg->len < 5)
  {
    controller->sendGRSP(OPC_RDGN, getServiceID(), CMDERR_INV_CMD);
    return;
  }

  byte serviceIndex = msg->data[3];
  if (serviceIndex > controller->getServices().size())
  {
    controller->sendGRSP(OPC_RDGN, getServiceID(), GRSP_INVALID_SERVICE);
    return;
  }

  if (serviceIndex == 0)
  {
    // Request for diagnostics for all services.
    for (serviceIndex = 1; serviceIndex <= controller->getServices().size(); serviceIndex++)
    {
      Service * svc = controller->getServices()[serviceIndex - 1];
      if (svc->getServiceID() == 0)
      {
        // Not a real service, skip it.
        continue;
      }

      svc->reportAllDiagnostics(serviceIndex);
    }
  }
  else
  {
    // Request for diagnostics for a specific service.
    Service *svc = controller->getServices()[serviceIndex - 1];
    if (svc->getServiceID() == 0)
    {
      // Not a real service, send error response.
      controller->sendGRSP(OPC_RDGN, getServiceID(), GRSP_INVALID_SERVICE);
      return;
    }
    byte diagnosticCode = msg->data[4];
    if (diagnosticCode == 0)
    {
      svc->reportAllDiagnostics(serviceIndex);
    }
    else
    {
      svc->reportDiagnostics(serviceIndex, diagnosticCode);
    }
  }
}

void MinimumNodeServiceWithDiagnostics::reportDiagnostics(byte serviceIndex, byte diagnosticsCode)
{
  switch (diagnosticsCode)
  {
    case 0x00:
      reportAllDiagnostics(serviceIndex);
      break;
    case 0x01: // Status code -- TODO: not implemented, always good
      controller->sendMessageWithNN(OPC_DGN, serviceIndex, diagnosticsCode, 0, 0);
      break;
    case 0x02: // Uptime upper word
    {
      unsigned long now = millis() / 1000;
      controller->sendMessageWithNN(OPC_DGN, serviceIndex, diagnosticsCode, (now >> 24) & 0xFF , (now >> 16) & 0xFF);
      break;
    }
    case 0x03: // Uptime lower word
    {
      unsigned long now = millis() / 1000;
      controller->sendMessageWithNN(OPC_DGN, serviceIndex, diagnosticsCode, (now >> 8) & 0xFF , now & 0xFF);
      break;
    }
    case 0x04: // Memory error count -- TODO: not implemented
      controller->sendMessageWithNN(OPC_DGN, serviceIndex, diagnosticsCode, 0, 0);
      break;
    case 0x05: // Node Number changes
      controller->sendMessageWithNN(OPC_DGN, serviceIndex, diagnosticsCode, highByte(diagNodeNumberChanges), lowByte(diagNodeNumberChanges));
      break;
    case 0x06: // Received messages acted on 
    {
      unsigned int diagMsgsActed = controller->getMessagesActedOn();
      controller->sendMessageWithNN(OPC_DGN, serviceIndex, diagnosticsCode, highByte(diagMsgsActed), lowByte(diagMsgsActed));
      break;
    }
    default:
      controller->sendGRSP(OPC_RDGN, serviceIndex, GRSP_INVALID_DIAGNOSTIC);
      return;
  }
}

void MinimumNodeServiceWithDiagnostics::reportAllDiagnostics(byte serviceIndex)
{
  controller->sendMessageWithNN(OPC_DGN, serviceIndex, 0, 0, 6);
  for (byte i = 1; i <= 0x06 ; ++i)
  {
    reportDiagnostics(serviceIndex, i);
  }
}

}
