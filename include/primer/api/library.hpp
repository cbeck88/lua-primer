//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * Defines a simple interface to make lua libraries as API objects
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/lua.hpp>

namespace primer {
namespace api {

struct lua_base_lib {
  static constexpr const char * name = "";
  static constexpr lua_CFunction func = &luaopen_base;
};

#define CORE_LIB_DEFN(STR)                                                     \
struct lua_##STR##_lib {                                                       \
  static constexpr const char * name = #STR;                                   \
  static constexpr lua_CFunction func = &luaopen_##STR;                        \
};                                                                             \
static_assert(true, "")

CORE_LIB_DEFN(table);
CORE_LIB_DEFN(string);
CORE_LIB_DEFN(math);
CORE_LIB_DEFN(io);
CORE_LIB_DEFN(os);
CORE_LIB_DEFN(debug);

#undef CORE_LIB_DEFN

/***
 * An API feature object which registers a number of libraries
 */

template <typename... Ts>
class libraries {

  template <typename T>
  static void load_lib_globally(lua_State * L) {
    luaL_requiref(L, T::name, T::func, 1);
    lua_pop(L, 1);
  }

  template <typename T, bool kv_order>
  static void load_lib_into_table(lua_State * L) {
    PRIMER_ASSERT_TABLE(L);                  // [target]
    PRIMER_ASSERT_STACK_NEUTRAL(L);          // [target]
    luaL_requiref(L, T::name, T::func, 0);   // [target] [lib]
    lua_pushnil(L);                          // [target] [lib] [nil]
    while (lua_next(L, -2)) {                // [target] [lib] [k] [v]
      if (lua_iscfunction(L, -1)) {          // [target] [lib] [k] [f]
        lua_pushvalue(L, -2);                // [target] [lib] [k] [f] [k]
        if (kv_order) { lua_insert(L, -2); } // [target] [lib] [k] [?] [?]
        lua_settable(L, -5);                 // [target] [lib] [k]
      } else {                               //
        lua_pop(L, 1);                       // [target] [lib] [k]
      }                                      //
    }                                        // [target] [lib]
    lua_pop(L, 1);                           // [target]
  }


public:

  static constexpr bool is_serial = false;

  void on_init(lua_State * L) {
    int dummy[] = { (load_lib_globally<Ts>(L), 0)..., 0};
    static_cast<void>(dummy);
  }

  void on_persist_table(lua_State * L) {
    int dummy[] = { (load_lib_into_table<Ts, false>(L), 0)..., 0 };
    static_cast<void>(dummy);
  }

  void on_unpersist_table(lua_State * L) {
    int dummy[] = { (load_lib_into_table<Ts, true>(L), 0)..., 0 };
    static_cast<void>(dummy);
  }
};

using basic_libraries = libraries<lua_base_lib, lua_table_lib, lua_math_lib, lua_string_lib>;

} // end namespace api
} // end namespace primer
