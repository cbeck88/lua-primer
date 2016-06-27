//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/lua.hpp>

#include <primer/support/asserts.hpp>
#include <primer/support/diagnostics.hpp>
#include <primer/support/userdata_common.hpp>

#include <primer/traits/userdata.hpp>
#include <primer/traits/util.hpp>

#include <cstring>
#include <type_traits>
#include <utility>

namespace primer {

namespace detail {

template <typename T, typename ENABLE = void>
struct udata_helper;

template <typename T>
struct udata_helper<
  T,
  typename std::enable_if<primer::traits::is_userdata<T>::value>::type> {
  using udata_check = primer::traits::assert_userdata<T>;
  using udata = primer::traits::userdata<T>;

  // Push metatable onto the stack. If it doesn't exist, then creates it. In
  // both cases, get it on the stack.
  // result = true implies it was created
  // result = false implies it already existed.
  static bool get_or_create_metatable(lua_State * L) {
    bool result = luaL_newmetatable(L, udata::name);
    if (result) {

      // Set the metatable to be its own __index table, unless user overrides
      // it.
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
      const char * gc_name = "__gc";

      for (auto ptr = udata::methods; ptr->name; ++ptr) {
        lua_pushcfunction(L, ptr->func);
        lua_setfield(L, -2, ptr->name);

        if (::strcmp(ptr->name, gc_name)) { saw_gc_metamethod = true; }
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
    return result;
  }

  // Based on impl of luaL_testudata
  static T * test_udata(lua_State * L, int idx) {
    PRIMER_ASSERT_STACK_NEUTRAL(L);
    if (void * p = lua_touserdata(L, idx)) { /* value is a userdata? */
      if (lua_getmetatable(L, idx)) {        /* does it have a metatable? */
        get_or_create_metatable(L);          /* get correct metatable */
        if (!lua_rawequal(L, -1, -2)) {      /* not the same? */
          p = nullptr; /* value is a userdata with wrong metatable */
        }
        lua_pop(L, 2); /* remove both metatables */
        return static_cast<T *>(p);
      }
    }
    return nullptr; /* value is not a userdata with a metatable */
  }

  // Based on impl of luaL_setmetatable
  // Sets the top of the stack entry to this metatable
  static void set_metatable(lua_State * L) {
    PRIMER_ASSERT_STACK_NEUTRAL(L);
    get_or_create_metatable(L);
    lua_setmetatable(L, -2);
  }
};

} // end namespace detail

/***
 * Forward facing interface
 */

/// Test if an entry on the stack is userdata of the given type, and if so,
/// return a pointer to it.
template <typename T>
T * test_udata(lua_State * L, int idx) {
  return detail::udata_helper<T>::test_udata(L, idx);
}

/// Create a userdata of the given type on the stack, using perfect forwarding.
template <typename T, typename... Args>
void push_udata(lua_State * L, Args &&... args) {
  new (lua_newuserdata(L, sizeof(T))) T{std::forward<Args>(args)...};
  detail::udata_helper<T>::set_metatable(L);
}

/// Easy, checked access to udata::name
template <typename T>
const char * udata_name() {
  return detail::udata_helper<T>::udata::name;
}

} // end namespace primer
