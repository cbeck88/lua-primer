//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * How to push `std::array` to the stack, as a table
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/lua.hpp>
#include <primer/support/asserts.hpp>
#include <primer/traits/push.hpp>
#include <primer/traits/read.hpp>
#include <primer/traits/util.hpp>

#include <array>
#include <utility>

namespace primer {

namespace traits {

template <typename T, std::size_t N>
struct push<std::array<T, N>> {
  static void to_stack(lua_State * L, const std::array<T, N> & arr) {
    lua_newtable(L);

    PRIMER_ASSERT_STACK_NEUTRAL(L);
    for (int i = 0; i < static_cast<int>(N); ++i) {
      traits::push<remove_cv_t<T>>::to_stack(L, arr[i]);
      lua_rawseti(L, -2, (i+1));
    }
  }
};

template <typename T, std::size_t N>
struct read<std::array<T, N>> {
  static expected<std::array<T, N>> from_stack(lua_State * L, int idx) {
    PRIMER_ASSERT_STACK_NEUTRAL(L);

    expected<std::array<T, N>> result{primer::default_construct_in_place_tag{}};

    idx = lua_absindex(L, idx);
    if (!lua_istable(L, idx)) { result = primer::error("Expected: table, found ", primer::describe_lua_value(L, idx)); }

    for (int i = 0; (i < static_cast<int>(N)) && result; ++i) {
      lua_rawgeti(L, idx, i+1);
      if (auto object = traits::read<remove_cv_t<T>>::from_stack(L, -1)) {
        (*result)[i] = std::move(*object);
      } else {
        result = std::move(object.err().prepend_error_line("In index [" + std::to_string(i+1) + "],"));
      }
      lua_pop(L, 1);
    }

    return result;
  }
};

} // end namespace traits

} // end namespace primer
