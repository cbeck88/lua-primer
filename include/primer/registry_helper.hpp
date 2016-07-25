//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * Support object which stashes a pointer to an instance of some type at a
 * unique location in the registry, and allows to recover later.
 *
 * Use with care
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/lua.hpp>
#include <primer/support/asserts.hpp>

namespace primer {

template <typename T>
class registry_helper {

  static int get_key(lua_State * L) {
    lua_pushcfunction(L, &get_key);
    return 0;
  }

public:
  static void store_self(lua_State * L, T * t) {
    get_key(L);
    lua_pushlightuserdata(L, static_cast<void *>(t));
    lua_settable(L, LUA_REGISTRYINDEX);
  }

  static T * obtain_self(lua_State * L) {
    get_key(L);
    lua_gettable(L, LUA_REGISTRYINDEX);
    void * ptr = lua_touserdata(L, -1);
    lua_pop(L, 1);
    return static_cast<T *>(ptr);
  }
};

} // end namespace primer
