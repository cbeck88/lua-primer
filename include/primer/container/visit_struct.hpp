//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * How to transfer "visitable structures" to and from the stack, as tables
 *
 * Note:
 * In order to be read, the members must be default-constructible and
 * nothrow move-assignable, nothrow swappable, or nothrow move constructible
 * to be used here.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/error_capture.hpp>
#include <primer/expected.hpp>
#include <primer/lua.hpp>
#include <primer/support/asserts.hpp>
#include <primer/traits/push.hpp>
#include <primer/traits/read.hpp>
#include <primer/detail/type_traits.hpp>
#include <primer/detail/move_assign_noexcept.hpp>

#include <visit_struct/visit_struct.hpp>

#include <type_traits>
#include <utility>

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
    PRIMER_ASSERT_TABLE(L);
    traits::push<T>::to_stack(L, value);
    lua_setfield(L, -2, name);
  }
};

struct stack_space_push_checker {
  int needed = 0;

  template <typename T>
  void operator()(const char *, const T &) {
    constexpr int s = stack_space_for_push<T>();
    if (s > needed) { needed = s; }
  }
};

struct field_counter {
  int count = 0;

  template <typename T>
  void operator()(const char *, const T &) {
    ++count;
  }
};

} // end namespace detail


namespace traits {

template <typename T>
struct push<T, enable_if_t<visit_struct::traits::is_visitable<T>::value>> {
  static void to_stack(lua_State * L, const T & t) {
    {
      detail::stack_space_push_checker sc{};
      visit_struct::apply_visitor(sc, t);
      if (sc.needed > 1 && !lua_checkstack(L, 1 + sc.needed)) {
        luaL_error(L, "insufficient stack space, needed %d", sc.needed);
      }
    }

    {
      detail::field_counter fc{};
      visit_struct::apply_visitor(fc, t);
      lua_createtable(L, 0, fc.count);
    }

    detail::push_helper vis{L};
    visit_struct::apply_visitor(vis, t);
  }
  static constexpr int stack_space_needed = 2;
};

} // end namespace traits

/***
 * Visitor which reads members from a table in lua
 */

namespace detail {

struct stack_space_read_checker {
  int needed = 0;

  template <typename T>
  void operator()(const char *, const T &) {
    constexpr int s = stack_space_for_read<T>();
    if (s > needed) { needed = s; }
  }
};

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
  void operator()(const char * name, T & value) noexcept {
    if (!ok) { return; }

    PRIMER_ASSERT_STACK_NEUTRAL(L);
    lua_getfield(L, index, name);

    if (auto result = traits::read<remove_cv_t<T>>::from_stack(L, -1)) {
      detail::move_assign_noexcept(value, std::move(*result));
    } else {
      ok = std::move(
        result.err().prepend_error_line("In field name '", name, "',"));
    }
    lua_pop(L, 1);
  }
};

} // end namespace detail

namespace traits {

template <typename T>
struct read<T, enable_if_t<visit_struct::traits::is_visitable<T>::value>> {
  static expected<T> from_stack(lua_State * L, int index) noexcept {
    PRIMER_STATIC_ASSERT(std::is_nothrow_constructible<T>::value,
                         "Primer cannot read this structure because it is not "
                         "no-throw constructible");

    expected<T> result{};

    detail::stack_space_read_checker sc{};
    visit_struct::apply_visitor(sc, *result);
    if (sc.needed > 1 && !lua_checkstack(L, 1 + sc.needed)) {
      result = primer::error::insufficient_stack_space(sc.needed);
    } else {
      index = lua_absindex(L, index);

      if (!lua_istable(L, index)) {
        result = primer::arg_error(L, index, "table");
      } else {
        detail::read_helper vis{L, index};
        visit_struct::apply_visitor(vis, *result);

        if (!vis.ok) { result = std::move(vis.ok.err()); }
      }
    }

    return result;
  }
  static constexpr int stack_space_needed = 2;
};

} // end namespace traits

} // end namespace primer
