//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * primer::push function. Push a single object to the stack, resolves
 * how to do so using the push trait.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/detail/stack_push_each_helper.hpp>
#include <primer/traits/push.hpp>
#include <utility>

namespace primer {

//[ primer_push
template <typename T>
void push(lua_State * L, const T & t);
//]

//[ primer_push_impl
template <typename T>
void push(lua_State * L, const T & t) {
  ::primer::traits::push<T>::to_stack(L, t);
//<-
  // Silence an unused warning
  static_cast<void>(::primer::traits::push<T>::stack_space_needed);
//->
}
//]

//[ primer_push_each
// Variadic push: Push a sequence of objects onto the stack.
template <typename... Args>
void push_each(lua_State * L, Args &&... args) {
  int dummy[] = {(::primer::push(L, std::forward<Args>(args)), 0)..., 0};
  static_cast<void>(dummy);
//<-
  // If args is empty this may resolve to a no-op
  static_cast<void>(L);
//->
}
//]

//[ primer_stack_space_for_push
template <typename T>
constexpr int stack_space_for_push();
//]

template <typename T>
constexpr int stack_space_for_push() {
  return ::primer::traits::push<T>::stack_space_needed;
}


//[ primer_stack_space_for_push_each
template <typename... Args>
constexpr int stack_space_for_push_each() {
  return primer::detail::stack_push_each_helper<Args...>::value();
}
//]

} // end namespace primer
