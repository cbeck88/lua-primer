//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * How to push `std::map` to the stack, as a table
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/lua.hpp>
#include <primer/support/asserts.hpp>
#include <primer/traits/push.hpp>
#include <primer/traits/read.hpp>
#include <primer/traits/util.hpp>

#include <map>
#include <string>
#include <utility>

namespace primer {

namespace traits {

template <typename T, typename U>
struct push<std::map<T, U>> {
  static void to_stack(lua_State * L, const std::map<T, U> & m) {
    lua_newtable(L);

    PRIMER_ASSERT_STACK_NEUTRAL(L);
    for (const auto & item : m) {
      push<T>::to_stack(L, item.first);
      if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
      } else {
        push<U>::to_stack(L, item.second);
        lua_settable(L, -3);
      }
    }
  }
};

template <typename T, typename U>
struct read<std::map<T, U>> {
  static expected<std::map<T, U>> from_stack(lua_State * L, int index) {
    if (!lua_istable(L, index) && !lua_isuserdata(L, index)) { return primer::error("Not a table"); }
    PRIMER_ASSERT_STACK_NEUTRAL(L);

    index = lua_absindex(L, index);
    std::map<T, U> result;

    lua_pushnil(L);
    while (lua_next(L, index) != 0) { // Stack is now ... index ... original_top , k, v
      // Backup the key in case one of the read functions messes with it, can cause very subtle problems
      lua_pushvalue(L, -2);       // original_top, k, v, k
      if (auto first = read<T>::from_stack(L, -1)) {
        if (auto second = read<U>::from_stack(L, -2)) {
          result.emplace(std::move(*first), std::move(*second));
          lua_pop(L, 2);
        } else {
          lua_pop(L, 3);
          return second.err();
        }
      } else {
        lua_pop(L, 3);
        return first.err();
      }
    }
    return result;
  }
};

} // end namespace traits

} // end namespace primer
