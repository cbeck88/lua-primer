//  (C) Copyright 2015 - 2017 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * Interface to write quick-n-dirty function call code.
 *
 * While they all handle popping of errors and return values,
 * they don't handle memory errors that can occur while popping the return
 * values. So they aren't all that safe.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/support/function.hpp>
#include <primer/support/function_return.hpp>

namespace primer {

// Expects: Function, followed by narg arguments, on top of the stack.
// Returns: Either a reference to the value, or an error message. In either case
// the results are cleared from the stack.
inline expected<lua_ref>
fcn_call_one_ret(lua_State * L, int narg) {
  expected<lua_ref> result;
  detail::fcn_call(result, L, narg);
  return result;
}

// Expects: Function, followed by narg arguments, on top of the stack.
// Returns either a reference to the value, or an error message. In either case
// the results are cleared from the stack.
// This one is noexcept because it does not create any lua_ref's.
inline expected<void>
fcn_call_no_ret(lua_State * L, int narg) noexcept {
  expected<void> result;
  detail::fcn_call(result, L, narg);
  return result;
}

// Expects: Function, followed by narg arguments, on top of the stack.
// Returns all of the functions' results or an error message. In either case
// the results are cleared from the stack.
inline expected<lua_ref_seq>
fcn_call(lua_State * L, int narg) {
  expected<lua_ref_seq> result;
  detail::fcn_call(result, L, narg);
  return result;
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
// The expected<lua_ref> is first return value.
// Use `lua_status` to figure out if it was return or yield.
inline expected<lua_ref>
resume_one_ret(lua_State * L, int narg) {
  expected<lua_ref> result;
  detail::resume_call(result, L, narg);
  return result;
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
// The expected<void> is (potentially) the error message.
// Use `lua_status` to figure out if it was return or yield.
// This one is noexcept because it does not create any lua_ref's.
inline expected<void>
resume_no_ret(lua_State * L, int narg) noexcept {
  expected<void> result;
  detail::resume_call(result, L, narg);
  return result;
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
// The expected<lua_ref_seq> is return sequence.
// Use `lua_status` to figure out if it was return or yield.
inline expected<lua_ref_seq>
resume(lua_State * L, int narg) {
  expected<lua_ref_seq> result;
  detail::resume_call(result, L, narg);
  return result;
}

} // end namespace primer
