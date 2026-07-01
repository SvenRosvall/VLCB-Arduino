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

class ServiceDiagnosticsResponse : public TimedResponse::Task
{
  Service * svc;
  int serviceIndex;
  int diagnosticCount;
public:
  ServiceDiagnosticsResponse(Controller * controller, Service * svc, int serviceIndex, int diagnosticCount)
    : Task(controller), svc(svc), serviceIndex(serviceIndex), diagnosticCount(diagnosticCount) {}

  TimedResponse::Result runStep() override
  {
    if (this->sequence == diagnosticCount)
    {
      return TimedResponse::FINISHED;
    }
    svc->reportDiagnostics(serviceIndex, sequence + 1);
    return TimedResponse::PROGRESS;
  }
};

class AllServiceDiagnosticsResponse : public TimedResponse::Task
{
  int serviceIndex;
  ServiceDiagnosticsResponse * svcResponder = nullptr;
public:
  AllServiceDiagnosticsResponse(Controller * controller) 
    : Task(controller), serviceIndex(0)
  { }

  TimedResponse::Result nextService()
  {
    if (++serviceIndex == controller->getServices().size())
    {
      return TimedResponse::FINISHED;
    }
    return TimedResponse::PROGRESS;
  }

  TimedResponse::Result runStep() override
  {
    if (svcResponder == nullptr)
    {
      Service * svc = controller->getServices()[serviceIndex];
      if (svc->getServiceID() == 0)
      {
        // Not a real service, skip it.
        return nextService();
      }
      int diagnosticCount = svc->getDiagnosticCount();
      controller->sendDGN(serviceIndex + 1, 0, diagnosticCount);
      if (diagnosticCount > 0)
      {
        // Create a responder for this service.
        svcResponder = new ServiceDiagnosticsResponse(controller, svc, serviceIndex + 1, diagnosticCount);
        return TimedResponse::PROGRESS;
      }
      else
      {
        // No diagnostics, move to next service
        return nextService();
      }
    }
    TimedResponse::Result result = svcResponder->runStep();
    switch (result)
    {
      case TimedResponse::RETRY:
        return TimedResponse::RETRY;
        
      case TimedResponse::PROGRESS:
        // Move on to next diagnostic.
        svcResponder->sequence++;
        return TimedResponse::PROGRESS;

      case TimedResponse::FINISHED:
        // This service is complete. Go to next service.
        delete svcResponder;
        svcResponder = nullptr;
        return nextService();
    }
    // Shouldn't happen. Keep the warnings quiet.
    return result;
  }
};

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
    controller->addTimedResponseTask(new AllServiceDiagnosticsResponse(controller));
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
      int diagnosticCount = svc->getDiagnosticCount();
      controller->sendDGN(serviceIndex, 0, diagnosticCount);
      if (diagnosticCount > 0)
      {
        controller->addTimedResponseTask(new ServiceDiagnosticsResponse(controller, svc, serviceIndex, diagnosticCount));
      }
    }
    else
    {
      svc->reportDiagnostics(serviceIndex, diagnosticCode);
    }
  }
}

void MinimumNodeServiceWithDiagnostics::reportDiagnostics(byte serviceIndex, byte diagnosticsCode)
{
  unsigned int diagnosticsValue;
  switch (diagnosticsCode)
  {
    case 0x02: // Uptime upper word
      diagnosticsValue = ((millis() / 1000) >> 16) & 0xFFFF;
      break;
    case 0x03: // Uptime lower word
      diagnosticsValue = (millis() / 1000) & 0xFFFF;
      break;
    case 0x05: // Node Number changes
      diagnosticsValue = diagNodeNumberChanges;
      break;
    case 0x06: // Received messages acted on 
      diagnosticsValue = controller->getMessagesActedOn();
      break;

    // Diagnostics codes not yet implemented
    case 0x01: // Status code -- TODO: not implemented, always good
    case 0x04: // Memory error count -- TODO: not implemented
      diagnosticsValue = 0;
      break;

    default:
      controller->sendGRSP(OPC_RDGN, serviceIndex, GRSP_INVALID_DIAGNOSTIC);
      return;
  }
  
  controller->sendDGN(serviceIndex, diagnosticsCode, diagnosticsValue);
}

void MinimumNodeServiceWithDiagnostics::diagNodeNumberChanged()
{
  ++diagNodeNumberChanges;
}

int MinimumNodeServiceWithDiagnostics::getDiagnosticCount()
{
  return 6;
}

}
