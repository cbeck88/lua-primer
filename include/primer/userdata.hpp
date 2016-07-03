//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/lua.hpp>
#include <primer/support/userdata.hpp>

#include <utility>

//[ primer_userdata
namespace primer {

/// Test if an entry on the stack is userdata of the given type, and if so,
/// return a pointer to it.
template <typename T>
T * test_udata(lua_State * L, int idx) {
  return detail::udata_helper<T>::test_udata(L, idx);
}

/// Create a userdata of the given type on the stack, using perfect forwarding.
template <typename T, typename... Args>
void push_udata(lua_State * L, Args &&... args) {
  new (lua_newuserdata(L, sizeof(T))) T{std::forward<Args>(args)...};
  detail::udata_helper<T>::set_metatable(L);
}

/// Easy, checked access to udata::name
template <typename T>
const char * udata_name() {
  return detail::udata_helper<T>::udata::name;
}

} // end namespace primer
//]
