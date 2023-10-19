//  Copyright (C) Sven Rosvall (sven@rosvall.ie)
//  This file is part of VLCB-Arduino project on https://github.com/SvenRosvall/VLCB-Arduino
//  Licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//  The full licence can be found at: http://creativecommons.org/licenses/by-nc-sa/4.0
//

#pragma once

#include "initializer_list.h"

namespace VLCB
{

template<typename E>
class ArrayHolder
{
public:
  ArrayHolder(const std::initializer_list<E> & il);
  ~ArrayHolder();

  // Number of elements.
  constexpr size_t
  size() const noexcept { return len; }

  // First element.
  constexpr const E*
  begin() const noexcept { return array; }

  // One past the last element.
  constexpr const E*
  end() const noexcept { return begin() + size(); }
  
  constexpr const E
  operator[](size_t index) const noexcept { return array[index]; }

private:
  static E* copyArray(const E * a, size_t len);

  const E* array;
  size_t len;
};

template<typename E>
ArrayHolder<E>::ArrayHolder(const std::initializer_list<E> &il)
  : array(copyArray(il.begin(), il.size()))
  , len(il.size())
{ }

template<typename E>
ArrayHolder<E>::~ArrayHolder()
{
  delete array;
  array = 0;
  len = 0;
}

template<typename E>
E* ArrayHolder<E>::copyArray(const E * a, size_t len)
{
  E * array = new E[len];
  for (int i = 0 ; i < len ; ++i)
  {
    array[i] = a[i];
  }
  return array;
}

}
