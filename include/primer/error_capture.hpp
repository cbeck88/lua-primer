//  (C) Copyright 2015 - 2018 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * Support code related to interpretting lua runtime errors
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/error.hpp>
#include <primer/lua.hpp>
#include <primer/support/asserts.hpp>
#include <primer/support/diagnostics.hpp>

namespace primer {
namespace detail {

inline const char *
error_code_to_string(const int err_code) noexcept {
  switch (err_code) {
    case LUA_ERRSYNTAX:
      return "a syntax error:";
    case LUA_ERRRUN:
      return "a runtime error:";
    case LUA_ERRMEM:
      return "a memory allocation error:";
    case LUA_ERRERR:
      return "an error in the error handler function:";
    case LUA_ERRGCMM:
      return "an error in a __gc metamethod";
    case LUA_OK:
      return "this error code means there was no error... please report this:";
    default:
      return "an unknown type of error:";
  }
}

} // end namespace detail

// Gets an error string from the top of the stack, forms a primer::error.
// Pops the error string.
// Note: This is noexcept because primer::error consumes any bad_alloc within
// ctor and sets itself to bad_alloc state.
inline primer::error
pop_error(lua_State * L, int err_code) noexcept {
  PRIMER_ASSERT(lua_gettop(L), "No error object to pop!");
  primer::error e{lua_isstring(L, -1) ? lua_tostring(L, -1)
                                      : "(no description available)"};
  e.prepend_error_line(detail::error_code_to_string(err_code));
  lua_pop(L, 1);
  return e;
}

// Pushes an error onto the stack.
// Use this rather than pushing `e.what` directly in case we change the
// interface or something.
inline void
push_error(lua_State * L, const primer::error & e) noexcept {
  lua_pushstring(L, e.what());
}

// Create an "unexpected value" error
inline primer::error
arg_error(lua_State * L, int index, const char * expected) noexcept {
  return primer::error::unexpected_value(expected,
                                         primer::describe_lua_value(L, index));
}

} // end namespace primer
