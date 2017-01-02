//  (C) Copyright 2015 - 2017 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * A tiny helper function that gets the pointer to the "main thread" from a
 * given thread pointer, using the registry.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/lua.hpp>

namespace primer {

inline lua_State *
main_thread(lua_State * L) {
  lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_MAINTHREAD);
  lua_State * M = lua_tothread(L, -1);
  lua_pop(L, 1);
  return M;
}

} // end namespace primer
