// Copyright (C) Sven Rosvall (sven@rosvall.ie)
// This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
// Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
// The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0/

// Use the same include guard as G++ uses so that this doesn't clash for running the test suite.
#ifndef _INITIALIZER_LIST
#define _INITIALIZER_LIST

#include <stddef.h>

namespace std
{
  template<class E>
    class initializer_list
    {
    public:

      constexpr initializer_list() noexcept : array(0), len(0) { }
      
      // This copy ctor is only used in test code for creating a controller.
      initializer_list(const initializer_list & rhs)
        : len(rhs.len)
        , array(rhs.array)
      {
      }

      // Number of elements.
      constexpr size_t
      size() const noexcept { return len; }

      // First element.
      constexpr const E*
      begin() const noexcept { return array; }

      // One past the last element.
      constexpr const E*
      end() const noexcept { return begin() + size(); }

      // private:
      // The compiler uses this
      constexpr initializer_list(const E* a, size_t l)
        : array(a), len(l) { }
        
      const E* array;
      size_t len;
    };

  template<class T>
    constexpr const T*
    begin(initializer_list<T> ils) noexcept
    { return ils.begin(); }

  template<class T>
    constexpr const T*
    end(initializer_list<T> ils) noexcept
    { return ils.end(); }
}

#endif