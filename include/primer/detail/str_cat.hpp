//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * Helper function that takes string-like things and concatenates them.
 * Also takes numbers and calls "std::to_string" on them.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/detail/type_traits.hpp>

#include <string>
#include <type_traits>
#include <utility>

namespace primer {
namespace detail {

// Trait
template <typename T>
struct str_cat_helper {};

template <>
struct str_cat_helper<std::string> {
  static std::string to_string(std::string s) { return s; }
};

template <>
struct str_cat_helper<const char *> {
  static std::string to_string(const char * s) { return s; }
};

template <>
struct str_cat_helper<int> {
  static std::string to_string(int i) { return std::to_string(i); }
};

template <>
struct str_cat_helper<long int> {
  static std::string to_string(long int i) { return std::to_string(i); }
};

template <>
struct str_cat_helper<long long int> {
  static std::string to_string(long long int i) { return std::to_string(i); }
};


// Template function
inline std::string str_cat() { return {}; }

template <typename T, typename... Args>
std::string str_cat(T && t, Args &&... args) {
  using helper_t = str_cat_helper<remove_reference_t<remove_cv_t<decay_t<T>>>>;
  return helper_t::to_string(std::forward<T>(t)) +
         str_cat(std::forward<Args>(args)...);
}

} // end namespace detail
} // end namespace primer
