//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * How to push `std::function` to the stack, as a closure object.
 * Note that this is fundamentally incompatible with eris, we can't really hope
 * to persist and restore these things. However it is provided for convenience
 * in general lua applications.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/adapt.hpp>
#include <primer/lua.hpp>
#include <primer/support/asserts.hpp>
#include <primer/support/push_cached.hpp>

#include <functional>
#include <new>
#include <utility>

namespace primer {

namespace detail {

template <typename R, typename... Args>
struct std_function_udata {
  using function_type = std::function<R(lua_State *, Args...)>;
  using this_type = std_function_udata<R, Args...>;

  function_type func_;

  // gc function for userdata
  static int gc(lua_State * L) {
    PRIMER_ASSERT(lua_isuserdata(L, 1),
                  "gc called with argument that is not userdata");

    using T = this_type;

    T * ptr = static_cast<T *>(lua_touserdata(L, 1));
    ptr->~T();
    return 0;
  }

  static void push_metatable(lua_State * L) {
    lua_newtable(L);
    lua_pushcfunction(L, &this_type::gc);
    lua_setfield(L, -2, "__gc");
    lua_pushliteral(L, "std_function");
    lua_setfield(L, -2, "__metatable");
  }

  static R closure_function(lua_State * L, Args... args) {
    PRIMER_ASSERT(lua_isuserdata(L, lua_upvalueindex(1)),
                  "closure_function called but first upvalue is not userdata");
    void * v = lua_touserdata(L, lua_upvalueindex(1));
    this_type * t = static_cast<this_type *>(v);
    return (t->func_)(L, std::forward<Args>(args)...);
  }

  static void push_instance(lua_State * L, function_type f) {
    new (lua_newuserdata(L, sizeof(std_function_udata)))
      std_function_udata{std::move(f)};
    push_cached<&this_type::push_metatable>(L);
    lua_setmetatable(L, -2);
    lua_pushcclosure(L, &adapt<R (*)(lua_State *, Args...),
                                 &this_type::closure_function>::adapted,
                     1);
  }
};

} // end namespace detail

template <typename R, typename... Args>
void push_std_function(lua_State * L, std::function<R(lua_State *, Args...)> f) {
  using helper = detail::std_function_udata<R, Args...>;
  helper::push_instance(L, std::move(f));
}

} // end namespace primer
