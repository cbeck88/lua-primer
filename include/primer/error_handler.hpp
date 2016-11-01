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
inline void
fetch_traceback_function(lua_State * L) noexcept {
  luaopen_debug(L);
  int result = lua_getfield(L, -1, "traceback");
  PRIMER_ASSERT(result == LUA_TFUNCTION,
                "could not find debug traceback function");
  static_cast<void>(result);
  lua_remove(L, -2);
}

static constexpr const char * error_handler_reg_key = "primer_error_handler";

} // end namespace detail

//[ primer_error_handler_decl

// Push the current error handler to top of stack. Default is debug.traceback.
inline int get_error_handler(lua_State * L) noexcept;

// Set a new error handler. Pops one object from top of stack.
inline void set_error_handler(lua_State * L) noexcept;
//]

//[ primer_protected_call_decl
// Simplified version of lua_pcall which handles setting up the error handler,
// and removing it after the pcall returns.
// Otherwise it is the same as pcall -- leaves any results / errors on top
// of the stack.
inline int protected_call(lua_State * L, int narg, int nret) noexcept;

//]

inline int
get_error_handler(lua_State * L) noexcept {
  lua_getfield(L, LUA_REGISTRYINDEX, detail::error_handler_reg_key);
  if (!lua_toboolean(L, -1)) {
    lua_pop(L, 1);
    primer::push_singleton<detail::fetch_traceback_function>(L);
  }
  return 1;
}

inline void
set_error_handler(lua_State * L) noexcept {
  lua_setfield(L, LUA_REGISTRYINDEX, detail::error_handler_reg_key);
}

//[ primer_protected_call_defn
inline int
protected_call(lua_State * L, int narg, int nret) noexcept {
  primer::get_error_handler(L);
  lua_insert(L, -2 - narg);
  int eidx = lua_absindex(L, -2 - narg);

  int result = lua_pcall(L, narg, nret, eidx);

  lua_remove(L, eidx);
  return result;
}
//]

} // end namespace primer
