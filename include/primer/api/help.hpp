//  (C) Copyright 2015 - 2017 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * Defines a simple registry-based help interface.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/lua.hpp>
#include <primer/push_singleton.hpp>

namespace primer {

namespace api {

// The help database table does not need to be anything special.
inline void
make_help_table(lua_State * L) {
  lua_newtable(L);
}

/***
 * set_help_string: Registers a help string for a function in the help database
 * for this lua state.
 */
inline void
set_help_string(lua_State * L, lua_CFunction f, const char * help_str) {
  PRIMER_ASSERT_STACK_NEUTRAL(L);

  primer::push_singleton<&make_help_table>(L);
  lua_pushcfunction(L, f);
  if (help_str) {
    lua_pushstring(L, help_str);
  } else {
    lua_pushnil(L);
  }
  lua_settable(L, -3);
  lua_pop(L, 1);
}

/***
 * get_help_string: Try to obtain a help string from the database.
 * return nullptr if lookup fails.
 *
 * The string is owned by lua and is linked to from the help database, so its
 * lifetime is at least until set_help_string is called to change this string,
 * or the lua_State is collected.
 */

inline const char *
get_help_string(lua_State * L, lua_CFunction f) {
  PRIMER_ASSERT_STACK_NEUTRAL(L);

  primer::push_singleton<&make_help_table>(L);
  lua_pushcfunction(L, f);
  lua_gettable(L, -2);
  const char * str = lua_tostring(L, -1);
  lua_pop(L, 2);
  // Note: The string is still in the registry table, so it will not be
  // garbage collected by lua until that link is broken.
  return str;
}

/***
 * intf_help_impl: Respond to a lua query for help with a function.
 *                 A generic implementation using get_help_string.
 */

inline int
intf_help_impl(lua_State * L) {
  if (lua_iscfunction(L, 1)) {
    if (const char * result = get_help_string(L, lua_tocfunction(L, 1))) {
      lua_pushstring(L, result);
    } else {
      lua_pushliteral(L, "No help entry was found.");
    }
    return 1;
  } else if (lua_isfunction(L, 1)) {
    lua_pushliteral(L,
                    "Expected a built-in function: This is a user-defined "
                    "function.");
  } else {
    lua_pushliteral(L, "Expected a function");
  }
  return lua_error(L);
}

} // end namespace api

} // end namespace primer
