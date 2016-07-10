//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * Support code related to capturing return values of a function.
 *
 * Helps to write generic code which works regardless of number of returns
 * expected.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/support/function_return_fwd.hpp>

#include <primer/error.hpp>
#include <primer/expected.hpp>
#include <primer/lua_ref.hpp>
#include <primer/lua_ref_seq.hpp>

namespace primer {
namespace detail {

template <>
struct return_helper<lua_ref> {
  using return_type = expected<lua_ref>;

  static void pop(lua_State * L, int, return_type & result) {
    result = lua_ref{L};
  }
  static constexpr int nrets = 1;
};

template <>
struct return_helper<lua_ref_seq> {
  using return_type = expected<lua_ref_seq>;

  static void pop(lua_State * L, int start_idx, return_type & result) {
    PRIMER_TRY_BAD_ALLOC {
      result = return_type{default_construct_in_place_tag{}};
      primer::pop_n(L, lua_gettop(L) - start_idx + 1, *result);
    }
    PRIMER_CATCH_BAD_ALLOC { result = primer::error::bad_alloc(); }
  }

  static constexpr int nrets = LUA_MULTRET;
};

} // end namespace detail
} // end namespace primer
