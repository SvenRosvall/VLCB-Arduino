// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#include "FlashStorage.h"

namespace VLCB
{

// #ifdef __AVR_XMEGA__
#if defined(DXCORE)
flash_page_t cache_page;                // flash page cache
byte curr_page_num = 0;
#endif


void FlashStorage::begin()
{
  // check flash is writable
#if defined(DXCORE)  // Flash is only available for DXCORE platforms
  byte check = Flash.checkWritable();

  if (check == FLASHWRITE_OK)
  {
    // DEBUG_SERIAL << F("> flash is writable, PROGMEM_SIZE = ") << PROGMEM_SIZE << endl;
  }
  else
  {
    // DEBUG_SERIAL << F("> flash is not writable, ret = ") << check << endl;
  }

  // cache the first page into memory
  flash_cache_page(0);
#endif
}


//
/// read a single byte from EEPROM
//
byte FlashStorage::readEEPROM(unsigned int eeaddress)
{
  byte rdata = 0;

// #ifdef __AVR_XMEGA__
#if defined(DXCORE)
  rdata = Flash.readByte(FLASH_AREA_BASE_ADDRESS + eeaddress);
  // DEBUG_SERIAL << F("> read byte = ") << rdata << F(" from address = ") << eeaddress << endl;
#endif

  return rdata;
}


//
/// read a number of bytes from EEPROM
/// external EEPROM must use 16-bit addresses !!
//
byte FlashStorage::readBytesEEPROM(unsigned int eeaddress, byte nbytes, byte dest[])
{
  byte count = 0;

// #ifdef __AVR_XMEGA__
#if defined(DXCORE)
  for (count = 0; count < nbytes; count++) {
    dest[count] = Flash.readByte(FLASH_AREA_BASE_ADDRESS + eeaddress + count);
  }
#endif

  return count;
}


//
/// write a byte
//
void FlashStorage::writeEEPROM(unsigned int eeaddress, byte data)
{
// #ifdef __AVR_XMEGA__
#if defined(DXCORE)
  flash_write_bytes(eeaddress, &data, 1);
#endif
}

//
/// write a number of bytes to EEPROM
/// external EEPROM must use 16-bit addresses !!
//
void FlashStorage::writeBytesEEPROM(unsigned int eeaddress, byte src[], byte numbytes)
{
// #ifdef __AVR_XMEGA__
#if defined(DXCORE)
  flash_write_bytes(eeaddress, src, numbytes);
#endif
}

//
/// clear all event data in external EEPROM chip
//
void FlashStorage::resetEEPROM()
{
// #ifdef __AVR_XMEGA__
#if defined(DXCORE)
  for (byte i = 0; i < 4; i++)
  {
    memset(cache_page.data, 0xff, FLASH_PAGE_SIZE);
    cache_page.dirty = true;
    flash_writeback_page(i);
  }
#endif
}

//
/// flash routines for AVR-Dx devices
/// we allocate 2048 bytes at the far end of flash, and cache the data of one of four 512 byte pages (0-3)
/// dirty pages must be erased and written back before moving on
//

// #ifdef __AVR_XMEGA__
#if defined(DXCORE)
// cache a page of flash into memory

void flash_cache_page(const byte page)
{
  const uint32_t address_base = FLASH_AREA_BASE_ADDRESS + (page * FLASH_PAGE_SIZE);

  // DEBUG_SERIAL << F("> flash_cache_page, page = ") << page << endl;

  for (unsigned int a = 0; a < FLASH_PAGE_SIZE; a++)
  {
    cache_page.data[a] = Flash.readByte(address_base + a);
  }

  cache_page.dirty = false;
  return;
}

// write out a cached page to flash

bool flash_writeback_page(const byte page)
{
  bool ret = true;
  uint32_t address;

  // DEBUG_SERIAL << F("> flash_writeback_page, page = ") << curr_page_num << F(", dirty = ") << cache_page.dirty << endl;

  if (cache_page.dirty)
  {
    address = FLASH_AREA_BASE_ADDRESS + (FLASH_PAGE_SIZE * page);

    // erase the existing page of flash memory
    ret = Flash.erasePage(address, 1);

    if (ret != FLASHWRITE_OK)
    {
      // DEBUG_SERIAL.printf(F("error erasing flash page\r\n"));
    }

    // write the nexw data
    ret = Flash.writeBytes(address, cache_page.data, FLASH_PAGE_SIZE);

    if (ret != FLASHWRITE_OK)
    {
      // DEBUG_SERIAL.printf(F("error writing flash data\r\n"));
    }

    cache_page.dirty = false;
  }

  return ret;
}

// write one or more bytes into the page cache, handling crossing a page boundary
// address is the index into the flash area (0-2047), not the absolute memory address
bool flash_write_bytes(const uint16_t address, const uint8_t *data, const uint16_t number)
{
  bool ret = true;

  // DEBUG_SERIAL << F("> flash_write_bytes: address = ") << address << F(", data = ") << *data << F(", length = ") << length << endl;

  if (address >= (FLASH_PAGE_SIZE * NUM_FLASH_PAGES))
  {
    // DEBUG_SERIAL.printf(F("cache page address = %u is out of bounds\r\n"), address);
  }

  // calculate page number, 0-3
  byte new_page_num = address / FLASH_PAGE_SIZE;

  // DEBUG_SERIAL << F("> curr page = ") << curr_page_num << F(", new = ") << new_page_num << endl;

  // erase and write back current page if cache page number is changing
  if (new_page_num != curr_page_num)
  {
    flash_writeback_page(curr_page_num);                // write back current cache page
    curr_page_num = new_page_num;                       // set new cache page num
    flash_cache_page(curr_page_num);                    // cache the new page data from flash
    cache_page.dirty = false;                           // set flag
  }

  // calculate the address offset into the page cache buffer
  uint16_t buffer_index = address % FLASH_PAGE_SIZE;

  // write data into cached page buffer
  for (uint16_t a = 0; a < number; a++)
  {
    // we are crossing a page boundary ...
    if (buffer_index >= FLASH_PAGE_SIZE)
    {
      // DEBUG_SERIAL.printf(F("crossing page boundary\r\n"));
      cache_page.dirty = true;
      flash_writeback_page(curr_page_num);              // write back current cache page
      ++curr_page_num;                                  // increment the page number -- should really handle end of flash area
      flash_cache_page(curr_page_num);                  // cache the new page data from flash
      buffer_index = 0;                                 // set cache buffer index to zero
    }

    cache_page.data[buffer_index + a] = data[a];
  }

  // set page dirty flag
  cache_page.dirty = true;

  // write the cache page to flash
  flash_writeback_page(curr_page_num);

  return ret;
}

// read one byte from flash
// not via cache
byte flash_read_byte(uint32_t address)
{
  return Flash.readByte(address);
}

// read multiple bytes from flash
// address is internal address map offset
// not through cache
void flash_read_bytes(const uint16_t address, const uint16_t number, uint8_t *dest)
{
  uint32_t base_address = FLASH_AREA_BASE_ADDRESS + address;

  for (uint32_t a = 0; a < number; a++) {
    dest[a] = flash_read_byte(base_address + a);
  }

  return;
}

#endif

}