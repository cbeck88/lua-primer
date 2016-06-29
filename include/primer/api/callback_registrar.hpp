//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * Class callback_registrar is the base class which sets up a class to dispatch
 * its member functions directly to lua as callbacks.
 *
 * callback_registrar is only concerned with producing a compile-time list of
 * the callbacks.
 * The actual LUA_FUNCTIONALITY is provided by class callback_manager.
 */

#include <primer/api/extraspace_dispatch.hpp>

#include <primer/detail/rank.hpp>
#include <primer/detail/typelist.hpp>
#include <primer/detail/span.hpp>

#include <array>

namespace primer {

/***
 * luaW_Reg
 * An extension of luaL_Reg which has help info
 */

struct luaW_Reg {
  const char * name;
  lua_CFunction func;
  const char * help;
};

/***
 * luaW_RegType:
 * A compile-time version of luaW_Reg
 */
template <const char * (*name)(), const char * (*help)(), lua_CFunction (*func)()>
struct luaW_RegType {
  static constexpr luaW_Reg get() { return {name(), func(), help()}; }
};

namespace detail {

// Map compile-time list to an array
template <typename T, typename TL>
struct objlist_adaptor;

template <typename T, typename... Ts>
struct objlist_adaptor<T, TypeList<Ts...>> {
  static constexpr std::array<T, sizeof...(Ts)> to_array() {
    return {{Ts::get()...}};
  }
};

} // end namespace detail

/***
 * Actual callback_registrar template
 */

template <typename T>
class callback_registrar {

protected:
  typedef T owner_type;

  // detail, typelist assembly
  static inline detail::TypeList<> GetCallbacks(primer::detail::Rank<0>);

  static constexpr int maxCallbacks = 100;

public:
/***
 * Methods to access the registries in a simple form at runtime.
 */

#define GET_CALLBACKS                                                          \
  decltype(owner_type::GetCallbacks(primer::detail::Rank<maxCallbacks>{}))

  static detail::span<const luaW_Reg> callbacks_array() {
    // arrays of size zero are not allowed
    // this check silences clang ubsan
    if (!GET_CALLBACKS::size) { return {}; }

    static auto static_instance(
      detail::objlist_adaptor<const luaW_Reg, GET_CALLBACKS>::to_array());
    return static_instance;
  }
};

} // end namespace primer

/***
 * Macros to declare & register new callbacks
 */

/***
 * Use a given function pointer as a callback for this lua owner.
 * The function pointer is wrapped using PRIMER_ADAPT_EXTRASPACE
 */

#define USE_LUA_CALLBACK(name, help, fcn)                                         \
  static constexpr const char * lua_callback_name_##name() { return #name; }      \
  static constexpr const char * lua_callback_help_##name() { return help; }       \
  static constexpr lua_CFunction lua_get_member_ptr_##name() {                    \
    return PRIMER_ADAPT_EXTRASPACE(owner_type, fcn);                              \
  }                                                                               \
  typedef primer::luaW_RegType<                                                   \
    &owner_type::lua_callback_name_##name, &owner_type::lua_callback_help_##name, \
    &owner_type::lua_get_member_ptr_##name> reg_type_##name;                      \
  static inline primer::detail::Append_t<GET_CALLBACKS, reg_type_##name>          \
    GetCallbacks(primer::detail::Rank<GET_CALLBACKS::size + 1>);                  \
  static_assert(true, "")


/***
 * Declare & define inline a new lua callback. Return type is primer::result.
 * You are expected to use the automatic argument parsing by putting the
 * expected args in the function signature.
 * And signal errors by e.g. `return primer::error("...")`.
 * Don't longjmp out of this function, or it is a resource leak.
 */

#define NEW_LUA_CALLBACK(name, help)                                           \
  USE_LUA_CALLBACK(name, help, &owner_type::intf_##name);                      \
  primer::result intf_##name


/***
 * Declare & define inline a new "raw" lua callback, i.e. one which takes
 * lua_State * and returns int. This function is called basically directly from
 * lua, you have to call lua_error yourself and take care of longjmp yourself
 * if necessary.
 */
#define LUA_CALLBACK_RAW(name, help)                                           \
  USE_LUA_CALLBACK(name, help, &owner_type::intf_##name);                      \
  int intf_##name
