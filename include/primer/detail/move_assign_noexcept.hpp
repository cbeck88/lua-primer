//  (C) Copyright 2015 - 2016 Christopher Beck

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
#include <string>
#include <type_traits>
#include <utility>

namespace primer {
namespace detail {

template <typename T>
auto
move_assign_noexcept(T & dest, T && src) noexcept
  -> enable_if_t<std::is_nothrow_move_assignable<T>::value> {
  dest = std::move(src);
}

using std::swap;

// Need to use SFINAE for this in case there is no `swap`.
template <typename T, typename ENABLE = void>
struct is_nothrow_swappable : std::false_type {};

template <typename T>
struct is_nothrow_swappable<T, enable_if_t<noexcept(
                                 swap(*static_cast<T *>(nullptr),
                                      *static_cast<T *>(nullptr)))>>
  : std::true_type {};

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
