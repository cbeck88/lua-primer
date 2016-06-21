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
#include <primer/traits/is_optional.hpp>
#include <primer/traits/optional.hpp>
#include <primer/traits/is_userdata.hpp>
#include <primer/support/diagnostics.hpp>
#include <primer/support/userdata.hpp>

#include <string>

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
      return primer::error{"Expected string, found ", primer::describe_lua_value(L, idx)};
    }
  }
};

template <>
struct read<std::string> {
  static expected<std::string> from_stack(lua_State * L, int idx) {
    return read<const char *>::from_stack(L, idx);
  }
};

template <>
struct read<int> {
  static expected<int> from_stack(lua_State * L, int idx) {
    if (lua_isinteger(L, idx)) {
      return static_cast<int>(lua_tointeger(L, idx)); // TODO: Should this be different if LUA_INTEGER is larger than int? :/
    } else {
      return primer::error{"Expected integer, found ", primer::describe_lua_value(L, idx)};
    }
  }
};

template <>
struct read<uint> {
  static expected<uint> from_stack(lua_State * L, int idx) {
    auto maybe_int = read<int>::from_stack(L, idx);
    if (maybe_int && *maybe_int < 0) { return primer::error{"Expected nonnegative integer, found ", std::to_string(*maybe_int)}; }
    return maybe_int; // implicit static cast to uint, in expected ctor.
  }
};

template <>
struct read<float> {
  static expected<float> from_stack(lua_State * L, int idx) {
    if (lua_isnumber(L, idx)) {
      return lua_tonumber(L, idx);
    } else {
      return primer::error{"Expected number, found ", primer::describe_lua_value(L, idx)};
    }
  }
};

template <>
struct read<bool> {
  static expected<bool> from_stack(lua_State * L, int idx) {
    if (lua_isboolean(L, idx)) {
      return static_cast<bool>(lua_toboolean(L, idx));
    } else {
      return primer::error{"Expected boolean, found ", primer::describe_lua_value(L, idx)};
    }
  }
};


// Userdata
template <typename T>
struct read<T &, enable_if_t<primer::traits::is_userdata<T>::value>> {
  static expected<T&> from_stack(lua_State * L, int idx) {
    if (T * t = primer::test_udata<T>(L, idx)) {
      return *t;
    } else {
      return primer::error{"Expected userdata '", primer::udata_name<T>(), "', found ", primer::describe_lua_value(L, idx)};
    }
  }
};

// Strict optional
template <typename T>
struct read<T, enable_if_t<primer::traits::is_optional<T>::value && !primer::traits::is_relaxed_optional<T>::value>> {
  using helper = primer::traits::optional<T>;
  static expected<T> from_stack(lua_State * L, int idx) {
    if (lua_isnoneornil(L, idx)) { return helper::make_blank(); }
    if(auto maybe_result = primer::traits::read<typename helper::base_type>(L, idx)) {
      return helper::from_base(*std::move(maybe_result));
    } else {
      return std::move(maybe_result).err();
    }
  }
};

// Relaxed optional
template <typename T>
struct read<T, enable_if_t<primer::traits::is_optional<T>::value && primer::traits::is_relaxed_optional<T>::value>> {
  using helper = primer::traits::optional<T>;

  static expected<T> from_stack(lua_State * L, int idx) {
    if(auto maybe_result = primer::traits::read<typename helper::base_type>(L, idx)) {
      return helper::from_base(*std::move(maybe_result));
    } else {
      return helper::make_blank();
    }
  }
};

} // end namespace traits
} // end namespace primer
