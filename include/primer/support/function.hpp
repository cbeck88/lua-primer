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
#include <primer/lua_ref_seq.hpp>
#include <primer/support/error_capture.hpp>
#include <primer/support/push_cached.hpp>
#include <tuple>
#include <utility>

namespace primer {

namespace detail {

inline void fetch_traceback_function(lua_State * L) {
  luaopen_debug(L);
  int result = lua_getfield(L, -1, "traceback");
  PRIMER_ASSERT(result == LUA_TFUNCTION,
                "could not find debug traceback function");
  static_cast<void>(result);
  lua_remove(L, -2);
}

// Expects: Function, followed by narg arguments, on top of the stack.
// Calls pcall with traceback error handler, removes error handler.
// Error is left on top of the stack.
// Returns the error code, and the stack index at which the return values start.
inline std::tuple<int, int> pcall_helper(lua_State * L, int narg, int nret) {
  PRIMER_ASSERT(lua_gettop(L) >= (1 + narg),
                "Not enoguh arguments on stack for pcall!");
  PRIMER_ASSERT(lua_isfunction(L, -1 - narg), "Missing function for pcall!");
  detail::push_cached<fetch_traceback_function>(L);
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
  PRIMER_ASSERT(lua_gettop(L) >= (narg),
                "Not enough arguments on stack for resume!");

  const int result_index = lua_absindex(L, -1 - narg);

  const int result_code = lua_resume(L, nullptr, narg);
  if ((result_code != LUA_OK) && (result_code != LUA_YIELD)) {
    detail::push_cached<fetch_traceback_function>(L);
    lua_insert(L, -2);
    lua_call(L, 1, 1);
  }

  return std::tuple<int, int>{result_code, result_index};
}

// Helper structures to write a generic call function which works regardless of
// number of returns

struct return_none {
  using return_type = expected<void>;
  static return_type pop(lua_State *, int) { return {}; }
  static constexpr int nrets = 0;
};

struct return_one {
  using return_type = expected<lua_ref>;
  static return_type pop(lua_State * L, int) { return lua_ref{L}; }
  static constexpr int nrets = 1;
};

struct return_many {
  using return_type = expected<lua_ref_seq>;
  static return_type pop(lua_State * L, int start_idx) {
    return primer::pop_n(L, lua_gettop(L) - start_idx + 1);
  }
  static constexpr int nrets = LUA_MULTRET;
};

template <typename T>
using result_t = typename T::return_type;

/***
 * Generic scheme for calling a function
 */
template <typename return_pattern>
result_t<return_pattern> fcn_call(lua_State * L, int narg) {
  result_t<return_pattern> result;

  int err_code;
  int results_idx;

  std::tie(err_code, results_idx) =
    detail::pcall_helper(L, narg, return_pattern::nrets);
  if (err_code != LUA_OK) {
    result = detail::pop_error(L, err_code);
  } else {
    result = return_pattern::pop(L, results_idx);
  }

  PRIMER_ASSERT(lua_gettop(L) == (results_idx - 1),
                "hmm stack discipline error");

  return result;
}

/***
 * Generic scheme for resuming a coroutine
 */
template <typename return_pattern>
std::tuple<detail::result_t<return_pattern>, int> resume_call(lua_State * L,
                                                              int narg) {
  result_t<return_pattern> result;

  int err_code;
  int results_idx;

  std::tie(err_code, results_idx) = detail::resume_helper(L, narg);
  if (err_code == LUA_OK) {
    result = return_pattern::pop(L, results_idx);
  } else if (err_code == LUA_YIELD) {
    result = return_pattern::pop(L, results_idx);
  } else {
    result = detail::pop_error(L, err_code);
  }
  lua_settop(L, results_idx - 1);

  return std::tuple<detail::result_t<return_pattern>, int>{std::move(result),
                                                           err_code};
}

} // end namespace detail

// Expects: Function, followed by narg arguments, on top of the stack.
// Returns: Either a reference to the value, or an error message. In either case
// the results are cleared from the stack.
inline expected<lua_ref> fcn_call_one_ret(lua_State * L, int narg) {
  return detail::fcn_call<detail::return_one>(L, narg);
}

// Expects: Function, followed by narg arguments, on top of the stack.
// Returns either a reference to the value, or an error message. In either case
// the results are cleared from the stack.
inline expected<void> fcn_call_no_ret(lua_State * L, int narg) {
  return detail::fcn_call<detail::return_none>(L, narg);
}

// Expects: Function, followed by narg arguments, on top of the stack.
// Returns all of the functions' results or an error message. In either case
// the results are cleared from the stack.
inline expected<lua_ref_seq> fcn_call(lua_State * L, int narg) {
  return detail::fcn_call<detail::return_many>(L, narg);
}


// Expects a thread stack, satisfying the preconditions to call `lua_resume(L,
// nullptr, narg)`.
//   (That means, there should be narg arguments on top of the stack.
//    If coroutine has not been started, the function should be just beneath
//    them.
//    Otherwise, it shouldn't be.)
//
// Calls lua_resume with that many arguments.
// If it returns or yields, the single (expected) return value is popped from
// the stack.
// If there is an error, an error object is returned and the error message is
// popped from the stack, after running an error handler over it.
// The expected<lua_ref> is first return value, the second is the error code, so
// you can tell if yield occurred (LUA_YIELD) or return occurred (LUA_OK).
inline std::tuple<expected<lua_ref>, int> resume_one_ret(lua_State * L,
                                                         int narg) {
  return detail::resume_call<detail::return_one>(L, narg);
}

// Expects a thread stack, satisfying the preconditions to call `lua_resume(L,
// nullptr, narg)`.
//   (That means, there should be narg arguments on top of the stack.
//    If coroutine has not been started, the function should be just beneath
//    them.
//    Otherwise, it shouldn't be.)
//
// Calls lua_resume with that many arguments.
// If it returns or yields, the single (expected) return value is popped from
// the stack.
// If there is an error, an error object is returned and the error message is
// popped from the stack, after running an error handler over it.
// The expected<void> is (potentially) the error message, the second is the
// error code, so you can tell if yield occurred (LUA_YIELD) or return occurred
// (LUA_OK).
inline std::tuple<expected<void>, int> resume_no_ret(lua_State * L, int narg) {
  return detail::resume_call<detail::return_none>(L, narg);
}

// Expects a thread stack, satisfying the preconditions to call `lua_resume(L,
// nullptr, narg)`.
//   (That means, there should be narg arguments on top of the stack.
//    If coroutine has not been started, the function should be just beneath
//    them.
//    Otherwise, it shouldn't be.)
//
// Calls lua_resume with that many arguments.
// If it returns or yields, all of its return values are returned.
// If there is an error, an error object is returned and the error message is
// popped from the stack,
// after running an error handler over it.
// The expected<lua_ref_seq> is return sequence, the second is the error code,
// so you can tell if yield occurred (LUA_YIELD) or return occurred (LUA_OK).
inline std::tuple<expected<lua_ref_seq>, int> resume(lua_State * L, int narg) {
  return detail::resume_call<detail::return_many>(L, narg);
}

} // end namespace primer
