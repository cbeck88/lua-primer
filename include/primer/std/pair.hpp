//  (C) Copyright 2015 - 2017 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * How to transfer `std::pair` between the stack, as a table, and C++
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/error_capture.hpp>
#include <primer/lua.hpp>
#include <primer/traits/push.hpp>
#include <utility>

namespace primer {
namespace traits {

template <typename T, typename U>
struct push<std::pair<T, U>> {
  static void to_stack(lua_State * L, const std::pair<T, U> & p) {
    lua_createtable(L, 2, 0);
    push<T>::to_stack(L, p.first);
    lua_rawseti(L, -2, 1);
    push<U>::to_stack(L, p.second);
    lua_rawseti(L, -2, 2);
  }
  static constexpr int stack_space_needed = 2;
};

template <typename T, typename U>
struct read<std::pair<T, U>> {
  static expected<std::pair<T, U>> from_stack(lua_State * L, int idx) {
    PRIMER_ASSERT_STACK_NEUTRAL(L);

    std::pair<T, U> result;
    idx = lua_absindex(L, idx);

    if (!lua_istable(L, idx) && !lua_isuserdata(L, idx)) {
      return primer::arg_error(L, idx, "table");
    }

    // First
    {
      lua_pushinteger(L, 1);
      lua_gettable(L, idx);
      if (auto maybe_t = read<T>::from_stack(L, -1)) {
        result.first = std::move(*maybe_t);
        lua_pop(L, 1);
      } else {
        lua_pop(L, 1);
        return maybe_t.err().prepend_error_line("At index [1]:");
      }
    }

    // Second
    {
      lua_pushinteger(L, 2);
      lua_gettable(L, idx);
      if (auto maybe_u = read<U>::from_stack(L, -1)) {
        result.second = std::move(*maybe_u);
        lua_pop(L, 1);
      } else {
        lua_pop(L, 1);
        return maybe_u.err().prepend_error_line("At index [2]:");
      }
    }

    return result;
  }
  static constexpr int stack_space_needed = 1;
};

} // end namespace traits
} // end namespace primer
