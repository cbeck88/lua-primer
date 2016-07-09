//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/lua.hpp>
#include <primer/support/asserts.hpp>
#include <primer/support/push_cached.hpp>

namespace primer {

// TODO: Would be nice to push our own C traceback function which can be
// customized and technically cannot cause lua error when we luaopen_debug.
inline void fetch_traceback_function(lua_State * L) noexcept {
  luaopen_debug(L);
  int result = lua_getfield(L, -1, "traceback");
  PRIMER_ASSERT(result == LUA_TFUNCTION,
                "could not find debug traceback function");
  static_cast<void>(result);
  lua_remove(L, -2);
}

} // end namespace primer
