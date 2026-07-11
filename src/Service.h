// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#pragma once

#include <Arduino.h>
#include <vlcbdefs.hpp>

namespace VLCB
{

class Controller;
struct Action;

/// @brief Interface base class for all VLCB services.
/// 
/// Each VLCB module sketch shall be set up with a list of services that 
/// define capabilities of the module. 
/// @cond LIBRARY
/// This class defines the interface that each such service class must implement.
/// @endcond
class Service
{
/// @cond LIBRARY
protected:
  /// Helper function to decide if a given node number is the same as that assigned to this node.
  bool isThisNodeNumber(unsigned int nodeNumber);

  /// Pointer to the Controller object that can be used by implementing classes.
  Controller * controller;

public:
  /// Set a pointer to the controller object for use in implementing class.
  void setController(Controller * ctrl) { this->controller = ctrl; }
  
  /// @brief This optional method is called at the beginning of the Arduino sketch.
  /// Define this method for the service to do any setup required at the beginning. 
  virtual void begin() {}
  
  /// @brief Return a unique ID for this service.
  /// 
  /// This ID is used by configuration utilities to identify the service type.
  /// The service IDs are listed in the VLCB service specifications.
  virtual VlcbServiceTypes getServiceID() const = 0;
  
  /// @brief Return the version of the service specification implemented by this service.
  ///
  /// This version is used by configuration utilities to identify which features are
  /// implemented by this service.
  virtual byte getServiceVersionID() const = 0;

  /// @brief This method that be called regularly from the VLCB core to let the service perform regular tasks.
  /// 
  /// Implementing service classes shall implement this to perform tasks specific to that service
  /// such as polling for changes of input pins.
  /// This method does not need to be implemented if there are no such regular tasks for the service.
  virtual void process() {}

  /// @brief Called when there is an action available.
  /// 
  /// This method does not need to be implemented if the service does react to any actions.
  /// 
  /// @param action The action that the service may have interest in.
  virtual void processAction(const Action & action) {};

  /// @brief Report a given diagnostic value
  /// 
  /// @param serviceIndex index of the service. Not used by the implementation, just passed through to the response message.
  /// @param diagnosticsCode code for the diagnostic to report. This code is specific to the implemented service.
  virtual void reportDiagnostics(byte serviceIndex, byte diagnosticsCode);

  /// @brief report how many diagnostics this service supports.
  virtual int getDiagnosticCount() { return 0; }

  /// Container for service data bytes.
  struct Data { byte data1, data2, data3; };

  /// @brief Return Data bytes specific for the implementing service. 
  /// The meaning of each data byte is defined in the service specification.
  virtual Data getServiceData();
/// @endcond
};

}
