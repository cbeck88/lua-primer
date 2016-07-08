//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * Implementation of "common" metamethods for an arbitrary userdata type.
 * Currently we just have a `__gc` implementation in case (or in order that)
 * the user does not need to provide one.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/lua.hpp>

#include <primer/support/diagnostics.hpp>
#include <primer/traits/userdata.hpp>
#include <type_traits>

namespace primer {

namespace detail {

template <typename T>
int common_gc_impl(lua_State * L) noexcept {
  PRIMER_STATIC_ASSERT(std::is_nothrow_destructible<T>::value,
                       "userdata type must be no-throw destructible, or you "
                       "must write a custom __gc which handles the exception");

  using udata = primer::traits::userdata<T>;

  void * d = luaL_testudata(L, 1, udata::name);
  PRIMER_ASSERT(d, "garbage collection metamethod for userdata '"
                     << udata::name << "' called on object of type '"
                     << describe_lua_value(L, 1) << "'");
  static_cast<T *>(d)->~T();
  // Set metatable to nil. This prevents further access to the userdata, as
  // can happen in some obscure corner cases
  lua_pushnil(L);
  lua_setmetatable(L, 1);
  return 0;
}

} // end namespace detail

} // end namespace primer
