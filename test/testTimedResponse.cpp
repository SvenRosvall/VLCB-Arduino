//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

#include "TestTools.hpp"
#include "ArduinoMock.hpp"

#include "Controller.h"
#include "TimedResponse.h"
#include "VlcbCommon.h"

namespace
{

void testCreateTimedResponse()
{
  test();
  
  VLCB::Controller controller = createController({});
  
  int callCount = 0;
  bool deleted = false;
  class TestResponder : public VLCB::TimedResponse::Task
  {
  public:

    int & callCount;
    bool & deleted;
    TestResponder(int &callCount, bool &deleted)
    : callCount(callCount), deleted(deleted)
    {}

    ~TestResponder()
    {
      deleted = true;
    }

    VLCB::TimedResponse::Result operator()()
    {
      ++callCount;
      return VLCB::TimedResponse::Result::FINISHED;
    }
  };
  
  controller.addTimedResponse(new TestResponder(callCount, deleted));
  
  // Ensure there is a timed response task.
  assertEquals(true, controller.pendingTasks());

  addMillis(5);
  process(controller);
  
  // Ensure the timed response task was executed.
  assertEquals(1, callCount);
  
  // Ensure the timed response task has been removed.
  assertEquals(false, controller.pendingTasks());
  assertEquals(true, deleted);
}

void testTimeResponseCalledAtInterval()
{
  test();
  
  VLCB::Controller controller = createController({});
  
  int callCount = 0;
  class TestResponder : public VLCB::TimedResponse::Task
  {
  public:
    int & callCount;
    TestResponder(int &callCount)
    : callCount(callCount)
    {}

    VLCB::TimedResponse::Result operator()()
    {
      ++callCount;
      if (sequence == 3)
        return VLCB::TimedResponse::Result::FINISHED;
      return VLCB::TimedResponse::Result::PROGRESS;
    }
  };
  
  controller.addTimedResponse(new TestResponder(callCount));
  
  addMillis(5);
  process(controller);
  assertEquals(1, callCount);
  addMillis(5);
  process(controller);
  assertEquals(2, callCount);
  addMillis(5);
  process(controller);
  assertEquals(3, callCount);
  addMillis(5);
  process(controller);
  assertEquals(4, callCount);
  addMillis(5);
  process(controller);
  // Done, not being called again.
  assertEquals(4, callCount);
}

}

void testTimedResponse()
{
  testCreateTimedResponse();
  testTimeResponseCalledAtInterval();
}