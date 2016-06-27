//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * This lua functionality takes a list of callbacks with names and help strings,
 * as is produced by lua::callback_registrar.
 *
 * It provides them as global functions to the lua state, and ensures that they
 * are in the persistent objects table.
 *
 * It also registers their help strings. The "help" function implementation is
 * provided to the lua state.
 *
 * Finally, it is initalizes the lua_extraspace pointer to the value "owner_ptr"
 * which is passed to it. This should point to the base class of any member functions
 * that are passed to lua using the LUA_DELEGATE macro. This should generally be
 * the class deriving from api_base. There may only be one such class for each lua
 * state, otherwise, you must use a different mechanism for registering the other
 * functions.
 * (For example you could pass `std::function` objects as userdata to lua.)
 * Note that the extraspace method will be the most performant.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/lua.hpp>

#include <primer/api/help.hpp>
#include <primer/api/callback_registrar.hpp>

#include <primer/detail/span.hpp>

namespace primer {

namespace api {

class callback_manager {

  detail::span<const luaW_Reg> list_;
  void * owner_ptr_;

public:
  template <typename T>
  constexpr explicit callback_manager(const detail::span<const luaW_Reg> & _l, T * _owner_ptr)
    : list_(_l)
    , owner_ptr_(static_cast<void*>(_owner_ptr))
  {}

  static constexpr bool is_serial = false;

  void on_init(lua_State * L) const {
    // Initialize the extraspace to point to the owner
    api::set_extraspace_ptr(L, owner_ptr_);

    for (const auto & r : list_) {
      if (r.func) {
        api::set_help_string(L, r.func, r.help);
        lua_pushcfunction(L, r.func);
        lua_setglobal(L, r.name);
      }
    }
  }

  // Register as func - name pairs
  void on_persist_table(lua_State * L) const {
    for (const auto & r : list_) {
      if (r.func) {
        lua_pushcfunction(L, r.func);
        lua_pushstring(L, r.name);
        lua_settable(L, -3);
      }
    }
  }

  // Register as name - func pairs
  void on_unpersist_table(lua_State * L) const {
    for (const auto & r : list_) {
      if (r.func) {
        lua_pushcfunction(L, r.func);
        lua_setfield(L, -2, r.name);
      }
    }
  }
};

} // end namespace api

} // end namespace primer