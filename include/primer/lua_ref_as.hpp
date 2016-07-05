//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * Implement the "as" method. This relies on primer::read, and
 * primer::traits::read is specialized for lua_ref... so this must be an 
 * out-of-line definition
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/lua.hpp>

#include <primer/error.hpp>
#include <primer/expected.hpp>
#include <primer/lua_ref.hpp>
#include <primer/read.hpp>

namespace primer {

template <typename T>
expected<T> lua_ref::as() const noexcept {
  expected<T> result{primer::error{"Can't lock VM"}};
  // ^ hoping for small string optimization here
  if (lua_State * L = this->push()) {
    result = primer::read<T>(L, -1);
    lua_pop(L, 1);
  }
  return result;
}

} // end namespace primer
