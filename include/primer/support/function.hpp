//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * Support code related to calling functions, capturing their return values,
 * reporting errors. Mostly this means "safe" or "friendly" wrappers over
 * lua_pcall and lua_resume.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/error.hpp>
#include <primer/expected.hpp>
#include <primer/lua.hpp>
#include <primer/lua_ref.hpp>
#include <tuple>
#include <utility>

namespace primer {

namespace detail {

inline void fetch_traceback_function(lua_State * L) {
  luaopen_debug(L);
  int result = lua_getfield(L, -1, "traceback");
  PRIMER_ASSERT(result == LUA_TFUNCTION, "could not find debug traceback function");
  static_cast<void>(result);
  lua_remove(L, -2);
}

inline void get_traceback_function_cached(lua_State * L) {
  constexpr const char * const traceback_key = "primer_debug_traceback";
  if (LUA_TFUNCTION != lua_getfield(L, LUA_REGISTRYINDEX, traceback_key)) {
    lua_pop(L, 1);
    fetch_traceback_function(L);
    lua_pushvalue(L, -1);
    lua_setfield(L, LUA_REGISTRYINDEX, traceback_key);
  }
}

inline const char * error_code_to_string(const int err_code) {
  switch (err_code) {
    case LUA_ERRSYNTAX:
      return "a syntax error:";
    case LUA_ERRRUN:
      return "a runtime error:";
    case LUA_ERRMEM:
      return "a memory allocation error:";
    case LUA_ERRERR:
      return "an error in the error handler function:";
    case LUA_OK:
      return "this error code means there was no error... please report this:";
    default:
      return "an unknown type of error:";
  }
}

// Expects: Function, followed by narg arguments, on top of the stack.
// Calls pcall with traceback error handler, removes error handler.
// Error is left on top of the stack.
// Returns the error code, and the stack index at which the return values start.
inline std::tuple<int, int> pcall_helper(lua_State * L, int narg, int nret) {
  PRIMER_ASSERT(lua_gettop(L) >= (1 + narg), "Not enoguh arguments on stack for pcall!");
  PRIMER_ASSERT(lua_isfunction(L, -1 - narg), "Missing function for pcall!");
  get_traceback_function_cached(L);
  lua_insert(L, -2 - narg);
  const int error_handler_index = lua_absindex(L, -2 - narg);
  const int result_code = lua_pcall(L, narg, nret, error_handler_index);
  lua_remove(L, error_handler_index);

  return std::tuple<int, int>{result_code, error_handler_index};
}

// Expects: Function, followed by narg arguments, on top of the stack.
// Calls lua_resume. If an error occurs, calls the traceback error handler,
// removes error handler. Error is left on top of the stack.
// Returns the error code, and the stack index at which the return values start.
inline std::tuple<int, int> resume_helper(lua_State * L, int narg) {
  PRIMER_ASSERT(lua_gettop(L) >= (narg), "Not enough arguments on stack for resume!");

  const int result_index = lua_absindex(L, -1 - narg);

  const int result_code = lua_resume(L, nullptr, narg);
  if ((result_code != LUA_OK) && (result_code != LUA_YIELD)) {
    get_traceback_function_cached(L);
    lua_insert(L, -2);
    lua_call(L, 1, 1);
  }

  return std::tuple<int, int>{result_code, result_index};
}

// Gets an error string from the top of the stack, forms a primer::error from it.
// Pops the error string.
inline primer::error pop_error(lua_State * L, int err_code) {
  PRIMER_ASSERT(lua_gettop(L), "No error object to pop!");
  primer::error e{lua_isstring(L, -1) ? lua_tostring(L, -1) : "(no description available)"};
  e.prepend_error_line(detail::error_code_to_string(err_code));
  lua_pop(L, 1);
  return e;
}

} // end namespace detail

// Expects: Function, followed by narg arguments, on top of the stack.
// Returns either a reference to the value, or an error message. In either case the
// results are cleared from the stack.
inline expected<lua_ref> fcn_call_one_ret(lua_State * L, int narg) {

  expected<lua_ref> result;

  int err_code;

  std::tie(err_code, std::ignore) = detail::pcall_helper(L, narg, 1);
  if (err_code != LUA_OK) {
    result = detail::pop_error(L, err_code);
  } else {
    result = lua_ref{L};
  }

  return result;
}

// Expects: Function, followed by narg arguments, on top of the stack.
// Returns either a reference to the value, or an error message. In either case the
// results are cleared from the stack.
inline expected<void> fcn_call_no_ret(lua_State * L, int narg) {

  expected<void> result;

  int err_code;

  std::tie(err_code, std::ignore) = detail::pcall_helper(L, narg, 0);
  if (err_code != LUA_OK) {
    result = detail::pop_error(L, err_code);
  } else {
    result = {};
  }

  return result;
}

// Expects a thread stack, satisfying the preconditions to call `lua_resume(L, nullptr, narg)`.
//   (That means, there should be narg arguments on top of the stack.
//    If coroutine has not been started, the function should be just beneath them.
//    Otherwise, it shouldn't be.)
//
// Calls lua_resume with that many arguments.
// If it returns or yields, the single (expected) return value is popped from the stack.
// If there is an error, an error object is returned and the error message is popped from the stack,
// after running an error handler over it.
// The expected<lua_ref> is first return value, the second is the error code, so you can tell if
// yield occurred (LUA_YIELD) or return occurred (LUA_OK).
inline std::tuple<expected<lua_ref>, int> resume_one_ret(lua_State * L, int narg) {
  expected<lua_ref> result;

  int err_code;
  int results_idx;

  std::tie(err_code, results_idx) = detail::resume_helper(L, narg);
  if (err_code == LUA_OK) {
    result = lua_ref{L};
  } else if (err_code == LUA_YIELD) {
    result = lua_ref{L};
  } else {
    result = detail::pop_error(L, err_code);
  }
  lua_settop(L, results_idx - 1);

  return std::tuple<expected<lua_ref>, int>{ std::move(result), err_code };
}

// Expects a thread stack, satisfying the preconditions to call `lua_resume(L, nullptr, narg)`.
//   (That means, there should be narg arguments on top of the stack.
//    If coroutine has not been started, the function should be just beneath them.
//    Otherwise, it shouldn't be.)
//
// Calls lua_resume with that many arguments.
// If it returns or yields, the single (expected) return value is popped from the stack.
// If there is an error, an error object is returned and the error message is popped from the stack,
// after running an error handler over it.
// The expected<lua_ref> is first return value, the second is the error code, so you can tell if
// yield occurred (LUA_YIELD) or return occurred (LUA_OK).
inline std::tuple<expected<lua_ref>, int> resume_no_ret(lua_State * L, int narg) {
  expected<lua_ref> result;

  int err_code;
  int results_idx;

  std::tie(err_code, results_idx) = detail::resume_helper(L, narg);
  if (err_code == LUA_OK) {
    result = {};
  } else if (err_code == LUA_YIELD) {
    result = {};
  } else {
    result = detail::pop_error(L, err_code);
  }
  lua_settop(L, results_idx - 1);

  return std::tuple<expected<lua_ref>, int>{ std::move(result), err_code };
}

} // end namespace primer
