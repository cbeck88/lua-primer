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

#include <primer/detail/integral_conversions.hpp>
#include <primer/detail/maybe_int.hpp>

#include <primer/support/types.hpp>

#include <primer/detail/type_traits.hpp>

#include <string>
#include <utility>

namespace primer {
namespace traits {

// Primary template
template <typename T, typename ENABLE = void>
struct push;

// Strings
template <>
struct push<const char *> {
  static void to_stack(lua_State * L, const char * s) { lua_pushstring(L, s); }
  static constexpr detail::maybe_int stack_space_needed{0};
};

// Std-String
template <>
struct push<std::string> {
  static void to_stack(lua_State * L, const std::string & str) {
    push<const char *>::to_stack(L, str.c_str());
  }
  static constexpr detail::maybe_int stack_space_needed{0};
};

// Manually decay string literals...
template <std::size_t n>
struct push<char[n]> {
  static void to_stack(lua_State * L, const char(&str)[n]) {
    push<const char *>::to_stack(L, str);
  }
  static constexpr detail::maybe_int stack_space_needed{0};
};

// Integral types
template <>
struct push<bool> {
  static void to_stack(lua_State * L, bool b) { lua_pushboolean(L, b); }
  static constexpr detail::maybe_int stack_space_needed{0};
};

// Signed types
template <typename T>
struct push<T,
            enable_if_t<std::is_same<T, int>::value ||  //
                        std::is_same<T, long>::value || //
                        std::is_same<T, long long>::value>> {
  PRIMER_STATIC_ASSERT(
    sizeof(T) <= sizeof(LUA_INTEGER),
    "Cannot push this type to lua, integer overflow could occur! "
    "Please convert to a smaller type.");
  static void to_stack(lua_State * L, T t) { lua_pushinteger(L, t); }
  static constexpr detail::maybe_int stack_space_needed{0};
};

// Unsigned types
template <typename T>
struct push<T,
            enable_if_t<std::is_same<T, unsigned int>::value ||  //
                        std::is_same<T, unsigned long>::value || //
                        std::is_same<T, unsigned long long>::value>> {
  static void to_stack(lua_State * L, T t) {
    // Pad or truncate to the size of LUA_INTEGER
    using unsigned_lua_int_t = std::make_unsigned<LUA_INTEGER>::type;
    const unsigned_lua_int_t temp = static_cast<unsigned_lua_int_t>(t);
    // Convert to signed value in a portable way
    const LUA_INTEGER temp2 = detail::unsigned_to_signed<LUA_INTEGER>(temp);
    // Defer to push<LUA_INTEGER>
    push<LUA_INTEGER>::to_stack(L, temp2);
  }
  static constexpr detail::maybe_int stack_space_needed{0};
};

// Floating point types
template <typename T>
struct push<T,
            enable_if_t<std::is_same<T, float>::value ||  //
                        std::is_same<T, double>::value || //
                        std::is_same<T, long double>::value>> {
  PRIMER_STATIC_ASSERT(
    sizeof(T) <= sizeof(LUA_NUMBER),
    "Cannot push this type to lua, floating point overflow could "
    "occur! Please convert to a smaller type.");
  static void to_stack(lua_State * L, T t) { lua_pushnumber(L, t); }
  static constexpr detail::maybe_int stack_space_needed{0};
};

// Misc support types
template <>
struct push<primer::nil_t> {
  static void to_stack(lua_State * L, primer::nil_t) { lua_pushnil(L); }
  static constexpr detail::maybe_int stack_space_needed{0};
};

template <>
struct push<truthy> {
  static void to_stack(lua_State * L, const truthy & t) {
    push<bool>::to_stack(L, t.value);
  }
  static constexpr detail::maybe_int stack_space_needed{0};
};

template <>
struct push<stringy> {
  static void to_stack(lua_State * L, const stringy & s) {
    push<std::string>::to_stack(L, s.value);
  }
  static constexpr detail::maybe_int stack_space_needed{0};
};

} // end namespace traits
} // end namespace primer
