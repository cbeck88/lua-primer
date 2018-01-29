//  (C) Copyright 2015 - 2018 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * This code is associated with taking a primer::result object and passing the
 * signal to lua using the C API.
 *
 * The tricky part is, we want to be able to support lua whether it is compiled
 * as C or C++. This means that when we call lua error or lua yield, there could
 * be an exception, or there could be a longjmp, so we can't have any nontrivial
 * objects on the stack.
 *
 * To resolve that, we handle the signal in two steps.
 * First we take the primer::result by value, and consume it. If we need to push
 * something onto the stack, we do it then.
 * Then we return a `primer::return_or_yield` object, which is a trivial object.
 * In case of an error, we put that object in an invalid state (number = -1).
 *
 * In step two, we check the `return_or_yield` object, and either return, call
 * yield, or call lua_error as appropriate.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/error_capture.hpp>
#include <primer/lua.hpp>
#include <primer/result.hpp>

namespace primer {

namespace detail {

inline primer::return_or_yield
implement_result_step_one(lua_State * L, primer::result r) {
  auto & p = r.get_payload();
  if (p) {
    return std::move(*p);
  } else {
    primer::push_error(L, p.err());
    // Implementation note: This push can raise a memory error, but in
    // that case it must be an exception, so `r` is destroyed and nothing else
    // is leaked. We were going to raise an error anyways, so its okay.
    return primer::return_or_yield{-1, true};
  }
}

inline int
implement_result_step_two(lua_State * L, primer::return_or_yield r) {
  if (r.is_valid()) {
    if (r.is_return_) {
      return r.n_;
    } else {
      return lua_yield(L, r.n_);
    }
  } else {
    return lua_error(L);
  }
}

} // end namespace detail

} // en dnamespace primer
