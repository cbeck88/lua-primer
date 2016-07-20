//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * How to push `std::unordered_map` to the stack, as a table
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/container/map_base.hpp>
#include <unordered_map>

namespace primer {

namespace traits {

template <typename T, typename U>
struct push<std::unordered_map<T, U>>
  : container::map_push_helper<std::map<T, U>> {};

template <typename T, typename U>
struct read<std::unordered_map<T, U>>
  : container::map_read_helper<std::map<T, U>> {};

} // end namespace traits

} // end namespace primer
