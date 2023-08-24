#pragma once

struct Serial_T
{
  void begin(int baudrate);

  bool available();
  char read();
  void flush();
  unsigned char readBytesUntil(int termChar, char *string, int length);
  void println(const char *);
};
extern struct Serial_T Serial;

Serial_T & operator<<(Serial_T &, int);
Serial_T & operator<<(Serial_T &, unsigned int);
Serial_T & operator<<(Serial_T &, long);
Serial_T & operator<<(Serial_T &, unsigned long);
Serial_T & operator<<(Serial_T &, const char *);
extern int endl;
template <typename T> T _HEX(T);