//  (C) Copyright 2015 - 2016 Christopher Beck

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
 * E.G.
 *
 * callback_result my_callback(lua_State *, int i, unsigned int u,
 *                               std::string s, util::maybe<float> f);
 *
 * can be adapted via `PRIMER_ADAPT(&my_callback)` to a
 * proper lua_CFunction and everything just works (TM).
 *
 * primer::adapt uses traits to figure out exactly how each arg
 * is parsed off of the stack, that's where the "common sense" comes from.
 *
 * primer::adapt's implementation handles the issues surrounding lua longjmp
 * while there are nontrivial C++ objects on the stack. If one of the
 * arguments cannot be read properly, all previous arguments are
 * destroyed before we longjmp.
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
#include <primer/read.hpp>
#include <primer/result.hpp>

#include <primer/support/count.hpp>
#include <primer/support/implement_result.hpp>

#ifndef PRIMER_NO_EXCEPTIONS
#include <primer/support/unwrap.hpp>
#endif

#include <tuple>
#include <type_traits>
#include <utility>

namespace primer {

/***
 * adaptor is what does the actual work.
 * The PRIMER_ADAPT macro provides a suitable interface to it
 */

template <typename T, T>
class adaptor;

/***
 * Simplified interface to the helper structure template
 * (This seems to be necessary since we can't deduce non-type template parameter types.)
 */
#define PRIMER_ADAPT(F) &::primer::adaptor<decltype(F), (F)>::adapted

/***
 * Traditional "raw" C-style lua callbacks. They aren't even member functions. We don't have to do any work.
 */

template <lua_CFunction target_func>
class adaptor<lua_CFunction, target_func> {
public:
  static int adapted(lua_State * L) { return target_func(L); }
};


/***
 * Now, implementation for a "free" function,
 * member function comes next.
 */

template <typename... Args, primer::result (*target_func)(lua_State * L, Args...)>
class adaptor<primer::result (*)(lua_State * L, Args...), target_func> {

  /***
   * We introduce a local class to do the actual dispatching.
   * This is because we want to have the "indices" parameter pack
   * corresponding to count_t available to us in the function body.
   */
  template <typename T>
  struct impl;

  template <std::size_t... indices>
  struct impl<primer::SizeList<indices...>> {
    // We also mark this as "noexcept" in order to prevent bad exceptions from propagating to lua.

#ifndef PRIMER_NO_EXCEPTIONS

    // There is some overhead for exception support, but this version makes basically no copies
    static primer::result helper(lua_State * L) noexcept {
      try {
        return target_func(L, primer::unwrap(primer::read<Args>(L, indices + 1))...);
      } catch (primer::error & e) {
        return std::move(e);
      }
    }

#else // PRIMER_NO_EXCEPTIONS

    template <typename T>
    static expected<T> short_circuiting_reader(lua_State * L, int index, expected<void> & ok) {
      // short circuit if we would have thrown an exception by now
      if (!ok) { return primer::error{}; }
      expected<T> result{primer::read<T>(L, index)};
      if (!result) {
        ok = result.err();
        PRIMER_ASSERT(!ok, "Bad copy assign");
      }
      return result;
    }

    static primer::result helper(lua_State * L) noexcept {
      using tuple_t = std::tuple<expected<Args>...>;

      expected<void> ok;
      // indices + 1 is because lua counts from 1
      tuple_t tup{short_circuiting_reader<Args>(L, indices + 1, ok)...};
      if (!ok) {
        return ok.err();
      }
      return target_func(L, *std::move(std::get<indices>(tup))...);
    }
    // Note: * std::move(...) rather than std::move(* ...)` is important, that allows it to work with expected<T&>

#endif // PRIMER_NO_EXCEPTIONS

  };

public:
  static int adapted(lua_State * L) {
    auto temp = detail::implement_result_step_one(L, impl<primer::Count_t<sizeof...(Args)>>::helper(L));
    // primer::result is destroyed at the end of "full expression" in the above line, so it is safe to
    // longjmp after this.
    return detail::implement_result_step_two(L, temp);
  }
};

} // end namespace primer
