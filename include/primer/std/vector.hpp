//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * How to push `std::vector` to the stack, as a tables
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/lua.hpp>
#include <primer/support/asserts.hpp>
#include <primer/traits/push.hpp>
#include <primer/traits/read.hpp>
#include <primer/traits/util.hpp>

#include <string>
#include <utility>
#include <vector>

namespace primer {

namespace traits {

template <typename T>
struct push<std::vector<T>> {
  static void to_stack(lua_State * L, const std::vector<T> & vec) {
    lua_newtable(L);

    PRIMER_ASSERT_STACK_NEUTRAL(L);
    int n = static_cast<int>(vec.size());
    for (int i = 0; i < n; ++i) {
      traits::push<remove_cv_t<T>>::to_stack(L, vec[i]);
      lua_rawseti(L, -2, (i+1));
    }
  }
};

template <typename T>
struct read<std::vector<T>> {
  static expected<std::vector<T>> from_stack(lua_State * L, int idx) {
    PRIMER_ASSERT_STACK_NEUTRAL(L);

    expected<std::vector<T>> result{primer::default_construct_in_place_tag{}};

    idx = lua_absindex(L, idx);
    if (lua_istable(L, idx)) {
      int n = lua_rawlen(L, idx);
      result->reserve(n);

      for (int i = 0; (i < n) && result; ++i) {
        lua_rawgeti(L, idx, i+1);
        if (auto object = traits::read<remove_cv_t<T>>::from_stack(L, -1)) {
          result->emplace_back(std::move(*object));
        } else {
          result = std::move(object.err());
          result.err().prepend_error_line("In index [" + std::to_string(i+1) + "],");
        }
        lua_pop(L, 1);
      }
    } else {
      result = primer::error("Expected: table, found ", primer::describe_lua_value(L, idx));
    }

    return result;
  }
};

} // end namespace traits

} // end namespace primer
