// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#include <Parameters.h>

namespace VLCB
{

unsigned char Parameters::params[21];

void Parameters::initProcessorParams()
{
  params[9] = 50;
  params[19] = CPUM_ATMEL;

  // set of pre-defined macros are taken from
  // https://github.com/backupbrain/ArduinoBoardManager/blob/master/ArduinoBoardManager.h
  // TODO: Provide values for all board types.
#if defined(__AVR_ATmega328P__) // uno, fio
  params[15] = '3';
  params[16] = '2';
  params[17] = '8';
  params[18] = 'P';
//#elif defined(__AVR_ATSAMD21G18A__) // zero
#elif defined(__AVR_ATSAM3X8E__) // Due
  params[15] = '3';
  params[16] = 'X';
  params[17] = '8';
  params[18] = 'E';
//#elif defined(__AVR_Atmega32U4__) // Yun 16Mhz, Micro, Leonardo, Esplora
//#elif defined(_AVR_AR9331__) // Yun 400Mhz
#elif defined(__AVR_ATmega16U4__) // leonardo
  params[15] = '1';
  params[16] = '6';
  params[17] = 'U';
  params[18] = '4';
#elif defined(__AVR_ATmega1280__) // mega, Mega ADK
  params[15] = '1';
  params[16] = '2';
  params[17] = '8';
  params[18] = '0';
#elif defined(__AVR_ATmega2560__) // mega, Mega ADK
  params[15] = '2';
  params[16] = '5';
  params[17] = '6';
  params[18] = '0';
//#elif defined(_AVR_ATmega328__) // Nano >= v3.0 or Arduino Pro, pro328, ethernet
//#elif defined(_AVR_ATmega168__) // Nano < v3.0 or uno, pro
//#elif defined(_AVR_ATmega168V__) // LilyPad
//#elif defined(_AVR_ATmega328V__) // LilyPad 2
//#elif defined(_AVR_ATTiny85__) // trinket
//#elif  defined(__AVR_ARCv2EM__) || (__CURIE_FACTORY_DATA_H_) // Intel Curie/101
#else
  params[15] = '?';
  params[16] = '?';
  params[17] = '?';
  params[18] = '?';
#endif
}

}