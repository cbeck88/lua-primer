//  (C) Copyright 2015 - 2017 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * How to transport `boost::container::vector` to and from the stack
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <boost/container/vector.hpp>
#include <primer/container/seq_base.hpp>

namespace primer {
namespace traits {

template <typename T>
struct push<boost::container::vector<T>>
  : container::push_seq_helper<boost::container::vector<T>> {};

template <typename T>
struct read<boost::container::vector<T>>
  : container::read_seq_helper<boost::container::vector<T>> {};

} // end namespace traits
} // end namespace primer
