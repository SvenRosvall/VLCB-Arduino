//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

#pragma once

#include <stdint.h>

namespace VLCB
{

template <typename E>
class CircularBuffer
{
public:
  explicit CircularBuffer(uint8_t bufferCapacity = 4);
  ~CircularBuffer();

  bool available();
  E *peek();
  E *get();
  void put(const E *entry);
  void clear();

  // Diagnostic metrics access
  unsigned int getNumberOfPuts();
  unsigned int getNumberOfGets();
  unsigned int getOverflows();
  unsigned int getHighWaterMark();   // High Watermark

private:
  uint8_t bufUse();

  uint8_t capacity;
  uint8_t head = 0;
  uint8_t tail = 0;
  uint8_t size = 0;
  bool full = false;

  // Diagnostic metrics
  uint8_t hwm = 0;  // High watermark
  uint8_t numPuts = 0;
  uint8_t numGets = 0;
  uint8_t numOverflows = 0;

  E *buffer;
};


template <typename E>
CircularBuffer<E>::CircularBuffer(uint8_t bufferCapacity)
        : capacity(bufferCapacity)
        , buffer(new E[capacity])
{}

template <typename E>
CircularBuffer<E>::~CircularBuffer()
{
  delete[] buffer;
}

/// if buffer has one or more stored items
template <typename E>
bool CircularBuffer<E>::available()
{
  return (size > 0);
}

/// store an item to the buffer - overwrite oldest item if buffer is full
/// never called from an interrupt context so we don't need to worry about interrupts
template <typename E>
void CircularBuffer<E>::put(const E * msg)
{
//  E msg;
  buffer[head] = *msg;

  if (full)
  {
    // if the buffer is full, this put will overwrite the oldest item
    tail = (tail + 1) % capacity;
    ++numOverflows;
  }

  head = (head + 1) % capacity;
  full = head == tail;
  size = bufUse();
  hwm = (size > hwm) ? size : hwm;   // Tracks high water mark (hwm)
  ++numPuts;        // Counts how many events put to buffer.
  // DEBUG_SERIAL << ">COE Puts = " << numPuts << " Size = " << size <<endl;
}

/// retrieve the next item from the buffer
template <typename E>
E *CircularBuffer<E>::get()
{
  E *p = nullptr;

  if (size > 0)
  {
    p = &buffer[tail];
    full = false;
    tail = (tail + 1) % capacity;
    size = bufUse();
    ++numGets;        // Counts how many events got from buffer.
    // DEBUG_SERIAL << ">COE Gets = " << numGets << endl;
  }

  return p;
}

/// peek at the next item in the buffer without removing it
template <typename E>
E *CircularBuffer<E>::peek()
{
  // should always call ::available first to avoid this
  if (size == 0)
  {
    return nullptr;
  }

  return &buffer[tail];
}

/// clear all items
template <typename E>
void CircularBuffer<E>::clear()
{
  head = 0;
  tail = 0;
  full = false;
  size = 0;
}

/// recalculate number of items in the buffer
template <typename E>
uint8_t CircularBuffer<E>::bufUse()
{
  if (full)
  {
    return capacity;
  }

  if (head >= tail)
  {
    return head - tail;
  }
  else
  {
    return capacity + head - tail;
  }
}

/// number of puts
template <typename E>
unsigned int CircularBuffer<E>::getNumberOfPuts()
{
  return numPuts;
}

/// number of gets
template <typename E>
unsigned int CircularBuffer<E>::getNumberOfGets()
{
  return numGets;
}

/// number of overflows
template <typename E>
unsigned int CircularBuffer<E>::getOverflows()
{
  return numOverflows;
}

template <typename E>
unsigned int CircularBuffer<E>::getHighWaterMark()
{
  return hwm;
}

}