//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * How to transport `std::array` to and from the stack, as tables
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/container/seq_base.hpp>
#include <array>

namespace primer {

namespace traits {

template <typename T, std::size_t N>
struct push<std::array<T, N>> : detail::push_seq_helper<std::array<T, N>> {};

template <typename T, std::size_t N>
struct read<std::array<T, N>>
  : detail::read_fixed_seq_helper<std::array<T, N>> {};

} // end namespace traits

} // end namespace primer
