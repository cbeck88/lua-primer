//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * The unwrap<T> template function takes expected<T> and reduces it to T.
 * If there is an exception, it is thrown.
 *
 * Note that this function is provided for the user, but is only used by the
 * library if PRIMER_NO_EXCEPTIONS is not defined.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/expected.hpp>
#include <utility>

namespace primer {

template <typename T>
T unwrap(expected<T> e) {
  if (e) {
    return *std::move(e);
  } else {
    throw std::move(e).err();
  }
}

} // end namespace primer
