//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * How to transport `std::vector` to and from the stack, as a tables
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/container/seq_base.hpp>
#include <vector>

namespace primer {

namespace traits {

template <typename T>
struct push<std::vector<T>> : container::push_seq_helper<std::vector<T>> {};

template <typename T>
struct read<std::vector<T>> : container::read_seq_helper<std::vector<T>> {};

} // end namespace traits

} // end namespace primer
