//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * These visitors define the concept of "api feature".
 * Am api feature is some component or collection of components installed in a
 * lua State which requires support for persistence.
 *
 * An api feature is a lua-state-wide entity, usually existing as a member of an
 * api::base. It must provide methods for initialization, which makes it
 * available to use scripts, for persistence, in which it registers whatever
 * objects it needs to in the permanent objects table / persistence target
 * table, and and in which it restores itself from the saved data.
 *
 * The purpose is to help manage the book-keeping associated to pushing C++
 * api entities to lua, while making sure that we still know how to save and
 * restore the lua state afterwards using eris.
 *
 * Generally, that means any "opaque" things like C function pointers which is
 * pushes to lua, must also be placed in the persistent objects table.
 *
 * If the functionality is a userdata type, it usually means that some function
 * associated to the "__persist" function must be stored in the persistent
 * objects table.
 *
 * CONCEPT:
 *
 * struct functionality {
 *   void on_init(lua_State * L);
 *   void on_persist_table(lua_State * L);
 *   void on_unpersist_table(lua_State * L);
 *   void on_serialize(lua_State * L);
 *   void on_deserialize(lua_State * L);
 * };
 *
 * All these functions should be stack neutral.
 * They should not throw exceptions but may raise lua errors.
 *
 * `on_init`
 *   is called by `initialize_api`. This is where metatables, registry entries,
 *   global entities should be created.
 * `on_persist`
 *   will be called when the persist table is on top of the stack, entries
 *   should be created.
 * `on_unpersist`
 *   will be called when the unpersist table is on top of the stack.
 *   It should be dual.
 *
 * If `on_serialize` or `on_deserialize` is present, then the other should be
 * too, or there will be no effect.
 *
 * `on_serialize`
 *   should push one value onto the stack which can be persisted to represent
 *   this object.
 *
 * `on_deserialize`
 *   should recover one value from the top of the stack, to restore the object.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/detail/type_traits.hpp>
#include <primer/lua.hpp>
#include <primer/support/asserts.hpp>
#include <primer/support/diagnostics.hpp>

#include <type_traits>

namespace primer {

namespace api {

// Trait which validates that a type is an API feature
template <typename T, typename ENABLE = void>
struct has_on_init_method : std::false_type {};

template <typename T>
struct has_on_init_method<T, decltype(static_cast<T *>(nullptr)->on_init(
                                        static_cast<lua_State *>(nullptr)),
                                      void())> : std::true_type {};

template <typename T, typename ENABLE = void>
struct has_on_persist_table_method : std::false_type {};

template <typename T>
struct has_on_persist_table_method<T, decltype(
                                        static_cast<T *>(nullptr)
                                          ->on_persist_table(
                                            static_cast<lua_State *>(nullptr)),
                                        void())> : std::true_type {};

template <typename T, typename ENABLE = void>
struct has_on_unpersist_table_method : std::false_type {};

template <typename T>
struct has_on_unpersist_table_method<T, decltype(static_cast<T *>(nullptr)
                                                   ->on_unpersist_table(
                                                     static_cast<lua_State *>(
                                                       nullptr)),
                                                 void())> : std::true_type {};

// Trait which validates that a type is an API feature

template <typename T, typename ENABLE = void>
struct is_feature : std::false_type {};

template <typename T>
struct is_feature<T, decltype(static_cast<T *>(nullptr)->on_init(
                                static_cast<lua_State *>(nullptr)),
                              static_cast<T *>(nullptr)->on_persist_table(
                                static_cast<lua_State *>(nullptr)),
                              static_cast<T *>(nullptr)->on_unpersist_table(
                                static_cast<lua_State *>(nullptr)),
                              void())> : std::true_type {};

// Trait which validates that a type is a serial feature
template <typename T, typename ENABLE = void>
struct is_serial_feature : std::false_type {};

template <typename T>
struct is_serial_feature<T,
                         decltype(static_cast<T *>(nullptr)->on_init(
                                    static_cast<lua_State *>(nullptr)),
                                  static_cast<T *>(nullptr)->on_persist_table(
                                    static_cast<lua_State *>(nullptr)),
                                  static_cast<T *>(nullptr)->on_unpersist_table(
                                    static_cast<lua_State *>(nullptr)),
                                  static_cast<T *>(nullptr)->on_serialize(
                                    static_cast<lua_State *>(nullptr)),
                                  static_cast<T *>(nullptr)->on_deserialize(
                                    static_cast<lua_State *>(nullptr)),
                                  void())> : std::true_type {};

// Ptr to member type, for use in type lists

typedef const char * (*str_func_ptr_t)();

template <typename T, typename U, U T::*member_ptr, str_func_ptr_t name_func>
struct ptr_to_member {
  typedef T base_type;
  typedef U target_type;
  static constexpr U T::*get_ptr() { return member_ptr; }
  static U & get_target(T & t) { return t.*member_ptr; }
  static const char * get_name() { return name_func(); }
};

/***
 * It would be nice if we could use PRIMER_ASSERT_STACK_NEUTRAL here, but these
 * functions which we are calling may raise lua errors, which may be an
 * exception when lua is compiled as C++, and that would cause the stack
 * neutrality check to be fired from the dtor during stack unwinding. However,
 * here the stack neutrality should not be enforced when a lua error is raised,
 * the convention in lua is that you push the error message onto the stack
 * before raising the error.
 *
 * So, we need a version of PRIMER_ASSERT_STACK_NEUTRAL that doesn't happen
 * when exceptions are raised, and doesn't use the dtor of an object.
 */
 
#ifdef PRIMER_DEBUG
#define PRIMER_API_FEATURE_STACK_CHECK(L, S, NAME)                             \
   PRIMER_ASSERT(lua_gettop(L) == S, "API Feature " << NAME                    \
                 << " was not stack neutral: initial = " << S                  \
                 << " final = " << lua_gettop(L))
#else
   static_cast<void>(S)
#endif

struct on_init_visitor {
  lua_State * L;

