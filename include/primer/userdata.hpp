//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/lua.hpp>
#include <primer/support/userdata.hpp>
#include <primer/detail/is_userdata.hpp>
#include <primer/detail/type_traits.hpp>
#include <primer/detail/nothrow_newable.hpp>

#include <new>
#include <utility>

//[ primer_userdata
namespace primer {

/// Test if an entry on the stack is userdata of the given type, and if so,
/// return a pointer to it.
template <typename T>
T * test_udata(lua_State * L, int idx) {
  static_assert(detail::is_userdata<T>::value, "not a userdata type");
  return detail::udata_helper<T>::test_udata(L, idx);
}

/// Create a userdata of the given type on the stack, using perfect forwarding.
template <typename T, typename... Args>
auto push_udata(lua_State * L, Args &&... args)
  -> enable_if_t<detail::nothrow_newable<T, Args...>::value> {
  static_assert(detail::is_userdata<T>::value, "not a userdata type");
  new (lua_newuserdata(L, sizeof(T))) T{std::forward<Args>(args)...};
  detail::udata_helper<T>::set_metatable(L);
}

/// Use correct stack discipline if an exception is thrown
template <typename T, typename... Args>
auto push_udata(lua_State * L, Args &&... args)
  -> enable_if_t<!detail::nothrow_newable<T, Args...>::value> {
  static_assert(detail::is_userdata<T>::value, "not a userdata type");
  void * storage = lua_newuserdata(L, sizeof(T));

  PRIMER_TRY {
    new (storage) T{std::forward<Args>(args)...};
    detail::udata_helper<T>::set_metatable(L);
  }
  PRIMER_CATCH(...) {
    lua_pop(L, 1);
    PRIMER_RETHROW;
  }
}

/// Easy access to udata::name
template <typename T>
const char * udata_name() {
  static_assert(detail::is_userdata<T>::value, "not a userdata type");
  return detail::udata_helper<T>::udata::name;
}

/// Validate that a type is in fact recognized as userdata
template <typename T>
constexpr bool is_userdata() {
  return detail::is_userdata<T>::value;
}

} // end namespace primer
//]
