#pragma once

struct ENDL_T;

class Stream
{
public:
  Stream() = default;
  virtual ~Stream() = default;

  virtual void begin(int);
  virtual bool available();
  virtual char read();
  virtual void print(const char *);
  virtual void println(const char *);
  virtual void flush();
};
