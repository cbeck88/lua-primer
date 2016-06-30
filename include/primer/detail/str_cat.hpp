//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * Helper function that takes string-like things and concatenates them.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <string>

namespace primer {
namespace detail {

inline std::string str_cat() { return {}; }

template <typename... Args>
std::string str_cat(std::string s, Args &&... args) {
  return s + str_cat(std::forward<Args>(args)...);
}

template <typename... Args>
std::string str_cat(const char * s, Args &&... args) {
  return s + str_cat(std::forward<Args>(args)...);
}

} // end namespace detail
} // end namespace primer
