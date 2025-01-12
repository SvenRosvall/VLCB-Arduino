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
      controller->sendDGN(serviceIndex, diagnosticsCode, 0);
      break;
    case 0x02: // Uptime upper word
      controller->sendDGN(serviceIndex, diagnosticsCode, ((millis() / 1000) >> 16) & 0xFFFF);
      break;
    case 0x03: // Uptime lower word
      controller->sendDGN(serviceIndex, diagnosticsCode, (millis() / 1000) & 0xFFFF);
      break;
    case 0x04: // Memory error count -- TODO: not implemented
      controller->sendDGN(serviceIndex, diagnosticsCode, 0);
      break;
    case 0x05: // Node Number changes
      controller->sendDGN(serviceIndex, diagnosticsCode, diagNodeNumberChanges);
      break;
    case 0x06: // Received messages acted on 
      controller->sendDGN(serviceIndex, diagnosticsCode, controller->getMessagesActedOn());
      break;
    default:
      controller->sendGRSP(OPC_RDGN, serviceIndex, GRSP_INVALID_DIAGNOSTIC);
      return;
  }
}

void MinimumNodeServiceWithDiagnostics::diagNodeNumberChanged()
{
  ++diagNodeNumberChanges;
}

void MinimumNodeServiceWithDiagnostics::reportAllDiagnostics(byte serviceIndex)
{
  controller->sendDGN(serviceIndex, 0, 6);
  for (byte i = 1; i <= 0x06 ; ++i)
  {
    reportDiagnostics(serviceIndex, i);
  }
}

}
