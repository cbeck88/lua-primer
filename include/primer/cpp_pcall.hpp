//  (C) Copyright 2015 - 2018 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * cpp_pcall is a template which implements the `cpcall` idiom in a generic
 * fashion.
 *
 * Given any callbable C++ object which takes a lua_State *, whose operation
 * may generate lua errors, (but not foreign exceptions), cpp_pcall invokes it
 * from a protected context.
 *
 * The callable should have return type void -- any return value is discarded.
 * The callable must not throw exceptions, but it may raise any lua errors.
 *
 * The cpp_pcall itself returns `expected<void>`. It does not throw exceptions
 * or generate lua errors.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/detail/count.hpp>
#include <primer/detail/type_traits.hpp>
#include <primer/expected.hpp>
#include <primer/lua.hpp>
#include <primer/support/function.hpp>

#include <tuple>
#include <type_traits>
#include <utility>

//[ cpp_pcall_synopsis
namespace primer {

/*<< Takes a callable and some arguments, performs the call in a protected
context which captures lua errors.

Usually, `f` needs a `lua State *` also as one of the arguments, to do whatever
it will do. It might or might not be the same `lua_State *`, it could be
operating on a thread.

The first parameter to `cpp_pcall` should be the main
thread, and it should be related to whatever threads `f` is operating on, or the
error capture won't work.

The optional template parameter `narg` signals how many values on the top of L's
stack should be consumed as arguments to the `pcall`. By default, none are.

The callable itself must not throw a C++ exception, or terminate will be called.
>>*/
template <int narg = 0, typename F, typename... Args>
expected<void> cpp_pcall(lua_State * L, F && f, Args &&... args) noexcept;
}
//]

namespace primer {

//[ cpp_pcall_implementation

namespace detail {

template <typename T>
int
lambda_upvalue_dispatch(lua_State * L) {
  T * t = static_cast<T *>(lua_touserdata(L, lua_upvalueindex(1)));
  (*t)();
  return lua_gettop(L);
}

} // end namespace detail

template <int narg, typename F, typename... Args>
expected<void>
cpp_pcall(lua_State * L, F && f, Args &&... args) noexcept {
  expected<void> result;

  auto lambda = [&]() { (std::forward<F>(f))(std::forward<Args>(args)...); };
  lua_pushlightuserdata(L, static_cast<void *>(&lambda));
  lua_pushcclosure(L, &detail::lambda_upvalue_dispatch<decltype(lambda)>, 1);

  if (narg) { lua_insert(L, -1 - narg); }

  int code; // pcall_helper installs a custom error handler
  std::tie(code, std::ignore) = detail::pcall_helper(L, narg, LUA_MULTRET);

  if (code != LUA_OK) { result = pop_error(L, code); }
  return result;
}
//]

//[ mem_pcall_implementation
//` Sometimes, we perform operations which can raise lua errors, but only of the
//` memory variety. In this case, we call `mem_pcall` instead of `cpp_pcall`,
//` which is resolves to either a `cpp_pcall` or a simple invocation of the
//` callable, depending on the `PRIMER_NO_MEMORY_FAILURE` define.
//`
//` This, together with `PRIMER_TRY_BAD_ALLOC` / `PRIMER_CATCH_BAD_ALLOC`, is
//` the total effect of the `PRIMER_NO_MEMORY_FAILURE` switch.
//`
//` mem_pcall should not be used with calls that can throw an exception. It
//` also should not be used with calls that can raise lua errors of the
//` non-memory variety, since when `PRIMER_NO_MEMORY_FAILURE` is defined the
//` protection is stripped out. If you need full protection then use `cpp_pcall`
//` directly.

template <int narg = 0, typename F, typename... Args>
expected<void>
mem_pcall(lua_State * L, F && f, Args &&... args) noexcept {
#ifdef PRIMER_NO_MEMORY_FAILURE
  static_cast<void>(L);
  std::forward<F>(f)(std::forward<Args>(args)...);
  return {};
#else
  return cpp_pcall<narg>(L, std::forward<F>(f), std::forward<Args>(args)...);
#endif
}

//]

} // end namespace primer
