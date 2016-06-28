//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * How to push `boost::flat_set` to the stack, as a table, using the lua "set" idiom.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/std/set_base.hpp>
#include <boost/container/flat_set.hpp>

namespace primer {

namespace traits {

template <typename T>
struct push<boost::container::flat_set<T>> : detail::set_push_helper<boost::container::flat_set<T>> {};

template <typename T>
struct read<boost::container::flat_set<T>> : detail::set_read_helper<boost::container::flat_set<T>> {};

} // end namespace traits

} // end namespace primer
