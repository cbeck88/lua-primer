//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * Helper trait to test if an object is correctly registered as userdata
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/traits/userdata.hpp>
#include <primer/detail/type_traits.hpp>

#include <type_traits>
#include <utility>

namespace primer {
namespace detail {

/***
 * Check if the userdata trait is specialized for a given type.
 */
template <typename T, typename ENABLE = void>
struct is_userdata : std::false_type {};

template <typename T>
struct is_userdata<
  T,
  enable_if_t<std::is_same<decltype(primer::traits::userdata<T>::name),
                           decltype(primer::traits::userdata<T>::name)>::value>>
  : std::true_type {};

} // end namespace traits
} // end namespace primer
