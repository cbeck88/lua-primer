//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * How to push standard map objects like `std::map`, `std::unordered_map` to the
 * stack, as a table. This is a pair of common base classes using a template
 * parameter.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/expected.hpp>
#include <primer/lua.hpp>
#include <primer/detail/maybe_int.hpp>
#include <primer/support/asserts.hpp>
#include <primer/traits/push.hpp>
#include <primer/traits/read.hpp>
#include <primer/detail/type_traits.hpp>

#include <utility>

namespace primer {

namespace detail {

template <typename M>
struct set_push_helper {
  using first_t = typename M::key_type;

  static void to_stack(lua_State * L, const M & m) {
    if (std::is_integral<first_t>::value) {
      lua_createtable(L, m.size(), 0);
    } else {
      lua_createtable(L, 0, m.size());
    }

    PRIMER_ASSERT_STACK_NEUTRAL(L);
    for (const auto & item : m) {
      traits::push<first_t>::to_stack(L, item);
      if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
      } else {
        lua_pushboolean(L, true);
        lua_settable(L, -3);
      }
    }
  }
  // Either three, after pushing the boolean, or max achieved when pushing key
  static constexpr detail::maybe_int stack_space_needed{
    detail::maybe_int::max(detail::maybe_int{3},
                           1 + detail::stack_space_needed<
                                 traits::push<first_t>>::value)};
};

template <typename M>
struct set_read_helper {
  using first_t = typename M::key_type;

  PRIMER_STATIC_ASSERT(std::is_nothrow_constructible<M>::value,
                       "set must be nothrow default constructible");
  PRIMER_STATIC_ASSERT(std::is_nothrow_move_constructible<first_t>::value,
                       "key type must be nothrow move constructible");

  static expected<M> from_stack(lua_State * L, int index) {
    if (!lua_istable(L, index) && !lua_isuserdata(L, index)) {
      return primer::error("Not a table");
    }
    PRIMER_ASSERT_STACK_NEUTRAL(L);

    index = lua_absindex(L, index);
    M result;

    lua_pushnil(L);
    while (lua_next(L, index) != 0) {
      // Stack is now ... index ... original_top , k, v
      // Backup the key in case one of the read functions messes with it, can
      // cause very subtle problems
      lua_pushvalue(L, -2); // original_top, k, v, k
      if (auto key = traits::read<first_t>::from_stack(L, -1)) {
        if (lua_toboolean(L, -2)) {
          PRIMER_TRY { result.emplace(std::move(*key)); }
          PRIMER_CATCH(std::bad_alloc &) {
            lua_pop(L, 3);
            return primer::error(bad_alloc_tag{});
          }
        }
        lua_pop(L, 2);
      } else {
        lua_pop(L, 3);
        return key.err();
      }
    }
    return std::move(result);
  }
  static constexpr detail::maybe_int stack_space_needed{
    3 + detail::stack_space_needed<traits::read<first_t>>::value};
};

} // end namespace detail

} // end namespace primer
