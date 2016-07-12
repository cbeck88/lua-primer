//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/error_handler_fwd.hpp>
#include <primer/lua.hpp>
#include <primer/push_singleton.hpp>
#include <primer/support/asserts.hpp>

namespace primer {
namespace detail {

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

static constexpr const char * error_handler_reg_key = "primer_error_handler";

} // end namespace detail

inline int get_error_handler(lua_State * L) noexcept {
  lua_getfield(L, LUA_REGISTRYINDEX, detail::error_handler_reg_key);
  if (!lua_toboolean(L, -1)) {
    lua_pop(L, 1);
    primer::push_singleton<detail::fetch_traceback_function>(L);
  }
  return 1;
}

inline void set_error_handler(lua_State * L) noexcept {
  lua_setfield(L, LUA_REGISTRYINDEX, detail::error_handler_reg_key);
}

} // end namespace primer
