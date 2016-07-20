//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * Contains details of various caches that should be initialized by
 * `initialize_api` function call. It's a little cleaner to have the list of
 * such things in their own header.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/error_handler.hpp>
#include <primer/lua.hpp>
#include <primer/push_singleton.hpp>
#include <primer/support/lua_state_ref.hpp>

namespace primer {
namespace api {

inline void
init_caches(lua_State * L) {
  PRIMER_ASSERT_STACK_NEUTRAL(L);

  // Initialize some cached objects. If these can't be created later in a low
  // memory situation it will cause problems, so, try to preempt this.
  lua_state_ref::obtain_weak_ref_to_state(L);

  primer::push_singleton<primer::detail::fetch_traceback_function>(L);
  lua_pop(L, 1);
}

} // end namespace api
} // end namespace primer
