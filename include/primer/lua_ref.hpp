//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * A lua_ref is a reference to an object in a lua VM.
 *
 * A lua_ref is created by pushing the object onto the stop of the stack, then
 * constructing lua_ref from the pointer `lua_State *`. This pops the object
 * from the stack.
 *
 * If the stack is empty, then the lua_ref is also in an
 * empty state. It can be default constructed in the empty state as well.
 *
 * If the lua_State * is destroyed (closed), the lua_ref reverts to the empty
 * state the next time it tries to be accessed.
 *
 * While the lua_State * is not closed, the referred object will not be garbage
 * collected by lua. There may simultaneously be other references to it
 * elsewhere in the VM.
 *
 * `class lua_ref` has three primary methods:

   - void reset()       // Releases the lua reference, reverts to empty state.

   - lua_State * push() const; // Attempts to push the object to the top of the
                               // *original* stack, used to create this lua_ref.
                               // Returns true if successful, (in fact, the
                               // original state). False (nullptr) if that
                               // lua_State is closed, or if we are in the empty
                               // state.

   - bool push(lua_State * L) const; // Attempts to push the object onto the top
                                     // of a given *thread stack*. It *must* be
                                     // a thread in the same VM as the original
                                     // stack, or the same as the original
                                     // stack.
                                     // Returns true if push was successful,
                                     // returns false and pushes nil to the
                                     // given state if the original stack is
                                     // gone.
                                     //
                                     // N.B. If you try to push onto a stack
                                     // from another lua VM, undefined and
                                     // unspecified behavior will result.
                                     // If PRIMER_DEBUG is defined, then primer
                                     // will check for this and call std::abort
                                     // if it finds that you broke this rule.
                                     // If PRIMER_DEBUG is not defined...
                                     // very bad things are likely to happen,
                                     // including stack corruption of lua VMs.
 *
 * Note that lua_ref is not thread-safe, it must not be passed among different
 * threads.
 *
 * Also note that while it is copyable, copying it is not recommended as it has
 * side-effects for the referred lua VM. If you need to copy a lua_ref, and your
 * lua VMs are very busy, it might be preferrable to put `lua_ref` in a
 * `std::shared_ptr` and copy those so that the `lua_ref` itself is not copied.
 * It would likely be worth profiling to compare the performance in your
 * application.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/lua.hpp>

#include <primer/error.hpp>
#include <primer/expected.hpp>
#include <primer/read.hpp>

#include <primer/lua_state_ref.hpp>
#include <primer/support/asserts.hpp>

namespace primer {

class lua_ref {
  lua_state_ref sref_;
  // If sref_ becomes empty, we want to set iref_ to LUA_NOREF immediately.
  mutable int iref_;

  // Initialize to empty / disengaged state
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

  // Release the ref if we are in an engaged state.
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
    other.set_empty();
  }

public:
  // Ctors
  explicit lua_ref(lua_State * L) noexcept { this->init(L); }

  lua_ref() noexcept { this->set_empty(); }

  lua_ref(lua_ref && other) noexcept { this->move(other); }

  lua_ref(const lua_ref & other) noexcept { this->init(other.push()); }

  // Dtor
  ~lua_ref() noexcept { this->release(); }

  // Assignment
  lua_ref & operator=(const lua_ref & other) noexcept {
    lua_ref temp{other};
    this->swap(temp);
    return *this;
  }

  lua_ref & operator=(lua_ref && other) noexcept {
    this->release();
    this->move(other);
    return *this;
  }

  // Swap
  void swap(lua_ref & other) noexcept {
    sref_.swap(other.sref_);

    int temp = other.iref_;
    other.iref_ = this->iref_;
    this->iref_ = temp;
  }

  // Push
  lua_State * push() const noexcept {
    if (lua_State * L = this->check_engaged()) {
      lua_rawgeti(L, LUA_REGISTRYINDEX, iref_);
      return L;
    }
    return nullptr;
  }

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

  // Reset
  void reset() noexcept { this->release(); }

  // operator bool
  explicit operator bool() const noexcept {
    return static_cast<bool>(this->check_engaged());
  }

  // Attempt to cast the lua value to a C++ value, using primer::read
  template <typename T>
  expected<T> as() const noexcept {
    if (lua_State * L = this->push()) {
      expected<T> result{primer::read<T>(L, -1)};
      lua_pop(L, 1);
      return result;
    } else {
      return primer::error("Could not lock lua state");
    }
  }
};

} // end namespace primer
