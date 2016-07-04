//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * Useful function that registers a null-terminated array of luaL_Reg-like
 * objects with a table on top of the stack. Similar to luaL_setfuncs.
 *
 * set_funcs_reverse uses the fuction as a key, as needed for persist table.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/lua.hpp>
#include <primer/support/asserts.hpp>

namespace primer {

template <typename T>
void set_funcs(lua_State * L, const T * ptr) {
  PRIMER_ASSERT_TABLE(L);

  for (; ptr->name; ++ptr) {
    if (ptr->func) {
      lua_pushcfunction(L, ptr->func);
      lua_setfield(L, -2, ptr->name);
    }
  }
}

template <typename T>
void set_funcs_reverse(lua_State * L, const T * ptr) {
  PRIMER_ASSERT_TABLE(L);

  for (; ptr->name; ++ptr) {
    if (ptr->func) {
      lua_pushstring(L, ptr->name);
      lua_pushcfunction(L, ptr->func);
      lua_settable(L, -3);
    }
  }
}

} // end namespace primer
