#pragma once

struct ENDL_T;

class Stream
{
public:
  Stream() = default;
  virtual ~Stream() = default;

  virtual bool available() { return false; }
  virtual char read() { return 0; }
  virtual void print(const char *) {}
  virtual void println(const char *) {}
  virtual void flush() {}
};

// Operator overloads for Stream to work with Streaming library
inline Stream & operator<<(Stream & s, int) { return s; }
inline Stream & operator<<(Stream & s, unsigned int) { return s; }
inline Stream & operator<<(Stream & s, long) { return s; }
inline Stream & operator<<(Stream & s, unsigned long) { return s; }
inline Stream & operator<<(Stream & s, const char *) { return s; }
inline Stream & operator<<(Stream & s, const ENDL_T &) { return s; }
