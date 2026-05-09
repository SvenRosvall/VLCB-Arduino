// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#pragma once

#include "Service.h"
#include "Configuration.h"
#include "Transport.h"

namespace VLCB
{
/// @brief User interface "service" that communicates via a serial connection
/// 
/// This service writes text to indicate current mode and any activity
/// and allows the user to enter commands to change mode. 
/// See `VLCB Technical Introduction` document for more details.
class SerialUserInterface : public Service
{
public:
  /// @cond LIBRARY
  virtual VlcbServiceTypes getServiceID() const override { return SERVICE_ID_NONE; };
  virtual byte getServiceVersionID() const override { return 1; };

  virtual void process(const Action *action) override;
  /// @endcond

private:
  void handleAction(const Action *action);
  void processSerialInput();
  void indicateMode(VlcbModeParams i);
};

}