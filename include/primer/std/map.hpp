//  (C) Copyright 2015 - 2017 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * How to push `std::map` to the stack, as a table, and read it
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <map>
#include <primer/container/map_base.hpp>

namespace primer {

namespace traits {

template <typename T, typename U>
struct push<std::map<T, U>> : container::map_push_helper<std::map<T, U>> {};

template <typename T, typename U>
struct read<std::map<T, U>> : container::map_read_helper<std::map<T, U>> {};

} // end namespace traits

} // end namespace primer
