//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * How to push `std::set` to the stack, as a table, using the lua "set" idiom.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/std/set_base.hpp>
#include <set>

namespace primer {

namespace traits {

template <typename T>
struct push<std::set<T>> : detail::set_push_helper<std::set<T>> {};

template <typename T>
struct read<std::set<T>> : detail::set_read_helper<std::set<T>> {};

} // end namespace traits

} // end namespace primer
