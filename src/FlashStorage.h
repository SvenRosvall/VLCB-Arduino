// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

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

  virtual byte read(unsigned int eeaddress) override;
  virtual byte readBytes(unsigned int eeaddress, byte nbytes, byte dest[]) override;
  virtual void write(unsigned int eeaddress, byte data) override;
  virtual void writeBytes(unsigned int eeaddress, const byte src[], byte numbytes) override;
  virtual void reset() override;

};

}