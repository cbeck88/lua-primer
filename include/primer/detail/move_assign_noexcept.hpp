//  (C) Copyright 2015 - 2018 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * Tries very hard to find a way to move assign an object without generating
 * exceptions.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <new>
#include <primer/detail/type_traits.hpp>
#include <type_traits>
#include <utility>

namespace primer {
namespace detail {

/***
 * Primary implementation for move_assign_noexcept
 */
template <typename T>
auto
move_assign_noexcept(T & dest, T && src) noexcept
  -> enable_if_t<std::is_nothrow_move_assignable<T>::value> {
  dest = std::move(src);
}

using std::swap;

/***
 * is_nothrow_swappable
 *   This implementation is stolen from WG proposal 0185
 *   http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0185r1.html#Appendix
 *
 * However, MSVC 2015, 2017 fail here
 *   error C2660: 'std::swap': function does not take 1 arguments
 *
 * Newest MSVC (2015 update 3) provides std::is_nothrow_swappable.
 */

#if (!defined _MSC_VER)

struct do_is_nothrow_swappable {
  template <class T>
  static auto test(int)
    -> std::integral_constant<bool, noexcept(swap(std::declval<T &>(),
                                                  std::declval<T &>()))>;

  template <class>
  static std::false_type test(...);
};


template <typename T>
struct is_nothrow_swappable : decltype(do_is_nothrow_swappable::test<T>(0)) {};

#elif (_MSC_FULL_VER >= 190024210)

// This is C++17 but it is available in recent MSVC

template <typename T>
using is_nothrow_swappable = std::is_nothrow_swappable<T>;

#else

// For older MSVC I don't have a solution
template <typename T>
struct is_nothrow_swappable : std::false_type {};

#endif

/***
 * More overloads for `move_assign_noexcept` depending on `is_nothrow_swappable`
 */
template <typename T>
auto
move_assign_noexcept(T & dest, T && src) noexcept
  -> enable_if_t<!std::is_nothrow_move_assignable<T>::value
                 && is_nothrow_swappable<T>::value> {
  swap(dest, src);
}

template <typename T>
auto
move_assign_noexcept(T & dest, T && src) noexcept
  -> enable_if_t<!std::is_nothrow_move_assignable<T>::value
                 && !is_nothrow_swappable<T>::value
                 && std::is_nothrow_move_constructible<T>::value> {
  dest.~T();
  new (&dest) T(std::move(src));
}

} // end namespace detail
} // end namespace primer
