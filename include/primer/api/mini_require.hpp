//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/lua.hpp>

// A miniature version of "require" for sandboxed systems that want to use require.
// Based on `ll_require` from the package lib "loadlib.c".
//
// 
// It only searches `_LOADED` table, and doesn't use FS search or any
// loaders.

namespace primer {
namespace api {

inline int mini_require(lua_State * L) {
  const char *name = luaL_checkstring(L, 1);
  lua_settop(L, 1);  /* _LOADED table will be at index 2 */
  lua_getfield(L, LUA_REGISTRYINDEX, "_LOADED");
  lua_getfield(L, 2, name);  /* _LOADED[name] */
  if (lua_toboolean(L, -1)) {
    return 1;
  }
  lua_pop(L, 1);
  lua_pushboolean(L, true);
  return 1;
}

} // end namespace api
} // end namespace primer
