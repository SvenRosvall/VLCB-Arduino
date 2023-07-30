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
