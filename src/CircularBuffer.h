//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0

#pragma once

#include <stdint.h>

namespace VLCB
{

template <typename E, int bufferCapacity = 4>
class CircularBuffer
{
public:
  explicit CircularBuffer();
  ~CircularBuffer();

  bool available() const;
  E *peek();
  const E & pop();
  void put(const E &entry);
  void clear();

  // Diagnostic metrics access
  uint8_t bufUse() const;
  unsigned int getNumberOfPuts() const { return numPuts; }
  unsigned int getNumberOfGets() const { return numGets; }
  unsigned int getOverflows() const { return numOverflows; }
  unsigned int getHighWaterMark() const { return hwm; }

private:

  uint8_t capacity;
  uint8_t head = 0;
  uint8_t tail = 0;
  bool full = false;

  // Diagnostic metrics
  uint8_t hwm = 0;  // High water Mark
  uint8_t numPuts = 0;
  uint8_t numGets = 0;
  uint8_t numOverflows = 0;

  E buffer[bufferCapacity];
};


template <typename E, int bufferCapacity>
CircularBuffer<E, bufferCapacity>::CircularBuffer()
        : capacity(bufferCapacity)
{}

template <typename E, int bufferCapacity>
CircularBuffer<E, bufferCapacity>::~CircularBuffer()
{
}

/// if buffer has one or more stored items
template <typename E, int bufferCapacity>
bool CircularBuffer<E, bufferCapacity>::available() const
{
  return full || (head != tail);
}

/// store an item to the buffer - overwrite the oldest item if buffer is full
/// never called from an interrupt context so we don't need to worry about interrupts
template <typename E, int bufferCapacity>
void CircularBuffer<E, bufferCapacity>::put(const E &entry)
{
  buffer[head] = entry;

  if (full)
  {
    // if the buffer is full, this put will overwrite the oldest item
    tail = (tail + 1) % capacity;
    ++numOverflows;
  }

  head = (head + 1) % capacity;
  full = head == tail;
  uint8_t size = bufUse();
  if (size > hwm)
  {
    // Tracks high water mark (hwm)
    hwm = size;
  }
  ++numPuts;        // Counts how many events put to buffer.
  // DEBUG_SERIAL << ">COE Puts = " << numPuts << " Size = " << size <<endl;
}

/// retrieve the next item from the buffer, requires that available() is checked first.
template <typename E, int bufferCapacity>
const E & CircularBuffer<E, bufferCapacity>::pop()
{
  uint8_t oldTail = tail;
  full = false;
  tail = (tail + 1) % capacity;
  ++numGets;        // Counts how many events got from buffer.
  // DEBUG_SERIAL << ">COE Gets = " << numGets << endl;
  return buffer[oldTail];
}

/// peek at the next item in the buffer without removing it
template <typename E, int bufferCapacity>
E *CircularBuffer<E, bufferCapacity>::peek()
{
  // should always call ::available first to avoid this
  if (available())
  {
    return &buffer[tail];
  }
  else
  {
    return nullptr;
  }
}

/// clear all items
template <typename E, int bufferCapacity>
void CircularBuffer<E, bufferCapacity>::clear()
{
  head = 0;
  tail = 0;
  full = false;
}

/// recalculate number of items in the buffer
template <typename E, int bufferCapacity>
uint8_t CircularBuffer<E, bufferCapacity>::bufUse() const
{
  int8_t size = capacity;
  if (!full)
  {
    size = head - tail;
    if (size < 0)
    {
      size += capacity;
    }
  }
  return size;
}

}