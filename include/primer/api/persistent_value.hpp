//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * Represents a C++ value that is stored in the target table just prior to
 * serialization, and restored from the target table upon deserialization.
 * This is an easy way to create a persistent value outside of the lua state,
 * provided that the type can be pushed and read.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/lua.hpp>
#include <primer/push.hpp>
#include <primer/read.hpp>

#include <type_traits>
#include <utility>

namespace primer {
namespace api {

template <typename T>
struct persistent_value {
  T value_;

  T & get() & { return value_; }
  T const & get() const & { return value_; }

  //
  // API Feature
  //

  void on_init(lua_State *) {}
  void on_persist_table(lua_State *) {}
  void on_unpersist_table(lua_State *) {}

  void on_serialize(lua_State * L) { primer::push(L, value_); }

  PRIMER_STATIC_ASSERT(std::is_nothrow_move_constructible<T>::value,
                       "persistent value must be nothrow move constructible");
  void on_deserialize(lua_State * L) {
    if (auto result = primer::read<T>(L, -1)) {
      value_ = std::move(*result);
    } else {
      // XXX TODO
    }
    lua_pop(L, 1);
  }
};

} // end namespace api
} // end namespace primer
