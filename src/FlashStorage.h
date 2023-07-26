#pragma once

#include "Storage.h"

#include <Arduino.h>                // for definition of byte datatype

// #ifdef __AVR_XMEGA__
#if defined(DXCORE)
#include <Flash.h>
//#else
//#error Can only use flash memory on DXCORE platforms.
#endif

namespace VLCB
{

#if defined(DXCORE)
// Note: Using #if here as PROGMEM_SIZE is only available on DXCORE
const int FLASH_AREA_BASE_ADDRESS = (PROGMEM_SIZE - (0x800));        // top 2K bytes of flash
#endif
const int FLASH_PAGE_SIZE = 512;
const int NUM_FLASH_PAGES = 4;

struct flash_page_t {
  bool dirty;
  uint8_t data[FLASH_PAGE_SIZE];
};

void flash_cache_page(const byte page);
bool flash_writeback_page(const byte page);
bool flash_write_bytes(const uint16_t address, const uint8_t *data, const uint16_t number);
byte flash_read_byte(const uint16_t address);
void flash_read_bytes(const uint16_t address, const uint16_t number, uint8_t *dest);

class FlashStorage : public Storage
{
public:
  virtual void begin() override;

  virtual byte readEEPROM(unsigned int eeaddress) override;
  virtual byte readBytesEEPROM(unsigned int eeaddress, byte nbytes, byte dest[]) override;
  virtual void writeEEPROM(unsigned int eeaddress, byte data) override;
  virtual void writeBytesEEPROM(unsigned int eeaddress, byte src[], byte numbytes) override;
  virtual void resetEEPROM(void) override;

};

}