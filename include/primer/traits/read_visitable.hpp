//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * How to read "visitable structures" from the stack, as tables (or userdata)
 *
 * The members must be default-constructible and move-constructible or copy-constructible to be used here.
 *
 * TODO: This whole implementation here should be conditioned on `std::is_nothrow_move_constructible` and
 * try-catch should be used with user-defined types for safety. For now we just assume that it's okay.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/expected.hpp>
#include <primer/lua.hpp>
#include <primer/support/asserts.hpp>
#include <primer/traits/read.hpp>
#include <primer/traits/util.hpp>

#include <visit_struct/visit_struct.hpp>

#include <utility>

namespace primer {

/***
 * Visitor which pushes members to a table in lua
 */

namespace detail {

struct read_helper {
  lua_State * L;
  int index;
  expected<void> ok;

  explicit read_helper(lua_State * _L, int _idx)
    : L(_L)
    , index(_idx)
    , ok{}
  {}

  template <typename T>
  void operator()(const char * name, T & value) {
    if (!ok) { return; }

    PRIMER_ASSERT_STACK_NEUTRAL(L);
    lua_getfield(L, index, name);
    if (auto result = primer::traits::read<primer::traits::remove_cv_t<T>>::from_stack(L, -1)) {
      value = std::move_if_noexcept(*result);
    } else {
      ok = std::move(result.err());
    }
    lua_pop(L, 1);
  }
};

} // end namespace detail

namespace traits {

template <typename T>
struct read<T, enable_if_t<visit_struct::traits::is_visitable<T>::value>> {
  static expected<T> from_stack(lua_State * L, int index) {
    expected<T> result{std::move(T{})}; // TODO: Improve this, possibly requires change to `expected`.

    detail::read_helper vis{L, lua_absindex(L, index)};
    visit_struct::apply_visitor(vis, *result);

    if (!vis.ok) { result = std::move(vis.ok.err()); }

    return result;
  }
};

} // end namespace traits

} // end namespace primer
