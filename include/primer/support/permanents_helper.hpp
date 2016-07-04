//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * This is similar to the "metatable" header. It creates a trait that handles
 * the possibilities for the "permanents" member of the userdata trait, so that
 * we can populate the permanent objects table.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/lua.hpp>

#include <primer/detail/luaL_Reg.hpp>
#include <primer/detail/type_traits.hpp>

#include <primer/support/asserts.hpp>
#include <primer/support/diagnostics.hpp>
#include <primer/support/set_funcs.hpp>

namespace primer {
namespace detail {

template <typename T, typename ENABLE = void>
struct permanents_helper {
  static void populate(lua_State *) {}
  static void populate_reverse(lua_State *) {}
  static constexpr int value = 0;
};

template <typename T>
struct permanents_helper<T,
                         enable_if_t<detail::is_L_Reg_ptr<decltype(
                           primer::traits::userdata<T>::permanents)>::value>> {
  static void populate(lua_State * L) {
    primer::set_funcs(L, primer::traits::userdata<T>::permanents);
  }

  static void populate_reverse(lua_State * L) {
    primer::set_funcs_reverse(L, primer::traits::userdata<T>::permanents);
  }

  static constexpr int value = 1;
};

} // end namespace detail
} // end namespace primer
