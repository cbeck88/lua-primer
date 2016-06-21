//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * Useful function that registers a null-terminated array of luaL_Reg-like
 * objects with a table on top of the stack.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/lua.hpp>
#include <primer/support/asserts.hpp>

namespace primer {

template <typename T>
void set_funcs(lua_State * L, const T * ptr) {

#ifdef PRIMER_DEBUG
  {
    auto t = lua_type(L, -1);
    PRIMER_ASSERT(t == LUA_TTABLE || t == LUA_TUSERDATA || t == LUA_TLIGHTUSERDATA, "In set_funcs, no table or table-like thing was found!");
  }
#endif // PRIMER_DEBUG

  for (; ptr->name; ++ptr) {
    lua_pushcfunction(L, ptr->func);
    lua_setfield(L, -2, ptr->name);
  }
}

} // end namespace primer
