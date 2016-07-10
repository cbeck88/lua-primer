//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <utility>

namespace primer {

namespace detail {

template <typename F>
constexpr int right_assoc_int(F &&, int x) { return x; }

template <typename F, typename... Args>
constexpr int right_assoc_int(F && f, int x, Args &&... args) {
  return std::forward<F>(f)(x, right_assoc_int(std::forward<F>(f), std::forward<Args>(args)...));
}

inline constexpr int max(int x, int y) { return x > y ? x : y; }
inline constexpr int min(int x, int y) { return x < y ? x : y; }

template <typename... Args>
constexpr int max_int(int x, Args &&... args) {
  return right_assoc_int(&primer::detail::max, x, std::forward<Args>(args)...);
}

template <typename... Args>
constexpr int min_int(int x, Args &&... args) {
  return right_assoc_int(&primer::detail::min, x, std::forward<Args>(args)...);
}
  
} // end namespace detail

} // end namespace primer
