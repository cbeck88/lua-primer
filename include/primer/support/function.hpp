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
#include <primer/error_capture.hpp>
#include <primer/error_handler.hpp>
#include <primer/expected.hpp>
#include <primer/lua.hpp>
#include <primer/support/function_return_fwd.hpp>
#include <tuple>
#include <utility>

namespace primer {

namespace detail {

// Expects: Function, followed by narg arguments, on top of the stack.
// Calls pcall with traceback error handler, removes error handler.
// Error is left on top of the stack.
// Returns the error code, and the stack index at which the return values start.
inline std::tuple<int, int>
pcall_helper(lua_State * L, int narg, int nret) noexcept {
  PRIMER_ASSERT(lua_gettop(L) >= (1 + narg),
                "Not enough arguments on stack for pcall!");
  PRIMER_ASSERT(lua_isfunction(L, -1 - narg), "Missing function for pcall!");
  primer::get_error_handler(L);
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
inline std::tuple<int, int>
resume_helper(lua_State * L, int narg) noexcept {
  PRIMER_ASSERT(lua_gettop(L) >= (narg),
                "Not enough arguments on stack for resume!");

  const int result_index = lua_absindex(L, -1 - narg);

  const int result_code = lua_resume(L, nullptr, narg);
  if ((result_code != LUA_OK) && (result_code != LUA_YIELD)) {
    primer::get_error_handler(L);
    lua_insert(L, -2);
    lua_call(L, 1, 1);
  }

  return std::tuple<int, int>{result_code, result_index};
}

/***
 * Generic scheme for calling a function
 * Note: NOT noexcept, it can `primer::pop_n` can cause lua memory allocation
 * failure.
 * It is `noexcept` in the no return values case.
 *
 * Note: This function should not have nontrivial objects on the stack.
 */
template <typename T>
void
fcn_call(expected<T> & result, lua_State * L, int narg) {
  int err_code;
  int results_idx;

  std::tie(err_code, results_idx) =
    detail::pcall_helper(L, narg, return_helper<T>::nrets);
  if (err_code != LUA_OK) {
    result = primer::pop_error(L, err_code);
  } else {
    return_helper<T>::pop(L, results_idx, result);
  }

  PRIMER_ASSERT(lua_gettop(L) == (results_idx - 1),
                "hmm stack discipline error");
}

/***
 * Generic scheme for resuming a coroutine
 */
template <typename T>
void
resume_call(expected<T> & result, lua_State * L, int narg) {
  int err_code;
  int results_idx;

  std::tie(err_code, results_idx) = detail::resume_helper(L, narg);
  if (err_code == LUA_OK || err_code == LUA_YIELD) {
    return_helper<T>::pop(L, results_idx, result);
  } else {
    result = primer::pop_error(L, err_code);
  }
  lua_settop(L, results_idx - 1);
}

} // end namespace detail

} // end namespace primer
