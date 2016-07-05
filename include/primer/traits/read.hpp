//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * How to read primitive values from the lua stack
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/lua.hpp>

#include <primer/error.hpp>
#include <primer/expected.hpp>
#include <primer/userdata.hpp>
#include <primer/detail/maybe_int.hpp>
#include <primer/detail/is_userdata.hpp>
#include <primer/detail/type_traits.hpp>
#include <primer/support/diagnostics.hpp>
#include <primer/support/types.hpp>

#include <limits>
#include <string>
#include <type_traits>

namespace primer {
namespace traits {

// Primary template
template <typename T, typename ENABLE = void>
struct read;

// Primitive types
template <>
struct read<const char *> {
  static expected<const char *> from_stack(lua_State * L, int idx) {
    if (lua_type(L, idx) == LUA_TSTRING) {
      return lua_tostring(L, idx);
    } else {
      return primer::error{"Expected string, found ",
                           primer::describe_lua_value(L, idx)};
    }
  }
  static constexpr detail::maybe_int stack_space_needed{0};
};

template <>
struct read<std::string> {
  static expected<std::string> from_stack(lua_State * L, int idx) {
    return read<const char *>::from_stack(L, idx);
  }
  static constexpr detail::maybe_int stack_space_needed{0};
};

template <>
struct read<bool> {
  static expected<bool> from_stack(lua_State * L, int idx) {
    if (lua_isboolean(L, idx)) {
      return static_cast<bool>(lua_toboolean(L, idx));
    } else {
      return primer::error{"Expected boolean, found ",
                           primer::describe_lua_value(L, idx)};
    }
  }
  static constexpr detail::maybe_int stack_space_needed{0};
};

// Integral types

template <typename T, typename ENABLE = void>
struct signed_read_helper;

// No narrowing
template <typename T>
struct signed_read_helper<T, enable_if_t<sizeof(T) >= sizeof(LUA_INTEGER)>> {
  static expected<T> from_stack(lua_State * L, int idx) {
    if (lua_isinteger(L, idx)) {
      return static_cast<T>(lua_tointeger(L, idx));
    } else {
      return primer::error{"Expected integer, found ",
                           primer::describe_lua_value(L, idx)};
    }
  }
  static constexpr detail::maybe_int stack_space_needed{0};
};

// Narrowing, must do overflow check
template <typename T>
struct signed_read_helper<T, enable_if_t<sizeof(T) < sizeof(LUA_INTEGER)>> {
  static expected<T> from_stack(lua_State * L, int idx) {
    if (lua_isinteger(L, idx)) {
      LUA_INTEGER i = lua_tointeger(L, idx);
      if (i > std::numeric_limits<T>::max() ||
          i < std::numeric_limits<T>::min()) {
        return primer::error{"Integer overflow occurred: ", i};
      }
      return static_cast<T>(i);
    } else {
      return primer::error{"Expected integer, found ",
                           primer::describe_lua_value(L, idx)};
    }
  }
  static constexpr detail::maybe_int stack_space_needed{0};
};

// When reading unsigned, must do 0 check
template <typename T>
struct unsigned_read_helper {
  static expected<typename std::make_unsigned<T>::type> from_stack(lua_State * L,
                                                                   int idx) {
    auto maybe = read<T>::from_stack(L, idx);

    if (maybe && *maybe < 0) {
      maybe = primer::error{"Expected nonnegative integer, found ", *maybe};
    }

    // implicit conversion to expected<unsigned T> here, in expected ctor
    return maybe;
  }
  static constexpr detail::maybe_int stack_space_needed{0};
};

// Specialize read, using the helpers
// Note: It is done this way, as a partial specialization rather than a series
// of full-specializations,
// so that the user is free to supply full specializations.
template <typename T>
struct read<T,
            enable_if_t<std::is_same<T, int>::value ||  //
                        std::is_same<T, long>::value || //
                        std::is_same<T, long long>::value>>
  : signed_read_helper<T> {};

template <typename T>
struct read<T,
            enable_if_t<std::is_same<T, unsigned int>::value ||  //
                        std::is_same<T, unsigned long>::value || //
                        std::is_same<T, unsigned long long>::value>>
  : unsigned_read_helper<typename std::make_signed<T>::type> {};

// Floating point types
template <typename T>
struct read<T,
            enable_if_t<std::is_same<T, float>::value ||  //
                        std::is_same<T, double>::value || //
                        std::is_same<T, long double>::value>> {
  static expected<T> from_stack(lua_State * L, int idx) {
    expected<T> result;

    if (lua_isnumber(L, idx)) {
      result = static_cast<T>(lua_tonumber(L, idx));
    } else {
      result = primer::error{"Expected number, found ",
                             primer::describe_lua_value(L, idx)};
    }

    return result;
  }
  static constexpr detail::maybe_int stack_space_needed{0};
};


// Userdata
template <typename T>
struct read<T &, enable_if_t<primer::detail::is_userdata<T>::value>> {
  static expected<T &> from_stack(lua_State * L, int idx) {
    if (T * t = primer::test_udata<T>(L, idx)) {
      return *t;
    } else {
      return primer::error{"Expected userdata '", primer::udata_name<T>(),
                           "', found ", primer::describe_lua_value(L, idx)};
    }
  }
  static constexpr detail::maybe_int stack_space_needed{1};
  // have to push metatable to test
};

// Misc support types
template <>
struct read<nil_t> {
  static expected<nil_t> from_stack(lua_State * L, int idx) {
    if (lua_isnoneornil(L, idx)) {
      return nil_t{};
    } else
      return primer::error{"Expected nil, found ",
                           primer::describe_lua_value(L, idx)};
  }
  static constexpr detail::maybe_int stack_space_needed{0};
};

template <>
struct read<truthy> {
  static expected<truthy> from_stack(lua_State * L, int idx) {
    return primer::truthy{static_cast<bool>(lua_toboolean(L, idx))};
  }
  static constexpr detail::maybe_int stack_space_needed{0};
};

template <>
struct read<stringy> {
  static expected<stringy> from_stack(lua_State * L, int idx) {
    expected<stringy> result;

    if (luaL_callmeta(L, idx, "__tostring")) {
      if (const char * str = lua_tostring(L, -1)) {
        result = stringy{str};
      } else {
        result =
          primer::error{"__tostring metamethod did not produce a string: ",
                        primer::describe_lua_value(L, idx)};
      }
      lua_pop(L, 1);
    } else {
      switch (lua_type(L, idx)) {
        case LUA_TSTRING: {
          result = stringy{lua_tostring(L, idx)};
          break;
        }
        case LUA_TNUMBER: {
          lua_pushvalue(L, idx);
          result = stringy{lua_tostring(L, -1)};
          lua_pop(L, 1);
          break;
        }
        default:
          result = primer::error{"Could not convert to string: ",
                                 primer::describe_lua_value(L, idx)};
      }
    }
    return result;
  }
  static constexpr detail::maybe_int stack_space_needed{1};
};

} // end namespace traits
} // end namespace primer
