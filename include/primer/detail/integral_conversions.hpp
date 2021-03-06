//  (C) Copyright 2015 - 2018 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * Trait for converting unsigned to signed integers in a efficient and mostly
 * portable way.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <limits>
#include <primer/support/asserts.hpp>
#include <type_traits>

namespace primer {
namespace detail {

template <typename T>
T
unsigned_to_signed(typename std::make_unsigned<T>::type x) {
  using U = decltype(x);

  constexpr T max = std::numeric_limits<T>::max();
  constexpr T min = std::numeric_limits<T>::min();

  if (x <= static_cast<U>(max)) { return static_cast<T>(x); }

  if (x >= static_cast<U>(min)) { return static_cast<T>(x - min) + min; }

  PRIMER_ASSERT(false, "Bad unsigned -> signed integer conversion! x = " << x);
  return 0;
}

} // end namespace detail
} // end namespace primer
