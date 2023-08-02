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