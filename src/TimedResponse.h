//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

#pragma once

#include "CircularBuffer.h"

namespace VLCB
{

class Controller;

/// @brief Manage tasks that respond with messages at timed intervals
/// 
/// Users of this class add a task that sends a response message each time
/// it is called.
/// This avoids sending messages faster than the transport object can send them.
class TimedResponse
{
public:
  enum Result { PROGRESS, RETRY, FINISHED };
  
  class Task
  {
  public:
    Task(Controller * controller) : controller(controller) {}
    Task() : controller(nullptr) {}
    virtual ~Task() = default;
    virtual Result operator()() = 0;
    int sequence = 0;
  protected:
    Controller *controller;
  };
  
  void add(Task * task)
  {
    tasks.put(task);
  }
  
  void process();
  
  bool pendingTasks() const
  {
    return tasks.available();
  }

private:
  CircularBuffer<Task *> tasks;
  unsigned long lastTaskTime = 0;
};

} // VLCB
