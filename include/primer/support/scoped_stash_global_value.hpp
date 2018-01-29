//  (C) Copyright 2015 - 2018 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * Support object which stashes a global value in the registry for its lifetime,
 * and restores it in the dtor.
 *
 * Use with care.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/lua.hpp>

namespace primer {
namespace detail {

class scoped_stash_global_value {
  lua_State * L_;
  const char * key_;

public:
  explicit scoped_stash_global_value(lua_State * L, const char * key)
    : L_(L)
    , key_(key) {
    lua_pushlightuserdata(L_, static_cast<void *>(this));
    lua_getglobal(L_, key_);
    lua_settable(L_, LUA_REGISTRYINDEX);
  }

  ~scoped_stash_global_value() {
    lua_pushlightuserdata(L_, static_cast<void *>(this));
    lua_gettable(L_, LUA_REGISTRYINDEX);
    lua_setglobal(L_, key_);
  }
};

} // end namespace detail
} // end namespace primer
