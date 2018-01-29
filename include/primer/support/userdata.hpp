//  (C) Copyright 2015 - 2018 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/lua.hpp>
#include <primer/metatable.hpp>

#include <primer/detail/is_userdata.hpp>
#include <primer/detail/type_traits.hpp>

#include <primer/support/asserts.hpp>
#include <primer/support/diagnostics.hpp>
#include <primer/support/metatable.hpp>
#include <primer/support/userdata_common.hpp>

#include <primer/traits/userdata.hpp>

#include <cstring>
#include <new>
#include <type_traits>
#include <utility>

namespace primer {

namespace detail {

template <typename T, typename ENABLE = void>
struct udata_helper;

template <typename T>
struct udata_helper<T, enable_if_t<primer::detail::is_userdata<T>::value>> {
  using udata = primer::traits::userdata<T>;

  // Based on impl of luaL_testudata
  static T * test_udata(lua_State * L, int idx) {
    PRIMER_ASSERT_STACK_NEUTRAL(L);
    if (void * p = lua_touserdata(L, idx)) { /* value is a userdata? */
      if (lua_getmetatable(L, idx)) {        /* does it have a metatable? */
        primer::push_metatable<T>(L);        /* get correct metatable */
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
    primer::push_metatable<T>(L);
    lua_setmetatable(L, -2);
  }
};

} // end namespace detail

} // end namespace primer
