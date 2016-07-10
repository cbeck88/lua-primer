//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * A lua_ref is a reference to an object in a lua VM.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/cpp_pcall.hpp>
#include <primer/error.hpp>
#include <primer/expected.hpp>
#include <primer/lua.hpp>

#include <primer/support/asserts.hpp>
#include <primer/support/lua_state_ref.hpp>

#include <new>
#include <utility>

namespace primer {

//[ primer_lua_ref
class lua_ref {
  /*<< A weak reference to a lua state.
       See `<primer/support/lua_state_ref.hpp>` for details. >>*/
  lua_state_ref sref_;
  /*<< Holds the registry index to the object. Mutable because, if `sref_`
       becomes empty, we want to set `iref_` to `LUA_NOREF` immediately. >>*/
  mutable int iref_ = LUA_NOREF;

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
  // Note: Technically, memory allocation failure can occur in luaL_ref.
  void init(lua_State * L) {
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

  // Release the *ref* but not the lua state ref necessarily.

  // Release the ref if we are in an engaged state. Ends in empty state.
  void release() noexcept {
    if (lua_State * L = this->check_engaged()) {
      luaL_unref(L, LUA_REGISTRYINDEX, iref_);
    }
    this->set_empty();
  }

  // Pilfer the resources of another `lua_ref`.
  void move(lua_ref & other) noexcept {
    sref_ = std::move(other.sref_);
    iref_ = other.iref_;
    other.iref_ = LUA_NOREF;
    other.set_empty();
  }

  //->
public:
  // Special member functions
  lua_ref() noexcept = default;
  lua_ref(lua_ref && other) noexcept;
  /*<< If lua can't allocate memory for the copy, throws `std::bad_alloc`. >>*/
  lua_ref(const lua_ref & other);
  ~lua_ref() noexcept;

  lua_ref & operator=(const lua_ref & other);
  lua_ref & operator=(lua_ref && other) noexcept;

  // Primary constructor
  /*<< Pops an object from the top of given stack, and binds to it. If no
       object is on top, enters the empty state.

Note: This can cause a lua memory allocation failure.
  >>*/
  explicit lua_ref(lua_State * L);

  // Reset to empty state
  /*<< Releases the lua reference, reverts to empty state. >>*/
  void reset() noexcept;

  // Standard swap function
  void swap(lua_ref & other) noexcept;

  // Attempt to obtain a pointer to the underlying lua VM.
  // Succeeds if it wasn't destroyed yet.
  // Always returns a pointer to the *main thread* stack.
  /*<< Returns a valid lua_State * if successfully locked. Nullptr if not.
       No-fail. >>*/
  lua_State * lock() const noexcept;

  // Push to the main stack
  // Return value is same as lock()
  /*<< Note: Does not check for stack space

This cannot cause lua memory allocation error.>>*/
  lua_State * push() const noexcept;

  // Push to a thread stack (see "coroutines" in the manual)
  /*<< Attempts to push the object onto the top of a given ['thread stack].
It *must* be a thread in the same VM as the original stack, or the same
as the original stack.

Returns `true` if push was successful.

Returns `false` and pushes `nil` to
the given stack if the original VM is gone.

[caution If you try to push onto a stack belonging to *a different* lua VM,
undefined and unspecified behavior will result.

If `PRIMER_DEBUG` is defined, then
primer will check for this and call `std::abort` if an error was made.

If `PRIMER_DEBUG` is not defined... very bad things are
likely to happen, including stack corruption of lua VMs.] >>*/
  bool push(lua_State * T) const noexcept;

  // test validity
  /*<< Test if we can still be locked. >>*/
  explicit operator bool() const noexcept;

  // Try to interpret the value as a specific C++ type
  /*<< Attempt to cast the lua value to a C++ value, using primer::read

This cannot cause an exception or raise a lua error.
  >>*/
  template <typename T>
  expected<T> as() const noexcept;
};
//]

inline lua_ref::lua_ref(lua_State * L) { this->init(L); }

inline lua_ref::lua_ref(lua_ref && other) noexcept { this->move(other); }

// Note: Copy ctor used to be really simple: `this->init(other.push())`.
//       However that has a really nasty problem of causing lua memory
//       allocation failure, and it's really not very practical to require the
//       user to create a protected context for any, accidental, copy.
inline lua_ref::lua_ref(const lua_ref & other) {
  if (lua_State * L = other.push()) {
    // Protect against memory failure in `luaL_ref`.
    auto ok = primer::mem_pcall<1>(L, [this, L]() { init(L); });
#ifdef PRIMER_NO_EXCEPTIONS
    static_cast<void>(ok); // I don't see what else we can do here
                           // We could assert(false) I suppose.
#else
    if (!ok) { throw std::bad_alloc{}; }
#endif
  }
}

inline lua_ref::~lua_ref() noexcept { this->release(); }

inline lua_ref & lua_ref::operator=(const lua_ref & other) {
  lua_ref temp{other};
  *this = std::move(temp);
  return *this;
}

inline lua_ref & lua_ref::operator=(lua_ref && other) noexcept {
  this->release();
  this->move(other);
  return *this;
}

inline void lua_ref::reset() noexcept { this->release(); }

inline void lua_ref::swap(lua_ref & other) noexcept {
  sref_.swap(other.sref_);
  std::swap(iref_, other.iref_);
}


inline lua_State * lua_ref::lock() const noexcept {
  return this->check_engaged();
}


inline lua_State * lua_ref::push() const noexcept {
  if (lua_State * L = this->check_engaged()) {
    lua_rawgeti(L, LUA_REGISTRYINDEX, iref_);
    return L;
  }
  return nullptr;
}


inline bool lua_ref::push(lua_State * T) const noexcept {
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


inline lua_ref::operator bool() const noexcept {
  return static_cast<bool>(this->check_engaged());
}


inline void swap(lua_ref & one, lua_ref & other) noexcept { one.swap(other); }

} // end namespace primer
