//  (C) Copyright 2015 - 2018 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * Some simple code that "describes" a lua value on the stack in some
 * user-friendly way.
 * For error messages.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/lua.hpp>

namespace primer {

/// Generate an error message string describing a value at a given position.
inline const char *
describe_lua_value(lua_State * L, int idx) {
  return lua_typename(L, lua_type(L, idx));
}

} // end namespace primer
