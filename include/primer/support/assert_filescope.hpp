//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * PRIMER_ASSERT_FILESCOPE is a static assertion that causes a compilation
 * error if it is enclosed in a namespace. It does nothing at all
 * if the declaration is at filescope. (Well, nothing is odr-used.)
 */

// Fall through to this value if we can't see the test symbol
template <int N>
struct primer_assert_filescope_check_ {
  static constexpr bool value = false;
};

#define PRIMER_TOKEN_PASTE(x, y) x##y
#define PRIMER_ASSERT_FILESCOPE_CONCAT(x, y) PRIMER_TOKEN_PASTE(x, y)

#define PRIMER_ASSERT_FILESCOPE PRIMER_ASSERT_FILESCOPE_I(__COUNTER__)

#define PRIMER_ASSERT_FILESCOPE_I(counter)                                                         \
  template <int>                                                                                   \
  struct primer_assert_filescope_check_;                                                           \
                                                                                                   \
  template <>                                                                                      \
  struct primer_assert_filescope_check_<counter> {                                                 \
    static constexpr bool value = true;                                                            \
  };                                                                                               \
                                                                                                   \
  static_assert(::primer_assert_filescope_check_<counter>::value, "Assert filescope failed!")
