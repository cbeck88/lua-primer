//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * Helper macro which performs a stack check estimate, to be used before calling
 * push each.
 *
 * Pass it the lua stack and the parameter pack. It will return an error if the
 * check happens and fails.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/error.hpp>
#include <primer/expected.hpp>
#include <primer/lua.hpp>
#include <primer/push.hpp>

namespace primer {
namespace detail {

inline expected<void>
check_stack_push_n(lua_State * L, int n) {
  if (n && !lua_checkstack(L, n)) {
    return primer::error::insufficient_stack_space(n);
  }
  return {};
}

template <typename... Args>
expected<void>
check_stack_push_each(lua_State * L) {
  constexpr auto estimate = primer::stack_space_for_push_each<Args...>();
  return check_stack_push_n(L, estimate);
}

} // end namespace detail
} // end namespace primer
