// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#include <Configuration.h>
#include <vlcbdefs.hpp>               // MERG Controller constants

namespace VLCB
{

class Parameters
{
public:
  explicit Parameters(Configuration const & config)
  {
    params[0] = 20;                     //  0 num params = 20
    params[1] = MANU_MERG;              //  1 manf = MERG, 165
    params[4] = config.EE_MAX_EVENTS;   //  4 num events
    params[5] = config.EE_NUM_EVS;      //  5 num evs per event
    params[6] = config.EE_NUM_NVS;      //  6 num NVs
    params[10] = PB_CAN;                // CAN implementation of Controller
    params[11] = 0x00;
    params[12] = 0x00;
    params[13] = 0x00;
    params[14] = 0x00;
    initProcessorParams();
  }

  void setVersion(char major, char minor, char beta)
  {
    params[7] = major;                //  7 code major version
    params[2] = minor;                //  2 code minor version
    params[20] = beta;                // 20 code beta version
  }

  void setModuleId(byte id)
  {
    params[3] = id;                   //  3 module id
  }

  void setFlags(byte flags)
  {
    params[8] = flags;                //  8 flags - FLiM, consumer/producer
  }

  // Optional: use this to override processor info that is set by default.
  void setProcessor(byte manufacturer, byte id, char const * name)
  {
    params[9] = id;                  //  9 processor id
    params[19] = manufacturer;       // 19 processor manufacturer
    memcpy(params + 15, name, 4);   // 15-18 processor version
  }

  unsigned char * getParams()
  {
    return params;
  }

private:
  // Initializes processor specific parameters based on pre-defined macros in Arduino IDE.
  static void initProcessorParams();

  // Memory for the params is allocated on global memory and handed over to Controller.setParams().
  static unsigned char params[21];
};

}