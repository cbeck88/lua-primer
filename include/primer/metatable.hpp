//  (C) Copyright 2015 - 2017 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * Easy interface to metatable creation system.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/lua.hpp>
#include <primer/support/asserts.hpp>
#include <primer/support/metatable.hpp>
#include <primer/traits/userdata.hpp>

namespace primer {

// Fetches or initializes the metatable, leaves it on the stack.
template <typename T>
void
push_metatable(lua_State * L) {
  if (luaL_newmetatable(L, primer::traits::userdata<T>::name)) {
    primer::detail::metatable<T>::populate(L);
  }
}

// Calls the above, but pops result from the stack
template <typename T>
void
init_metatable(lua_State * L) {
  PRIMER_ASSERT_STACK_NEUTRAL(L);
  primer::push_metatable<T>(L);
  lua_pop(L, 1);
}

// Sanity check: Validate that primer can see the metatable of your userdata
// type
template <typename T>
constexpr bool
has_metatable() {
  return primer::detail::metatable<T>::value;
}

} // end namespace primer
