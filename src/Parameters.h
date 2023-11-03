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
    params[PAR_NUM] = 20;                     //  0 num params = 20
    params[PAR_MANU] = MANU_MERG;              //  1 manf = MERG, 165
    params[PAR_EVTNUM] = config.EE_MAX_EVENTS;   //  4 num events
    params[PAR_EVNUM] = config.EE_NUM_EVS;      //  5 num evs per event
    params[PAR_NVNUM] = config.EE_NUM_NVS;      //  6 num NVs
    params[PAR_BUSTYPE] = PB_CAN;                // CAN implementation of Controller
    params[PAR_LOAD] = 0x00;
    params[PAR_LOAD+1] = 0x00;
    params[PAR_LOAD+2] = 0x00;
    params[PAR_LOAD+3] = 0x00;
    initProcessorParams();
  }

  void setVersion(char major, char minor, char beta)
  {
    params[PAR_MAJVER] = major;                //  7 code major version
    params[PAR_MINVER] = minor;                //  2 code minor version
    params[PAR_BETA] = beta;                // 20 code beta version
  }

  void setModuleId(byte id)
  {
    params[PAR_MTYP] = id;                   //  3 module id
  }

  void setFlags(byte flags)
  {
    params[PAR_FLAGS] = flags;                //  8 flags - FLiM, consumer/producer
  }

  // Optional: use this to override processor info that is set by default.
  void setProcessor(byte manufacturer, byte id, char const * name)
  {
    params[PAR_CPUID] = id;                  //  9 processor id
    params[PAR_CPUMAN] = manufacturer;       // 19 processor manufacturer
    memcpy(params + PAR_CPUMID, name, 4);   // 15-18 processor version
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