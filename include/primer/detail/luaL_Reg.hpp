//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * Tests to see if a value matches the "list of luaL_Reg" concept.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/detail/type_traits.hpp>
#include <type_traits>

namespace primer {
namespace detail {

/***
 * Check if a type matches the "luaL_Reg" concept.
 *   const char * name
 *   int (*func)(lua_State *)
 */
template <typename T>
struct has_L_Reg_name
  : std::is_same<remove_reference_t<decltype(std::declval<T>().name)>,
                 const char *> {};

template <typename T>
struct has_L_Reg_func
  : std::is_same<remove_reference_t<decltype(std::declval<T>().func)>,
                 lua_CFunction> {};

template <typename T>
struct assert_L_Reg {
  PRIMER_STATIC_ASSERT(
    has_L_Reg_name<T>::value,
    "Type does not match luaL_Reg concept: T::name is not const char *");
  PRIMER_STATIC_ASSERT(
    has_L_Reg_func<T>::value,
    "Type does not match luaL_Reg concept: T::func is not lua_CFunction");
};

// Test if a type is "pointer to (const or nonconst) L_Reg"
template <typename T, typename ENABLE = void>
struct is_L_Reg_ptr : std::false_type {};

template <typename T>
struct is_L_Reg_ptr<T *,
                    enable_if_t<has_L_Reg_name<remove_cv_t<T>>::value &&
                                has_L_Reg_func<remove_cv_t<T>>::value>>
  : std::true_type {};

template <typename T>
struct is_L_Reg_ptr<T * const> : is_L_Reg_ptr<T *> {};

} // end namespace detail
} // end namespace primer
