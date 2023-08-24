#pragma once

#include <Arduino.h>

struct TwoWire
{
    void begin();
    void beginTransmission(byte);
    byte endTransmission();
    void write(int i);
    byte read();
    bool available();
    void requestFrom(int i, int i1);
};

extern TwoWire Wire;