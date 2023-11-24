// Copyright (C) Martin Da Costa 2023 (martindc.merg@gmail.com)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

#include <Streaming.h>
#include "ConsumeOwnEventsService.h"
#include <vlcbdefs.hpp>

namespace VLCB
{

ConsumeOwnEventsService::ConsumeOwnEventsService(byte bufferCapacity)
  : capacity(bufferCapacity)
{
  buffer = new VlcbMessage[capacity];
}

ConsumeOwnEventsService::~ConsumeOwnEventsService()
{
  delete[] buffer;
}

/// if buffer has one or more stored items

bool ConsumeOwnEventsService::available(void)
{
  return (size > 0);
}

/// store an item to the buffer - overwrite oldest item if buffer is full
/// never called from an interrupt context so we don't need to worry about interrupts

void ConsumeOwnEventsService::put(const VlcbMessage * msg)
{
//  VlcbMessage msg;
  memcpy(&buffer[head], msg, sizeof(VlcbMessage));

  // if the buffer is full, this put will overwrite the oldest item

  if (full)
  {
    tail = (tail + 1) % capacity;
    ++numOverflows;
  }

  head = (head + 1) % capacity;
  full = head == tail;
  size = bufUse();
  hwm = (size > hwm) ? size : hwm;   // Tracks high water mark (hwm)
  ++numPuts;        // Counts how many events put to buffer.
  // DEBUG_SERIAL << ">COE Puts = " << numPuts << " Size = " << size <<endl;

  return;
}

/// retrieve the next item from the buffer

VlcbMessage *ConsumeOwnEventsService::get(void)
{
  VlcbMessage *p = nullptr;

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

VlcbMessage *ConsumeOwnEventsService::peek(void)
{
  // should always call ::available first to avoid this
  if (size == 0)
  {
    return nullptr;
  }

  return (&buffer[tail]);
}

/// clear all items

void ConsumeOwnEventsService::clear(void)
{
  head = 0;
  tail = 0;
  full = false;
  size = 0;

  return;
}

/// recalculate number of items in the buffer

byte ConsumeOwnEventsService::bufUse(void)
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

unsigned int ConsumeOwnEventsService::getNumberOfPuts(void)
{
  return numPuts;
}

/// number of gets

unsigned int ConsumeOwnEventsService::getNumberofGets(void)
{
  return numGets;
}

/// number of overflows

unsigned int ConsumeOwnEventsService::getOverflows(void)
{
  return numOverflows;
}

unsigned int ConsumeOwnEventsService::getHighWaterMark(void)
{
  return hwm;
}

}
