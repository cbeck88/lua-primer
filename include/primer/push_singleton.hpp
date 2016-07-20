//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/lua.hpp>
#include <primer/support/asserts.hpp>

//[ primer_push_singleton_overview
/*`
It is fairly common in the lua API to want to create some private object
hidden in the registry to support some feature or hold some implementation
detail.

Often times, people make up string constants to use as the registry key,
and create a "lazy construction" function which first checks the registry
for a pre-cached value, and otherwise computes it.


This is essentially the same thing as lua's `require` all over again, but
from the C api side now.


We have a convenient template function which improves on this idiom. Primer
uses it internally for certain things. We use a template function called
`primer::push_singleton`:
*/

namespace primer {

template <void (*)(lua_State *)>
void push_singleton(lua_State *);

template <int (*)(lua_State *)>
void push_singleton(lua_State *);

} // end namespace primer

/*`
The idea is, the function which *creates* the value is the template
parameter, and the template function ensures the lazy construction aspect.


To ensure uniqueness of registry keys, the template uses the function
pointer ['corresponding to the producer function itself] as the registry
key.
*/
//]

namespace primer {

// Detail: A template which turns a `void(lua_State *)` function into an
// `int(lua_State *)` function trivially
namespace detail {

template <void (*producer_func)(lua_State * L)>
int
wrapped_as_cfunc(lua_State * L) {
  producer_func(L);
  return 0;
}

} // end namespace detail

template <int (*producer_func)(lua_State * L)>
void
push_singleton(lua_State * L) {
  constexpr lua_CFunction registry_key = producer_func;
  // Note that even if the function is inline and defined in a header, C++
  // guarantees that it has a unique address.
  //
  // The producer function should, without fail, push exactly one object onto
  // the stack.
  lua_pushcfunction(L, registry_key);
  if (LUA_TNIL == lua_rawget(L, LUA_REGISTRYINDEX)) {
    PRIMER_ASSERT_STACK_NEUTRAL(L);
    lua_pop(L, 1);

    producer_func(L);
    if (lua_gettop(L) && !lua_isnil(L, -1)) {
      lua_pushcfunction(L, registry_key);
      lua_pushvalue(L, -2);
      lua_rawset(L, LUA_REGISTRYINDEX);
    }
  }
}

template <void (*producer_func)(lua_State * L)>
void
push_singleton(lua_State * L) {
  push_singleton<detail::wrapped_as_cfunc<producer_func>>(L);
}

} // end namespace primer
