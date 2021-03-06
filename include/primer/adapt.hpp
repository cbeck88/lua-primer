//  (C) Copyright 2015 - 2018 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * PRIMER_ADAPT is a collection of templates which allows one to
 * write lua callback functions which take common C++ types.
 * The dispatch mechanism adapts such functions to lua_CFunctions,
 * and handles the process of parsing arguments off of the stack
 * and reporting appropriate error messages.
 *
 * If your method has a problem, you signal an error by returning Error,
 * not by calling lua_error yourself. PRIMER_ADAPT will signal the lua
 * error after all the dtors have been called.
 *
 * It is intended to permit you to write lua callbacks without
 * typically having to think about longjmp issues.
 *
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/expected.hpp>
#include <primer/lua.hpp>
#include <primer/read.hpp>
#include <primer/result.hpp>

#include <primer/detail/count.hpp>
#include <primer/detail/max_int.hpp>
#include <primer/support/implement_result.hpp>

#include <type_traits>
#include <utility>

namespace primer {

//[ primer_adapt_decl
// adapt is what does the actual work.
// The PRIMER_ADAPT macro provides a suitable interface to it
template <typename T, T>
class adapt;
//]

//[ primer_adapt
// Simplified interface to the helper structure template
// (This seems to be necessary since we can't deduce non-type template parameter
// types, at least prior to C++17.)
//
#define PRIMER_ADAPT(F) &::primer::adapt<decltype(F), (F)>::adapted
//]

//[ primer_adapt_trivial
// Traditional "raw" C-style lua callbacks.
// We don't have to do any work
template <lua_CFunction target_func>
class adapt<lua_CFunction, target_func> {
public:
  static int adapted(lua_State * L) { return target_func(L); }
};
//]

/***
 * Implementation for a "free" function, using `primer::result` return type.
 */
template <typename... Args,
          primer::result (*target_func)(lua_State * L, Args...)>
class adapt<primer::result (*)(lua_State * L, Args...), target_func> {

  /***
   * We introduce a local class to do the actual dispatching.
   * This is because we want to have the "indices" parameter pack
   * corresponding to count_t available to us in the function body.
   */
  template <typename T>
  struct impl;

  template <std::size_t... indices>
  struct impl<primer::detail::SizeList<indices...>> {
    // We also mark this as "noexcept" just in case, to prevent bad exceptions
    // from propagating to lua.

    static primer::result adapted(lua_State * L) noexcept {
      // Create a flag that all the readers can use in order to signal an error
      expected<void> ok;
      // indices + 1 is because lua counts from 1
      return call_helper(L, ok, read_helper<Args>(L, indices + 1, ok)...);
    }

    // When we don't use exceptions, we can't use "unwrap".
    // This version simulates manually the short-circuiting logic.
    template <typename T>
    static expected<T> read_helper(lua_State * L, int index,
                                   expected<void> & ok) {
      expected<T> result{primer::error{}};
      // Empty error is okay, this only is returned if we are short circuiting
      // This allows NRVO to take place.

      // short-circuit if we would have thrown an exception by now
      if (ok) {
        result = primer::read<T>(L, index);
        // move any errors onto the "global" channel
        if (!result) { ok = std::move(result.err()); }
      }

      return result;
    }

    // Before calling the target_func, we need to check the "global" channel
    // for an error. Then, unpack all the `expected` into the function call.
    static primer::result call_helper(lua_State * L, expected<void> & ok,
                                      expected<Args>... args) {
      if (!ok) { return std::move(ok.err()); }
      return target_func(L, (*std::move(args))...);
      // Note: * std::move(...) rather than std::move(* ...)` is important, that
      // allows it to work with expected<T&>
    }
  };

public:
  static int adapted(lua_State * L) {
    // Estimate how much stack space we will need to read the arguments.
    // If we don't have enough, then signal an error
    // We are guaranteed at least LUA_MINSTACK by the implementation whenever
    // this function is called by lua.
    constexpr int estimate =
      detail::max_int(0, stack_space_for_read<Args>()...);
    if (estimate > LUA_MINSTACK) {
      if (!lua_checkstack(L, estimate)) {
        return luaL_error(L, "not enough stack space, needed %d", estimate);
      }
    }

    using I = detail::Count_t<sizeof...(Args)>;

    auto temp = detail::implement_result_step_one(L, impl<I>::adapted(L));
    // primer::result is destroyed at the end of "full expression" in the above
    // line, so it is safe to longjmp after this.
    return detail::implement_result_step_two(L, temp);
  }
};

} // end namespace primer
