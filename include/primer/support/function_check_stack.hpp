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

#include <primer/lua.hpp>
#include <primer/push.hpp>
#include <primer/error.hpp>
#include <primer/expected.hpp>

namespace primer {
namespace detail {

template <typename ... Args>
expected<void> check_stack_push_each(lua_State * L) {
  constexpr auto estimate = primer::stack_space_for_push_each<Args...>();
  if (estimate && !lua_checkstack(L, *estimate)) {
    return primer::error{"Insufficient stack space: needed ", *estimate,
                         "  lua MAXSTACK = ", LUAI_MAXSTACK};
  }
  return {};
}


} // end namespace detail
} // end namespace primer
