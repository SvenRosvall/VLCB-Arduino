//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

#pragma once

namespace VLCB
{

class TimedResponse
{
public:
  enum Result { PROGRESS, RETRY, FINISHED };
  
  class Task
  {
  public:
    virtual Result operator()() = 0;
    int sequence = 0;
  };
  
};

} // VLCB
