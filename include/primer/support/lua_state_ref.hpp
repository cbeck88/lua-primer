//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * A lua_state_ref is a weak reference to a lua_State *.
 *
 * It is not like std::weak_ptr, in that it does not lock and become an owning
 * pointer.
 * Instead, either it locks and yields a lua_State *, or it produces nullptr,
 * indicating that the lua state is gone.
 *
 * The purpose of this is that you might want to be able to create references to
 * objects that are in the lua state, but the reference might outlive the lua
 * state and they need to be able to detect this. For example, if there is an
 * existing GUI framework, and you want to be able to implement callbacks as
 * lua functions. The GUI framework may need to be able to recieve some sort of
 * C++ delegate object, but you may not really know when the GUI will delete
 * those delegates and you don't want them to be able to take ownership of the
 * lua state.
 *
 * lua_state_ref is not thread safe -- there would be little purpose, as lua is
 * not thread-safe.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/lua.hpp>
#include <primer/push_singleton.hpp>
#include <primer/support/asserts.hpp>
#include <primer/support/diagnostics.hpp>
#include <primer/support/main_thread.hpp>

#include <nonstd/weak_ref.hpp>

#include <new>
#include <utility>

namespace primer {

class lua_state_ref {

  using weak_ptr_type = nonstd::weak_ref<lua_State>;
  using strong_ptr_type = nonstd::master_ref<lua_State>;

  weak_ptr_type weak_ptr_;

public:
  explicit lua_state_ref(const weak_ptr_type & _ptr) noexcept //
    : weak_ptr_(_ptr)                                         //
  {}

  explicit lua_state_ref(weak_ptr_type && _ptr) noexcept //
    : weak_ptr_(std::move(_ptr))                         //
  {}

  // Default all special member functions
  lua_state_ref() noexcept = default;
  lua_state_ref(const lua_state_ref &) noexcept = default;
  lua_state_ref(lua_state_ref &&) noexcept = default;
  lua_state_ref & operator=(const lua_state_ref &) noexcept = default;
  lua_state_ref & operator=(lua_state_ref &&) noexcept = default;
  ~lua_state_ref() noexcept = default;

  // Swap
  void swap(lua_state_ref & other) noexcept { weak_ptr_.swap(other.weak_ptr_); }

  // Access the pointed state if possible
  lua_State * lock() const noexcept { return weak_ptr_.lock(); }

  // Reset
  void reset() noexcept { weak_ptr_.reset(); }

  // Check validity
  explicit operator bool() const noexcept { return this->lock(); }

  // Obtain a weak ref to the given lua state.
  // This works by installing a strong ref at a special registry key.
  // If the strong ref is not found, it is lazily created.
  // The strong ref is destroyed in its __gc metamethod, or, it can be
  // explicitly destroyed by calling "close_weak_refs".
  static lua_state_ref obtain_weak_ref_to_state(lua_State * L) {
    PRIMER_ASSERT_STACK_NEUTRAL(L);
    weak_ptr_type result{get_strong_ptr(L)};
    lua_pop(L, 1);
    return lua_state_ref{result};
  }

  // Close the strong ref which is found in the registry.
  // Post condition: Any weak refs to this lua state will return nullptr when
  // locked.
  // Any attempts to obtain_weak_ref from this lua_State will result in a dead
  // weak_ref.
  // Even if no strong ref currently exists, a dead one will be created to
  // prevent future obtain_weak_ref calls from installing one.
  //
  // [note You do not need to call this explicitly, unless you are paranoid
  // about
  // bad things happening *during* the lua garbage collection process. Closing
  // the state closes all objects which are marked with finalizers, so the weak
  // refs will be closed by that, during the course of lua_close execution.]
  static void close_weak_refs_to_state(lua_State * L) {
    PRIMER_ASSERT_STACK_NEUTRAL(L);
    get_strong_ptr(L).reset();
    lua_pop(L, 1);
  }

private:
  // Create a strong pointer in a userdata. This object will be cached,
  // and needs to be destroyed when the lua state is destroyed.
  static void make_strong_ptr(lua_State * L) {
    // TODO: Handle std::bad_alloc ... ?
    {
      // Get the *main* thread, in case this is being called from a short-lived
      // thread. We want the strong_ptr to be pointing to something that lives
      // as long as the lua state.
      lua_State * M = primer::main_thread(L);
      PRIMER_ASSERT(M, "The main thread was not a thread (!)");
      new (lua_newuserdata(L, sizeof(strong_ptr_type))) strong_ptr_type{M};
    }

    lua_newtable(L); // Create the metatable
    lua_pushcfunction(L, &strong_ptr_gc);
    lua_setfield(L, -2, "__gc");
    lua_pushliteral(L, "primer strong pointer");
    lua_setfield(L, -2, "__metatable");
    lua_setmetatable(L, -2);
  }

  // gc function for userdata
  static int strong_ptr_gc(lua_State * L) {
    PRIMER_ASSERT(lua_isuserdata(L, 1),
                  "strong_ptr_gc called with argument that is not userdata");
    strong_ptr_type * ptr =
      static_cast<strong_ptr_type *>(lua_touserdata(L, 1));
    ptr->~strong_ptr_type();
    lua_pushnil(L);
    lua_setmetatable(L, -2);
    return 0;
  }

  // Get the strong ptr from the registry and push it onto the stack.
  // Also, get the userdata ptr thereof and return a reference.
  // If the strong ptr does not exist yet, then create it.
  static strong_ptr_type & get_strong_ptr(lua_State * L) {
    primer::push_singleton<&make_strong_ptr>(L);
    void * result = lua_touserdata(L, -1);
    PRIMER_ASSERT(result, "Failed to obtain strong ptr: got "
                            << describe_lua_value(L, -1));
    return *static_cast<strong_ptr_type *>(result);
  }
};

inline void
swap(lua_state_ref & one, lua_state_ref & other) noexcept {
  one.swap(other);
}

// Forward facing interface

inline lua_state_ref
obtain_state_ref(lua_State * L) noexcept {
  return lua_state_ref::obtain_weak_ref_to_state(L);
}

inline void
close_state_refs(lua_State * L) noexcept {
  lua_state_ref::close_weak_refs_to_state(L);
}

} // end namespace primer
