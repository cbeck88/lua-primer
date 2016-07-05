//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * A lua_ref is a safe reference to an object in a lua VM.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/lua.hpp>

#include <primer/error.hpp>
#include <primer/expected.hpp>
#include <primer/read.hpp>

#include <primer/support/asserts.hpp>
#include <primer/support/lua_state_ref.hpp>

#include <utility>

namespace primer {

//[ primer_lua_ref
class lua_ref {
  /*<< A weak reference to a lua state.
       See `<primer/detail/lua_state_ref.hpp>` for details. >>*/
  lua_state_ref sref_;
  /*<< Holds the registry index to the object. Mutable because, if `sref_`
       becomes empty, we want to set `iref_` to `LUA_NOREF` immediately. >>*/
  mutable int iref_;

  //<-

  // Set to empty / disengaged state
  void set_empty() noexcept {
    sref_.reset();
    iref_ = LUA_NOREF;
  }

  // Check if we are engaged, and return lua_State * to our state if we are.
  lua_State * check_engaged() const noexcept {
    if (iref_ != LUA_NOREF) {
      lua_State * result = sref_.lock();
      if (!result) { iref_ = LUA_NOREF; }
      return result;
    }
    return nullptr;
  }

  // Not to be called unless the ref is in an empty / disengaged state.
  void init(lua_State * L) noexcept {
    if (L) {
      if (lua_gettop(L)) {
        sref_ = primer::obtain_state_ref(L);
        if (sref_) {
          iref_ = luaL_ref(L, LUA_REGISTRYINDEX);
          return;
        }
      }
    }

    this->set_empty();
  }

  // Release the ref if we are in an engaged state. Ends in empty state.
  void release() noexcept {
    if (lua_State * L = this->check_engaged()) {
      luaL_unref(L, LUA_REGISTRYINDEX, iref_);
    }
    this->set_empty();
  }

  // Pilfer the resources of another lua_ref.
  void move(lua_ref & other) noexcept {
    sref_ = std::move(other.sref_);
    iref_ = other.iref_;
    other.iref_ = LUA_NOREF;
    other.set_empty();
  }

  //->
public:
  // Special member functions

  lua_ref() noexcept { this->set_empty(); }
  lua_ref(lua_ref && other) noexcept { this->move(other); }
  lua_ref(const lua_ref & other) noexcept { this->init(other.push()); }
  ~lua_ref() noexcept { this->release(); }

  lua_ref & operator=(const lua_ref & other) noexcept {
    lua_ref temp{other};
    *this = std::move(temp);
    return *this;
  }

  lua_ref & operator=(lua_ref && other) noexcept {
    this->release();
    this->move(other);
    return *this;
  }

  // Primary constructor
  /*<< Pops an object from the top of given stack, and binds to it. If no
       object is on top, enters the empty state. >>*/
  explicit lua_ref(lua_State * L) noexcept { this->init(L); }


  // Push to main stack
  /*<< Attempts to push the object to the top of the primary stack of the state
used to create this lua_ref.

Returns a (valid) `lua_State *` if successfully locked.

Returns `nullptr` if that VM is closed, or if we are in the empty state.
      >>*/
  lua_State * push() const noexcept {
    if (lua_State * L = this->check_engaged()) {
      lua_rawgeti(L, LUA_REGISTRYINDEX, iref_);
      return L;
    }
    return nullptr;
  }

  // Push to a thread stack
  /*<< Attempts to push the object onto the top of a given ['thread stack].
It *must* be a thread in the same VM as the original stack, or the same
as the original stack.

Returns `true` if push was successful.

Returns `false` and pushes `nil` to
the given stack if the original VM is gone.

[caution If you try to push onto a stack belonging to *a different* lua VM,
undefined and unspecified behavior will result.

If `PRIMER_DEBUG` is defined, then
primer will check for this and call `std::abort` if it finds that you
broke this rule.

If `PRIMER_DEBUG` is not defined... very bad things are
likely to happen, including stack corruption of lua VMs.] >>*/
  bool push(lua_State * T) const noexcept {
    if (lua_State * L = this->check_engaged()) {
#ifdef PRIMER_DEBUG
      // This causes a lua_assert failure if states are unrelated
      lua_xmove(L, T, 0);
#else
      static_cast<void>(L); // suppress unused warning
#endif
      lua_rawgeti(T, LUA_REGISTRYINDEX, iref_);
      return true;
    } else {
      // Even if we are empty, T exists, and expects a value, so push nil.
      lua_pushnil(T);
      return false;
    }
  }

  /*<< Releases the lua reference, reverts to empty state. >>*/
  void reset() noexcept { this->release(); }


  // test validity
  /*<< Test if we are not in the empty state and can still be locked. >>*/
  explicit operator bool() const noexcept {
    return static_cast<bool>(this->check_engaged());
  }

  // Try to interpret the value as a specific C++ type
  /*<< Attempt to cast the lua value to a C++ value, using primer::read >>*/
  template <typename T>
  expected<T> as() const noexcept {
    expected<T> result{primer::error{"Can't lock VM"}};
    // ^ hoping for small string optimization here
    if (lua_State * L = this->push()) {
      result = primer::read<T>(L, -1);
      lua_pop(L, 1);
    }
    return result;
  }
};
//]

} // end namespace primer
