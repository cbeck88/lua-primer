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
struct map_push_helper {
  using first_t = typename M::key_type;
  using second_t = typename M::mapped_type;

  static void to_stack(lua_State * L, const M & m) {
    lua_newtable(L);

    PRIMER_ASSERT_STACK_NEUTRAL(L);
    for (const auto & item : m) {
      traits::push<first_t>::to_stack(L, item.first);
      if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
      } else {
        traits::push<second_t>::to_stack(L, item.second);
        lua_settable(L, -3);
      }
    }
  }
  static constexpr detail::maybe_int stack_space_needed{
    detail::maybe_int::
      max(detail::stack_space_needed<traits::push<second_t>>::value,
          1 + detail::stack_space_needed<traits::push<first_t>>::value)};
};

// TODO: Exception safety, emplace can throw std::bad_alloc
template <typename M>
struct map_read_helper {
  using first_t = typename M::key_type;
  using second_t = typename M::mapped_type;

  PRIMER_STATIC_ASSERT(std::is_nothrow_constructible<M>::value,
                       "map type must be nothrow default constructible");
  PRIMER_STATIC_ASSERT(std::is_nothrow_move_constructible<first_t>::value,
                       "key type must be nothrow move constructible");
  PRIMER_STATIC_ASSERT(std::is_nothrow_move_constructible<second_t>::value,
                       "value type must be nothrow move constructible");

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
      if (auto first = traits::read<first_t>::from_stack(L, -1)) {
        if (auto second = traits::read<second_t>::from_stack(L, -2)) {
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
  static constexpr detail::maybe_int stack_space_needed{
    3 + detail::maybe_int::
          max(detail::stack_space_needed<traits::push<second_t>>::value,
              detail::stack_space_needed<traits::push<first_t>>::value)};
};

} // end namespace detail

} // end namespace primer
