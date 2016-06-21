//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * Helper trait to test if an object is correctly registered as userdata
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/traits/userdata.hpp>
#include <primer/traits/util.hpp>

#include <type_traits>
#include <utility>

namespace primer {
namespace traits {

/***
 * Check if a type matches the "luaL_Reg" concept.
 *   const char * name
 *   int (*func)(lua_State *)
 */
template <typename T>
struct has_L_Reg_name
  : std::is_same<remove_reference_t<remove_cv_t<decltype(std::declval<T>().name)>>,
                 const char *> {};

template <typename T>
struct has_L_Reg_func
  : std::is_same<remove_reference_t<remove_cv_t<decltype(std::declval<T>().func)>>,
                 lua_CFunction> {};

template <typename T>
struct assert_L_Reg {
  static_assert(
    has_L_Reg_name<T>::value,
    "Type does not match luaL_Reg concept: T::name is not const char *");
  static_assert(
    has_L_Reg_func<T>::value,
    "Type does not match luaL_Reg concept: T::func is not lua_CFunction");
};

/***
 * Check if a type matches our userdata trait concept.
 *
 *   static const char * name;
 *   static const luaL_Reg_concept * methods;
 *
 * where luaL_Reg_concept is the previous concept.
 */

template <typename T>
struct has_userdata_trait_name
  : std::is_same<remove_reference_t<remove_cv_t<decltype(T::name)>>, const char *> {
};

template <typename T>
struct assert_userdata {
  static_assert(has_userdata_trait_name<T>::value,
                "Type does not match userdata trait concept: T::name is not "
                "static const char *");
  using methods_pointed_type =
    remove_cv_t<remove_pointer_t<decltype(T::methods)>>;
  using check_L_Reg = assert_L_Reg<methods_pointed_type>;
};



/***
 * Check if the userdata trait is specialized for a given type. We use the
 * assertions above to check that it is actually
 * valid, when we actually use it.
 */

template <typename T, typename ENABLE = void>
struct is_userdata : std::false_type {};

template <typename T>
struct is_userdata<T,
                   enable_if_t<std::is_same<primer::traits::userdata<T>,
                                            primer::traits::userdata<T>>::value>>
  : std::true_type {};

} // end namespace traits
} // end namespace primer
