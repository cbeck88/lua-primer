//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * These visitors define the concept of "lua functionality".
 * A lua functionality installs a feature in a lua state and also sets up whatever
 * is necessary to make sure that the state can still be saved and reloaded using eris.
 *
 * Generally, that means any "opaque" things like C function pointers which is pushes
 * to lua, must also be placed in the persistent objects table.
 *
 * If the functionality is a userdata type, it usually means that some function associated
 * to the "__persist" function must be stored in the persistent objects table.
 *
 * CONCEPT:
 *
 * struct functionality {
 *   static constexpr bool is_serial = ???;
 *   void on_init(lua_State * L);
 *   void on_persist_table(lua_State * L);
 *   void on_unpersist_table(lua_State * L);
 *   void on_serialize(lua_State * L);
 *   void on_deserialize(lua_State * L);
 * };
 *
 * All these functions should be stack neutral.
 * on_init is where metatables, registry entries, global entities should be created.
 * on_persist will be called when the persist table is on top of the stack, entries should be created.
 * on_unpersist will be called when the unpersist table is on top of the stack. It should be dual.
 *
 * if is_serial is true, then
 * on_serialize should push one value onto the stack which can be persisted to represent this object.
 * on_deserialize should recover one value from the top of the stack, to restore the object.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/lua.hpp>
#include <primer/support/asserts.hpp>
#include <primer/support/diagnostics.hpp>

#include <type_traits>

namespace primer {

namespace api {

typedef const char * (*str_func_ptr_t)();

template <typename T, typename U, U::*T member_ptr, str_func_ptr_t name_func>
struct ptr_to_member {
  typedef T base_type;
  typedef U target_type;
  static constexpr U::*T get_ptr() { return member_ptr; }
  static U & get_target(T & t) { return t.*member_ptr; }
  static constexpr str_func_ptr_t get_name_func() { return name_func(); }
};

struct on_init_visitor {
  lua_State * L;

  template <typename H, typename T>
  void visit_type(T & t) {
    PRIMER_ASSERT_STACK_NEUTRAL(L);
    H::get_target(t).on_init(L);
  }
};

struct on_persist_table_visitor {
  lua_State * L;

  template <typename H, typename T>
  void visit_type(T & t) {
    PRIMER_ASSERT_STACK_NEUTRAL(L);
    H::get_target(t).on_persist_table(L);
  }
};

struct on_unpersist_table_visitor {
  lua_State * L;

  template <typename H, typename T>
  void visit_type(T & t) {
    PRIMER_ASSERT_STACK_NEUTRAL(L);
    H::get_target(t).on_persist_table(L);
  }
};

struct on_serialize_visitor {
  lua_State * L;

  template <typename H, typename T>
  typename std::enable_if<H::target_type::is_serial>::type
  visit_type(T & t) {
    PRIMER_ASSERT_STACK_NEUTRAL(L);
    H::get_target(t).on_serialize(L);
    lua_setfield(L, -2, H::get_name_func()());
  }

  template <typename H, typename T>
  typename std::enable_if<!H::target_type::is_serial>::type
  visit_type(T & t) {}
};

struct on_deserialize_visitor {
  lua_State * L;

  template <typename H, typename T>
  typename std::enable_if<H::target_type::is_serial>::type
  visit_type(T & t) {
    PRIMER_ASSERT_STACK_NEUTRAL(L);
    lua_getfield(L, -2, H::get_name_func()());
    H::get_target(t).on_deserialize(L);
  }

  template <typename H, typename T>
  typename std::enable_if<!H::target_type::is_serial>::type
  visit_type(T & t) {}
};

} // end namespace api

} // end namespace primer
