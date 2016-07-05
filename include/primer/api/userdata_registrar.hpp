//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * Defines an API feature which registerse a collection of userdata types and
 * makes sure they can be persisted
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/lua.hpp>
#include <primer/support/permanents_helper.hpp>
#include <primer/support/userdata.hpp>

namespace primer {
namespace api {

template <typename... Ts>
struct userdata_registrar {
  static constexpr bool is_serial = false;

  void on_init(lua_State * L) {
    int dummy[] = {(detail::udata_helper<Ts>::init_metatable(L), 0)..., 0};
    static_cast<void>(dummy);
  }

  void on_persist_table(lua_State * L) {
    int dummy[] = {(detail::permanents_helper<Ts>::populate_reverse(L), 0)...,
                   0};
    static_cast<void>(dummy);
  }

  void on_unpersist_table(lua_State * L) {
    int dummy[] = {(detail::permanents_helper<Ts>::populate(L), 0)..., 0};
    static_cast<void>(dummy);
  }
};

} // end namespace api
} // end namespace primer
