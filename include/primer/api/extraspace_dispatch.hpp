//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * Facilities to dispatch member function calls to an object using a pointer
 * to that object stored in the lua extraspace.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/lua.hpp>
#include <primer/adapt.hpp>
#include <primer/support/asserts.hpp>

namespace primer {

/***
 * The lua_getextraspace function returns a pointer to the start of the
 * extraspace block. This block has size and alignment at least that of a void
 * pointer, and is free for us to use, so we just treat the pointer to the block
 * as a pointer to such an object.
 *
 * access_extraspace_ptr returns a reference, which lets us manipulate this
 * pointer easily.
 */
namespace detail {
using void_ptr = void *;

inline void_ptr & access_extraspace_ptr(lua_State * L) {
  return *static_cast<void_ptr *>(lua_getextraspace(L));
}
} // end namespace detail

namespace api {

/***
 * Access to extraspace pointer.
 * Store and retrieve pointers to a user-defined type T there.
 *
 * The whole premise of this dispatch mechanism is that the pointer is assigned
 * when the callback functions are registered, so that "free functions" in lua
 * can actually be implemented as methods of some infrastructure object in C++.
 * See "api/callbacks" for the safe interface to this.
 */
template <typename T>
void set_extraspace_ptr(lua_State * L, T * t) {
  detail::access_extraspace_ptr(L) = static_cast<void *>(t);
}

template <typename T>
T * get_extraspace_ptr(lua_State * L) {
  return static_cast<T *>(detail::access_extraspace_ptr(L));
}

/***
 * Dispatcher primary template. Constructs the delegate, falling back to the
 * basic adaptor if possible.
 *
 * Template parameter T represents the extraspace type. It is explicitly
 * specified
 * for safety. You will get a type mismatch if it does not match the base type
 * of the member pointer.
 */

template <typename T, typename F, F f>
struct extraspace_dispatcher : public adapt<F, f> {};

/***
 * Specialize for a member function
 */

template <typename T,
          typename R,
          typename... Args,
          R (T::*target_func)(lua_State *, Args...)>
struct extraspace_dispatcher<T, R (T::*)(lua_State *, Args...), target_func> {

  static R dispatch_target(lua_State * L, Args... args) {
    T * object_ptr = get_extraspace_ptr<T>(L);
    PRIMER_ASSERT(object_ptr, "Extraspace pointer was not initialized!");
    return (object_ptr->*target_func)(L, std::forward<Args>(args)...);
  }

  static int adapted(lua_State * L) {
    using helper_t = adapt<R (*)(lua_State *, Args...), dispatch_target>;
    return helper_t::adapted(L);
  }
};

#define PRIMER_ADAPT_EXTRASPACE(t, f)                                          \
  &primer::api::extraspace_dispatcher<t, decltype(f), f>::adapted

} // end namespace api
} // end namespace primer
