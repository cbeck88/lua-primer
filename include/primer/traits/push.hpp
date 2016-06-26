//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * This trait is used to backup lua::push and tell it how to push
 * various types to the stack.
 *
 * The trait should provide:
 * static void to_stack(lua_State *, const T &);
 *   conversion function
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/lua.hpp>

#include <primer/support/integral_conversions.hpp>
#include <primer/support/userdata.hpp>

#include <primer/traits/is_optional.hpp>
#include <primer/traits/optional.hpp>
#include <primer/traits/is_userdata.hpp>
#include <primer/traits/userdata.hpp>

#include <string>
#include <utility>

namespace primer {
namespace traits {

// Primary template
template <typename T, typename ENABLE = void>
struct push;

// Nil (tag-dispatch)
template <>
struct push<primer::nil_t> {
  static void to_stack(lua_State * L, primer::nil_t) { lua_pushnil(L); }
};

// Strings
template <>
struct push<const char *> {
  static void to_stack(lua_State * L, const char * s) { lua_pushstring(L, s); }
};

// Std-String
template <>
struct push<std::string> {
  static void to_stack(lua_State * L, const std::string & str) {
    push<const char *>::to_stack(L, str.c_str());
  }
};

// Manually decay string literals...
template <std::size_t n>
struct push<char[n]> {
  static void to_stack(lua_State * L, const char(&str)[n]) {
    push<const char *>::to_stack(L, str);
  }
};

// Integral types
template <>
struct push<bool> {
  static void to_stack(lua_State * L, bool b) { lua_pushboolean(L, b); }
};

template <>
struct push<int> {
  static void to_stack(lua_State * L, int i) { lua_pushinteger(L, i); }
};

template <>
struct push<uint> {
  static void to_stack(lua_State * L, uint u) {
    push<int>::to_stack(L, detail::unsigned_to_signed<int>(u));
  }
};

template <>
struct push<float> {
  static void to_stack(lua_State * L, float f) { lua_pushnumber(L, f); }
};

// Userdata object
template <typename T>
struct push<T, enable_if_t<traits::is_userdata<T>::value>> {
  static void to_stack(lua_State * L, const T & t) {
    primer::push_udata<T>(L, t);
  }
};

// Optional
template <typename T>
struct push<T, enable_if_t<traits::is_optional<T>::value>> {
  static void to_stack(lua_State * L, const T & t) {
    if (const auto ptr = traits::optional<T>::as_ptr(t)) {
      using clean_base = remove_cv_t<typename traits::optional<T>::base_type>;
      push<clean_base>::to_stack(L, *ptr);
      // primer::push(L, *ptr);
    } else {
      lua_pushnil(L);
    }
  }
};

} // end namespace traits
} // end namespace primer
