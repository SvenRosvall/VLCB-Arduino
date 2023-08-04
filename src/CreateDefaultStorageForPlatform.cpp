// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#include "Storage.h"

#if defined(__SAM38E__)
#include "DueEepromEmulationStorage.h"
#elif defined(DXCORE)
#include "FlashStorage.h"
#else
#include "EepromInternalStorage.h"
#endif

namespace VLCB
{

Storage * createDefaultStorageForPlatform()
{
#if defined(__SAM38E__)
  static DueEepromEmulationStorage storage;
  return &storage;

#elif defined(DXCORE)
  static FlashStorage storage;
  return &storage;

#else
  static EepromInternalStorage storage;
  return &storage;
#endif
}

}