//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * Trait which sets up a metatable for a userdata type
 * Adapts to whether the user provided a list of methods, a function to call,
 * or nothing.
 *
 * Should provide `static void populate(lua_State *)`
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/lua.hpp>

#include <primer/detail/luaL_Reg.hpp>
#include <primer/detail/type_traits.hpp>

#include <primer/support/asserts.hpp>
#include <primer/support/diagnostics.hpp>
#include <primer/support/userdata_common.hpp>

#include <primer/traits/userdata.hpp>

#include <cstring>
#include <type_traits>
#include <utility>

namespace primer {

namespace detail {

// minimalistic, do-nothing metatable
template <typename T, typename ENABLE = void>
struct metatable {
  using udata = primer::traits::userdata<T>;

  static void populate(lua_State * L) {
    PRIMER_ASSERT_TABLE(L);
    // A minimalistic metatable with __gc
    lua_pushstring(L, udata::name);
    lua_setfield(L, -2, "__metatable");
    lua_pushcfunction(L, &primer::detail::common_meta<T>::impl_gc);
    lua_setfield(L, -2, "__gc");
  }

  static constexpr int value = 0;
};

// full manual control for user
template <typename T>
struct metatable<T,
                 enable_if_t<noexcept(primer::traits::userdata<T>::metatable(
                   static_cast<lua_State *>(nullptr)))>> {
  using udata = primer::traits::userdata<T>;

  static void populate(lua_State * L) {
    PRIMER_ASSERT_TABLE(L);
    PRIMER_ASSERT_STACK_NEUTRAL(L);
    primer::traits::userdata<T>::metatable(L);
  }

  static constexpr int value = 1;
};

// using luaL_Reg list
template <typename T>
struct metatable<T,
                 enable_if_t<detail::is_L_Reg_ptr<decltype(
                   primer::traits::userdata<T>::metatable)>::value>> {
  using udata = primer::traits::userdata<T>;

  static void populate(lua_State * L) {
    {
      detail::assert_L_Reg<remove_cv_t<remove_reference_t<decltype(
        *primer::traits::userdata<T>::metatable)>>> validate{};
      static_cast<void>(validate);
    }

    PRIMER_ASSERT_TABLE(L);
    PRIMER_ASSERT_STACK_NEUTRAL(L);

    // Set the metatable to be its own __index table, unless user overrides it.
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    // Set the udata_name string also to be the "__metatable" field of the
    // metatable.
    // This means that when the user runs "getmetatable" on an instance,
    // they get a string description instead of the actual metatable, which
    // could be frightening
    lua_pushstring(L, udata::name);
    lua_setfield(L, -2, "__metatable");

    // Assign the methods to the metatable
    // Use auto in case we use an expanded reg type later.
    bool saw_gc_metamethod = false;
    constexpr const char * gc_name = "__gc";

    for (auto ptr = udata::metatable; ptr->name; ++ptr) {
      if (ptr->func) {
        lua_pushcfunction(L, ptr->func);
        lua_setfield(L, -2, ptr->name);
      }

      if (0 == std::strcmp(ptr->name, gc_name)) { saw_gc_metamethod = true; }
    }

    // If the user did not register __gc then it is potentially (likely) a
    // leak,
    // so install a trivial guy which calls the dtor.
    // Rarely want anything besides this anyways.
    if (!saw_gc_metamethod) {
      lua_pushcfunction(L, &primer::detail::common_meta<T>::impl_gc);
      lua_setfield(L, -2, gc_name);
    }
  }

  static constexpr int value = 2;
};

} // end namesapce detail
} // end namespace primer