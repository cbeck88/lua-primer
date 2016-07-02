//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * A bound_function is a reference to an object of function type in a lua VM.
 *
 * This is just a convenience wrapper over lua_ref. The constructor provides
 * a check against `lua_isfunction`, and we provide several "call" methods which
 * are backed up by `primer/support/function.hpp`. These functions do not
 * require a lua_State * -- they attempt to lock the state that holds the
 * function, and the stack should be left unchanged, even if an error occurs.
 *
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/error.hpp>
#include <primer/expected.hpp>
#include <primer/lua.hpp>
#include <primer/lua_ref.hpp>
#include <primer/push.hpp>
#include <primer/support/function.hpp>

#include <utility>

namespace primer {

//[ primer_bound_function
class bound_function {
  lua_ref ref_;

public:
  // Special member functions
  bound_function() noexcept = default;
  bound_function(const bound_function &) noexcept = default;
  bound_function(bound_function &&) noexcept = default;
  bound_function & operator=(const bound_function &) noexcept = default;
  bound_function & operator=(bound_function &&) noexcept = default;
  ~bound_function() noexcept = default;

  // Only capture the top item if it is actually a function.
  explicit bound_function(lua_State * L) noexcept                  //
    : ref_((lua_gettop(L) && lua_isfunction(L, -1)) ? L : nullptr) //
  {}

  // Forwarded methods from lua_ref
  explicit operator bool() const noexcept { return static_cast<bool>(ref_); }

  lua_State * push() const noexcept { return ref_.push(); }
  bool push(lua_State * L) const noexcept { return ref_.push(L); }
  void reset() noexcept { ref_.reset(); }

  // Call methods
  template <typename... Args>
  expected<void> call_no_ret(Args &&... args) const noexcept /*<
    Calls the function, discards an return values. (But not errors.) >*/
  {
    if (lua_State * L = ref_.push()) {
      primer::push_each(L, std::forward<Args>(args)...);
      return primer::fcn_call_no_ret(L, sizeof...(args));
    } else {
      return primer::error("Could not lock lua state");
    }
  }

  template <typename... Args>
  expected<lua_ref> call_one_ret(Args &&... args) const noexcept /*<
    Calls the function, returns a reference to the first return value.
    (Or an error.) >*/
  {
    if (lua_State * L = ref_.push()) {
      primer::push_each(L, std::forward<Args>(args)...);
      return primer::fcn_call_one_ret(L, sizeof...(args));
    } else {
      return primer::error("Could not lock lua state");
    }
  }
};
//]

} // end namespace primer