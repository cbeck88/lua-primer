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
#include <primer/metatable.hpp>
#include <primer/support/permanents_helper.hpp>
#include <primer/support/userdata.hpp>

namespace primer {
namespace api {

template <typename... Ts>
struct userdatas {
  void on_init(lua_State * L) {
    int dummy[] = {(primer::init_metatable<Ts>(L), 0)..., 0};
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
