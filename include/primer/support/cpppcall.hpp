//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * cpppcall is a template which implements the `cpcall` idiom in a generic
 * fashion.
 *
 * Given any callbable C++ object which takes a lua_State *, whose operation
 * may generate lua errors, (but not foreign exceptions), cpppcall invokes it
 * from a protected context.
 *
 * The result is a function returning an `expected` type, which does not throw
 * exceptions or generate lua errors.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/expected.hpp>
#include <primer/lua.hpp>
#include <primer/detail/count.hpp>
#include <primer/detail/type_traits.hpp>
#include <primer/support/function.hpp>

#include <tuple>
#include <type_traits>
#include <utility>

namespace primer {

template <typename F, typename... Args>
struct protected_call_helper {
  using tuple_t = std::tuple<F, Args...>;

  template <typename T>
  struct impl;

  template <std::size_t... Is>
  struct impl<detail::SizeList<Is...>> {
    static int cfunc(lua_State * L) {
      tuple_t & t = *static_cast<tuple_t*>(lua_touserdata(L, lua_upvalueindex(1)));
      std::forward<F>(std::get<0>(t))(std::forward<Args>(std::get<1+Is>(t))...);
      return 0;
    }
  };
  
  static constexpr lua_CFunction cfunc = impl<detail::Count_t<sizeof...(Args)>>::cfunc;
};

template <typename F, typename... Args>
expected<void> cpppcall(lua_State * L, F && f, Args && ... args) noexcept {
  using P = protected_call_helper<F, Args...>;
  typename P::tuple_t tuple{std::forward<F>(f), std::forward<Args>(args)...};
  lua_pushlightuserdata(L, static_cast<void*>(&tuple));
  lua_pushcclosure(L, P::cfunc, 1);

  return fcn_call_no_ret(L, 0); // Note that this is noexcept
}

} // end namespace primer
