//  (C) Copyright 2015 - 2017 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * Facilities to dispatch member function calls to an object using a pointer
 * to that object stored in the lua extraspace.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/adapt.hpp>
#include <primer/lua.hpp>
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

inline void_ptr &
access_extraspace_ptr(lua_State * L) {
  return *static_cast<void_ptr *>(lua_getextraspace(L));
}
} // end namespace detail

namespace api {

/***
 * Dispatcher primary template. Constructs the delegate, falling back to the
 * basic adaptor if possible.
 *
 * Template parameter T represents the extraspace type. It is explicitly
 * specified for safety. You will get a type mismatch if it does not match the
 * base type of the member pointer.
 *
 * When using this dispatch method, make sure to use a `primer::api::callbacks`
 * object which will initialize the extraspace pointer to the correct value.
 */

template <typename T, typename F, F f>
struct extraspace_dispatcher : public adapt<F, f> {};

/***
 * Specialize for a member function
 */

template <typename T, typename R, typename... Args,
          R (T::*target_func)(lua_State *, Args...)>
struct extraspace_dispatcher<T, R (T::*)(lua_State *, Args...), target_func> {

  static R dispatch_target(lua_State * L, Args... args) {
    void * vptr = detail::access_extraspace_ptr(L);
    PRIMER_ASSERT(vptr, "Extraspace pointer was not initialized!");
    T * object_ptr = static_cast<T *>(vptr);
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
