/*

  Copyright (C) Sven Rosvall 2021

  This work is licensed under the:
      Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
   To view a copy of this license, visit:
      http://creativecommons.org/licenses/by-nc-sa/4.0/
   or send a letter to Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.

   License summary:
    You are free to:
      Share, copy and redistribute the material in any medium or format
      Adapt, remix, transform, and build upon the material

    The licensor cannot revoke these freedoms as long as you follow the license terms.

    Attribution : You must give appropriate credit, provide a link to the license,
                  and indicate if changes were made. You may do so in any reasonable manner,
                  but not in any way that suggests the licensor endorses you or your use.

    NonCommercial : You may not use the material for commercial purposes. **(see note below)

    ShareAlike : If you remix, transform, or build upon the material, you must distribute
                 your contributions under the same license as the original.

    No additional restrictions : You may not apply legal terms or technological measures that
                                 legally restrict others from doing anything the license permits.

   ** For commercial use, please contact the original copyright holder(s) to agree licensing terms

    This software is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE

*/

#include <CBUSConfig.h>

class CBUSParams
{
public:
  CBUSParams(CBUSConfig const & config)
    : params(new unsigned char[21])
  {
    params[0] = 20;
    params[1] = MANU_MERG;
    params[4] = config.EE_MAX_EVENTS;   //  4 num events
    params[5] = config.EE_NUM_EVS;      //  5 num evs per event
    params[6] = config.EE_NUM_NVS;      //  6 num NVs
    params[9] = 0x32;
    params[10] = PB_CAN;
    params[11] = 0x00;
    params[12] = 0x00;
    params[13] = 0x00;
    params[14] = 0x00;
    params[15] = '3';
    params[16] = '2';
    params[17] = '8';
    params[18] = 'P';
    params[19] = CPUM_ATMEL;
  }

  void setVersion(char major, char minor, char beta)
  {
    params[7] = major;
    params[2] = minor;
    params[20] = beta;
  }

  void setModuleId(byte id)
  {
    params[3] = id;
  }

  void setFlags(byte flags)
  {
    params[8] = flags;
  }

  void setProcessor(byte manufacturer, byte id, char const * name)
  {
    params[9] = id;
    params[19] = manufacturer;
    strncpy(params + 15, name, 4);
  }

  unsigned char * getParams()
  {
    return params;
  }
private:
  // Memory for the params is allocated on the heap and handed over to CBUS.setParams().
  // This memory will never be freed.
  unsigned char * params;
};
