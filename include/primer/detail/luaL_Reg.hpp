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
#include <primer/detail/span.hpp>
#include <type_traits>
#include <utility>

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

// Test if a type satisfies L_Reg concept
template <typename T, typename ENABLE = void>
struct is_L_Reg : std::false_type {};

template <typename T>
struct is_L_Reg<T, enable_if_t<has_L_Reg_name<T>::value && has_L_Reg_func<T>::value>>
  : std::true_type {};

// Assert that is a type satisfies L_Reg concept
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
struct is_L_Reg_ptr<T *, enable_if_t<is_L_Reg<remove_cv_t<T>>::value>>
  : std::true_type {};

template <typename T>
struct is_L_Reg_ptr<T * const> : is_L_Reg_ptr<T *> {};

// Helper that tries to make pointers / arrays look like containers, while
// letting containers pass through unchanged.
template <typename T, typename ENABLE = void>
struct is_L_Reg_sequence {
  static constexpr bool value = false;
};

// If it has a "begin" and "end" that look like an L_Reg iterator, then I guess
// it's a container
template <typename T>
struct is_L_Reg_sequence<
  T,
  enable_if_t<
    is_L_Reg<remove_reference_t<decltype(*std::declval<T>().begin())>>::value &&
    is_L_Reg<remove_reference_t<decltype(*std::declval<T>().end())>>::value>> {
  static constexpr bool value = true;
  static constexpr T adapt(T t) { return t; }
};

// If it an L_Reg pointer or a raw array of L_Reg, then convert it to a span
template <typename T>
struct is_L_Reg_sequence<T, enable_if_t<is_L_Reg_ptr<decay_t<T>>::value>> {
  static constexpr bool value = true;

  static constexpr decay_t<T> end_finder(decay_t<T> ptr) {
    return ptr->name ? end_finder(ptr + 1) : ptr;
  }

  using span_t = span<remove_pointer_t<decay_t<T>>>;
  static constexpr span_t adapt(decay_t<T> t) {
    return span_t{t, end_finder(t)};
  }
};

} // end namespace detail
} // end namespace primer
