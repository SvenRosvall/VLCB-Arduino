//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

#include "Arduino.h"
#include "TimedResponse.h"

namespace VLCB
{

static const int TASK_INTERVAL = 5; // Same interval as VLCBlib_PIC

void TimedResponse::process()
{
  if (lastTaskTime + TASK_INTERVAL > millis())
  {
    // Not time yet for next task step.
    return;
  }

  if (tasks.available())
  {
    Task * task = *tasks.peek();
    Result result = task->runStep();
    lastTaskTime = millis();

    switch (result)
    {
      case PROGRESS:
        ++task->sequence;
        break;

      case RETRY:
        // Keep the task so it can be run again with the same sequence number.
        break;

      case FINISHED:
        delete task;
        tasks.pop();
        break;
    }
  }
}

}