  template <typename H, typename T>
  void visit_type(T & t) {
    int top = lua_gettop(L);
    H::get_target(t).on_init(L);
    PRIMER_API_FEATURE_STACK_CHECK(L, top, H::get_name());
  }
};

struct on_persist_table_visitor {
  lua_State * L;

  template <typename H, typename T>
  void visit_type(T & t) {
    PRIMER_ASSERT_TABLE(L);
    int top = lua_gettop(L);
    H::get_target(t).on_persist_table(L);
    PRIMER_API_FEATURE_STACK_CHECK(L, top, H::get_name());
  }
};

struct on_unpersist_table_visitor {
  lua_State * L;

  template <typename H, typename T>
  void visit_type(T & t) {
    PRIMER_ASSERT_TABLE(L);
    int top = lua_gettop(L);
    H::get_target(t).on_unpersist_table(L);
    PRIMER_API_FEATURE_STACK_CHECK(L, top, H::get_name());
  }
};

struct on_serialize_visitor {
  lua_State * L;

  template <typename H, typename T>
  enable_if_t<is_serial_feature<typename H::target_type>::value> visit_type(
    T & t) {
    PRIMER_ASSERT_TABLE(L);
    int top = lua_gettop(L);
    H::get_target(t).on_serialize(L);
    lua_setfield(L, -2, H::get_name());
    PRIMER_API_FEATURE_STACK_CHECK(L, top, H::get_name());
  }

  template <typename H, typename T>
  enable_if_t<is_feature<typename H::target_type>::value
              && !is_serial_feature<typename H::target_type>::value>
  visit_type(T &) {}
};

struct on_deserialize_visitor {
  lua_State * L;

  template <typename H, typename T>
  enable_if_t<is_serial_feature<typename H::target_type>::value> visit_type(
    T & t) {
    PRIMER_ASSERT_TABLE(L);
    int top = lua_gettop(L);
    lua_getfield(L, -1, H::get_name());
    H::get_target(t).on_deserialize(L);
    PRIMER_API_FEATURE_STACK_CHECK(L, top, H::get_name());
  }

  template <typename H, typename T>
  enable_if_t<is_feature<typename H::target_type>::value
              && !is_serial_feature<typename H::target_type>::value>
  visit_type(T &) {}
};

} // end namespace api

} // end namespace primer
