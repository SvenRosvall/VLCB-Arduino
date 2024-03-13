//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0
//
//

#pragma once

#include <Storage.h>
#include <vector>

class MockStorage : public VLCB::Storage
{
public:
  MockStorage();
  virtual void begin() override;
  virtual byte read(unsigned int eeaddress) override;
  virtual void write(unsigned int eeaddress, byte data) override;
  virtual byte readBytes(unsigned int eeaddress, byte nbytes, byte dest[]) override;
  virtual void writeBytes(unsigned int eeaddress, const byte src[], byte numbytes) override;
  virtual void reset() override;
  
private:
  std::vector<byte> eeprom;
};
