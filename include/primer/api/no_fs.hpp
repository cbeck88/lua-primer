//  (C) Copyright 2015 - 2018 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/lua.hpp>

namespace primer {
namespace api {

// A miniature version of "require" for sandboxed systems that want to use
// require.
// Based on `ll_require` from the package lib "loadlib.c".
//
//
// It only searches `_LOADED` table, and doesn't use FS search or any
// loaders.

inline int
mini_require(lua_State * L) {
  const char * name = luaL_checkstring(L, 1);
  lua_settop(L, 1); /* _LOADED table will be at index 2 */
  lua_getfield(L, LUA_REGISTRYINDEX, "_LOADED");
  lua_getfield(L, 2, name); /* _LOADED[name] */
  if (lua_toboolean(L, -1)) { return 1; }
  lua_pop(L, 1);
  lua_pushboolean(L, true);
  return 1;
}

// A mini library which installs the above require function at global scope.
// This can be useful in concert with the sandboxed versions of libs.

struct no_fs {

  static constexpr const char * func_name = "require";
  static constexpr const char * persist_name = "no_fs_lib_require";
  static constexpr lua_CFunction func = &mini_require;

  // API Feature
  void on_init(lua_State * L) {
    lua_pushcfunction(L, func);
    lua_setglobal(L, func_name);
  }

  void on_persist_table(lua_State * L) {
    lua_pushstring(L, persist_name);
    lua_pushcfunction(L, func);
    lua_settable(L, -3);
  }

  void on_unpersist_table(lua_State * L) {
    lua_pushcfunction(L, func);
    lua_setfield(L, -2, persist_name);
  }
};

} // end namespace api
} // end namespace primer
