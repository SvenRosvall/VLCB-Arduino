
/*

  Copyright (C) Duncan Greenwood 2017 (duncan_greenwood@hotmail.com)

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

//
/// misc module-specific definitions
//

#if !defined __DEFS_H__
#define __DEFS_H__

#include <Arduino.h>                            // for definition of byte datatype

// constants
static const byte VER_MAJ = 1;                  // code major version
static const char VER_MIN = 'a';                // code minor version
static const byte VER_BETA = 0;                 // code beta sub-version
static const byte MODULE_ID = 99;               // CBUS module type

static const byte LED_GRN = 4;                  // CBUS green SLiM LED pin
static const byte LED_YLW = 5;                  // CBUS yellow FLiM LED pin

static const byte SWITCH0 = 6;                  // CBUS push button switch pin

#endif
