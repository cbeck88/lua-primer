//  (C) Copyright 2015 - 2018 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * General purpose "read from stack" interface.
 * Backed up by the 'read' trait, see primer/traits/read.hpp
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/expected.hpp>
#include <primer/support/asserts.hpp>
#include <primer/traits/read.hpp>

namespace primer {

//[ primer_read
template <typename T>
expected<T> read(lua_State * L, int index);
//]

//[ primer_read_impl
template <typename T>
expected<T>
read(lua_State * L, int index) {
  PRIMER_ASSERT_STACK_NEUTRAL(L);
  return ::primer::traits::read<T>::from_stack(L, index);
  //<-
  // Silence an unused warning
  static_cast<void>(::primer::traits::read<T>::stack_space_needed);
  //->
}
//]

//[ primer_stack_space_for_read
template <typename T>
constexpr int stack_space_for_read();
//]

template <typename T>
constexpr int
stack_space_for_read() {
  return ::primer::traits::read<T>::stack_space_needed;
}

} // end namespace primer
