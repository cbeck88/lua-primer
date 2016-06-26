//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * A helper which gives cached access to the result of a particular function.
 *
 * It uses the address of that function as a key in the registry, to cache the
 * output value of that function.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/lua.hpp>
#include <primer/support/asserts.hpp>

namespace primer {

namespace detail {

template <void (*producer_func)(lua_State *L)>
void push_cached(lua_State * L) {
  void * const registry_key = reinterpret_cast<void *>(producer_func);
  // Just get something usable as a key. It doesn't matter that it is implementation defined.
  // Note that even if the function is inline and defined in a header, C++ guarantees that it
  // has a unique address.
  //
  // The producer function should, without fail, push exactly one object onto the stack.
  lua_pushlightuserdata(L, registry_key);
  if (LUA_TNIL == lua_rawget(L, LUA_REGISTRYINDEX)) {
    PRIMER_ASSERT_STACK_NEUTRAL(L);
    lua_pop(L, 1);

    producer_func(L);
    if (lua_gettop(L) && !lua_isnil(L, -1)) {
      lua_pushlightuserdata(L, registry_key);
      lua_pushvalue(L, -2);
      lua_rawset(L, LUA_REGISTRYINDEX);
    }
  }
}

} // end namespace detail

} // end namespace primer
