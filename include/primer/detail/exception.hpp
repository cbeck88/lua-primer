//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * Primer exception is a simple exception type used by the library. It is not
 * thrown to user generated code, it is only used internally, in one
 * implementation of the "adapt" mechanism. 
 *
 * This file should not be used in PRIMER_NO_EXCEPTIONS builds
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/error.hpp>
#include <primer/expected.hpp>

#include <exception>
#include <string>
#include <utility>

namespace primer {
namespace detail {

/***
 * Actual exception object which can be thrown
 */

class exception : public std::runtime_error {
public:
  explicit exception(primer::error e)
    : std::runtime_error(e.str())
  {}
};

/***
 * Unwrap class takes an expected<T> type and unwraps it, throwing an exception
 * to signal the error.
 */

template <typename T>
T unwrap(expected<T> e) {
  if (e) {
    return *std::move(e);
  } else {
    throw primer::detail::exception{std::move(e).err()};
  }
}

} // end namespace detail
} // end namespace primer
