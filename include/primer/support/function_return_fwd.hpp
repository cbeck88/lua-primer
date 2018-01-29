//  (C) Copyright 2015 - 2018 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * Support code related to capturing return values of a function.
 *
 * Helps to write generic code which works regardless of number of returns
 * expected.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/error.hpp>
#include <primer/expected.hpp>

namespace primer {
namespace detail {

template <typename T>
struct return_helper;

template <>
struct return_helper<void> {
  using return_type = expected<void>;

  static void pop(lua_State *, int, return_type & result) noexcept {
    result = {};
  }
  static constexpr int nrets = 0;
};

} // end namespace detail
} // end namespace primer
