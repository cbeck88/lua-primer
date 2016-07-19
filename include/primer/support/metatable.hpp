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
    lua_pushcfunction(L, &primer::detail::common_gc_impl<T>);
    lua_setfield(L, -2, "__gc");
  }

  static constexpr int value = 0;
};

// full manual control for user
template <typename T>
struct metatable<T,
                 enable_if_t<noexcept(primer::traits::userdata<T>::metatable(
                               static_cast<lua_State *>(nullptr))) ==
                             noexcept(primer::traits::userdata<T>::metatable(
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
                 enable_if_t<detail::is_L_Reg_sequence<decltype(
                   primer::traits::userdata<T>::metatable)>::value>> {

  //[ primer_automatically_generated_metatable
  using udata = primer::traits::userdata<T>;

  static void populate(lua_State * L) {
    const auto & metatable_seq =
      detail::is_L_Reg_sequence<decltype(udata::metatable)>::adapt(
        udata::metatable);

    PRIMER_ASSERT_TABLE(L);
    PRIMER_ASSERT_STACK_NEUTRAL(L);

    // Assign the methods to the metatable
    // Use auto in case we use an expanded reg type later.
    bool saw_gc_metamethod = false;
    bool saw_index_metamethod = false;
    bool saw_metatable_metamethod = false;
    constexpr const char * gc_name = "__gc";
    constexpr const char * index_name = "__index";
    constexpr const char * metatable_name = "__metatable";

    for (const auto & reg : metatable_seq) {
      if (reg.name) {
        if (reg.func) {
          lua_pushcfunction(L, reg.func);
          lua_setfield(L, -2, reg.name);
        }
        if (0 == std::strcmp(reg.name, gc_name)) { saw_gc_metamethod = true; }
        if (0 == std::strcmp(reg.name, index_name)) {
          saw_index_metamethod = true;
        }
        if (0 == std::strcmp(reg.name, metatable_name)) {
          saw_metatable_metamethod = true;
        }
      }
    }

    // If the user did not register __gc then it is potentially (likely) a
    // leak, so install a trivial guy which calls the dtor.
    // Rarely want anything besides this anyways.
    if (!saw_gc_metamethod) {
      lua_pushcfunction(L, &primer::detail::common_gc_impl<T>);
      lua_setfield(L, -2, gc_name);
    }

    // Set the udata_name string also to be the "__metatable" field of the
    // metatable.
    // This means that when the user runs "getmetatable" on an instance,
    // they get a string description instead of the actual metatable, which
    // could be frightening
    if (!saw_metatable_metamethod) {
      lua_pushstring(L, udata::name);
      lua_setfield(L, -2, metatable_name);
    }

    // Set the metatable to be its own __index table, unless user overrides it.
    if (!saw_index_metamethod) {
      lua_pushvalue(L, -1);
      lua_setfield(L, -2, index_name);
    }
  }
  //]

  static constexpr int value = 2;
};

} // end namesapce detail
} // end namespace primer
