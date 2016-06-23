//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * How to push "visitable structures" to the stack, as tables
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/support/asserts.hpp>
#include <primer/traits/push.hpp>
#include <primer/traits/util.hpp>
#include <visit_struct/visit_struct.hpp>

namespace primer {

/***
 * Visitor which pushes members to a table in lua
 */

namespace detail {

struct push_helper {
  lua_State * L;

  template <typename T>
  void operator()(const char * name, const T & value) {
    PRIMER_ASSERT_STACK_NEUTRAL(L);
    traits::push<T>::to_stack(L, value);
    lua_setfield(L, -2, name);
  }
};

} // end namespace detail


namespace traits {

template <typename T>
struct push<T, enable_if_t<visit_struct::traits::is_visitable<T>::value>> {
  static void to_stack(lua_State * L, const T & t) {
    lua_newtable(L);
    detail::push_helper vis{L};

    visit_struct::apply_visitor(vis, t);
  }
};

} // end namespace traits

} // end namespace primer
