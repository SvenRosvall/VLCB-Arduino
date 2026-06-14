#pragma once

#include <Arduino.h>
#include <Stream.h>

struct ENDL_T {};

Stream & operator<<(Stream &, int);
Stream & operator<<(Stream &, unsigned int);
Stream & operator<<(Stream &, long);
Stream & operator<<(Stream &, unsigned long);
Stream & operator<<(Stream &, const char *);
Stream & operator<<(Stream & s, const ENDL_T & e);

extern ENDL_T endl;
template <typename T> T _HEX(T v) { return v;}
template <typename T> T _WIDTH(T v, int width) { return v;}
template <typename T> T _WIDTHZ(T v, int width) { return v;}
inline const char* _FMT(const char* fmt, ...) { return fmt;}
