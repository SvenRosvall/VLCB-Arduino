//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

#pragma once

#include "AbstractEventTeachingService.h"

namespace VLCB
{
/// # Event Teaching Service API
///
/// This service allows an Events Table to be created and managed using
/// the MMC or similar. A full description of events can be found in
/// the document `VLCB Event Model`.
/// The Events Table is used by the Producer and Consumer Services.
/// This Event Teaching Service is only rewired if one or both of these
/// other services are present.
///
/// There are no user functions in this service.
///
class EventTeachingService : public AbstractEventTeachingService
{
/// \cond LIBRARY
public:
  virtual void process(const Action * action) override;
  virtual VlcbServiceTypes getServiceID() const override { return SERVICE_ID_OLD_TEACH; }
  virtual byte getServiceVersionID() const override { return 3; }

private:
  void handleMessage(const VlcbMessage *msg);
  void handleRequestEventVariable(const VlcbMessage *msg, unsigned int nn, unsigned int en);
  void handleLearnEvent(const VlcbMessage *msg, unsigned int nn, unsigned int en);
/// \endcond
};

}  // VLCB
