//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * Check if a type has a declared specialization of primer::traits::optional
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/traits/optional.hpp>
#include <primer/traits/util.hpp>
#include <type_traits>

namespace primer {
namespace traits {

template <typename T, typename ENABLE = void>
struct is_optional : std::false_type {};

template <typename T>
struct is_optional<
  T,
  enable_if_t<std::is_same<typename primer::traits::optional<T>::base_type,
                           typename primer::traits::optional<T>::base_type>::value>>
  : std::true_type {};

template <typename T, typename ENABLE = void>
struct is_relaxed_optional : std::false_type {};

template <typename T>
struct is_relaxed_optional<T, enable_if_t<primer::traits::optional<T>::relaxed>>
  : std::true_type {};

} // end namespace traits
} // end namespace primer
