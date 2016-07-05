//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * This api feature manages a list of callbacks with names and help strings,
 * as is produced by lua::callback_registrar.
 *
 * It provides them as global functions to the lua state, and ensures that they
 * are in the persistent objects table.
 *
 * It also registers their help strings.
 *
 * Finally, it is initalizes the lua_extraspace pointer to the value "owner_ptr"
 * which is passed to it. This should point to the base class of any member
 * functions that are passed to lua using the ADAPT_EXTRASPACE macro. This
 * should generally be the class deriving from api::base.
 *
 * Note that you can use other methods for registering / dispatching callbacks,
 * but the extraspace method will be the most performant.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/lua.hpp>

#include <primer/api/help.hpp>
#include <primer/api/callback_registrar.hpp>

#include <primer/detail/span.hpp>
#include <primer/support/set_funcs.hpp>

namespace primer {

namespace api {

class callback_manager {

  detail::span<const luaW_Reg> list_;
  void * owner_ptr_;

public:
  template <typename T>
  constexpr explicit callback_manager(const detail::span<const luaW_Reg> & _l,
                                      T * _owner_ptr)
    : list_(_l)
    , owner_ptr_(static_cast<void *>(_owner_ptr))
  {}

  // This is the ctor you should usually use, when using callback_manger
  // with an api_base object
  template <typename T>
  constexpr explicit callback_manager(T * _owner_ptr)
    : callback_manager(T::callbacks_array(), _owner_ptr)
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
  void on_persist_table(lua_State * L) const { set_funcs_reverse(L, list_); }

  // Register as name - func pairs
  void on_unpersist_table(lua_State * L) const { set_funcs(L, list_); }
};

} // end namespace api

} // end namespace primer
