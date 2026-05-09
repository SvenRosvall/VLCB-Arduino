//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

#pragma once

#include "AbstractEventTeachingService.h"

namespace VLCB
{
/// @brief Slot Event Teaching Service
///
/// This service allows an Events Table to be created and managed using
/// the MMC or similar. A full description of events can be found in
/// the document `VLCB Event Model`.
/// The Events Table is used by the Producer and Consumer Services.
/// This Event Teaching Service is only required if one or both of these
/// other services are present.
/// With this service events are organised in the events table so that
/// each slot (table entry) has a meaning defined by the user sketch.
///
/// There are no user functions in this service.
///
class EventSlotTeachingService : public AbstractEventTeachingService
{
public:
  /// @cond LIBRARY
  virtual void process(const Action * action) override;
  virtual VlcbServiceTypes getServiceID() const override { return SERVICE_ID_TEACH; }
  virtual byte getServiceVersionID() const override { return 1; }
  /// @endcond

private:
  void handleMessage(const VlcbMessage *msg);
  void handleLearnEventIndex(const VlcbMessage *msg);
  void handleReadEventIndex(unsigned int nn, byte eventIndex);
};

}  // VLCB
