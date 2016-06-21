//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * A lua_weak_ref is a weak reference to a lua_State *.
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
 * lua_weak_ref is not thread safe -- there would be little purpose, as lua is
 * not thread-safe.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/lua.hpp>
#include <primer/support/asserts.hpp>
#include <primer/support/diagnostics.hpp>

#include <memory>

struct lua_State;

namespace primer {

class lua_weak_ref {

  // Pod type pointed to be shared_ptr. It's a bit inconvenient to make it point
  // to a raw pointer.
  struct ptr_pod {
    lua_State * ptr_;

    explicit ptr_pod(lua_State * L)
      : ptr_(L)
    {}
  };

  using weak_ptr_type = std::weak_ptr<const ptr_pod>;

  weak_ptr_type weak_ptr_;

  using strong_ptr_type = std::shared_ptr<ptr_pod>;

public:
  explicit lua_weak_ref(const weak_ptr_type & _ptr)
    : weak_ptr_(_ptr)
  {}

  // Default all special member functions
  lua_weak_ref() = default;
  lua_weak_ref(const lua_weak_ref &) = default;
  lua_weak_ref(lua_weak_ref &&) = default;
  lua_weak_ref & operator=(const lua_weak_ref &) = default;
  lua_weak_ref & operator=(lua_weak_ref &&) = default;
  ~lua_weak_ref() = default;

  // Access the pointed state if possible
  lua_State * lock() const noexcept {
    if (auto result = weak_ptr_.lock()) { return result->ptr_; }
    return nullptr;
  }

  explicit operator bool() const noexcept { return this->lock(); }

  // Obtain a weak ref to the given lua state.
  // This works by installing a strong ref at a special registry key.
  // If the strong ref is not found, it is lazily created.
  // The strong ref is destroyed in its __gc metamethod, or, it can be
  // explicitly
  // destroyed by calling "close_weak_refs".
  static lua_weak_ref obtain_weak_ref_to_state(lua_State * L) {
    PRIMER_ASSERT_STACK_NEUTRAL(L);
    lua_weak_ref result{std::shared_ptr<const ptr_pod>{get_strong_ptr(L)}};
    lua_pop(L, 1);
    return result;
  }

  // Close the strong ref which is found in the registry.
  // Post condition: Any weak refs to this lua state wlil return nullptr when
  // locked.
  // Any attempts to obtain_weak_ref from this lua_State will result in a dead
  // weak_ref.
  // Even if no strong ref currently exists, a dead one will be created to
  // prevent future obtain_weak_ref calls from installing one.
  //
  // N.B. You do not need to call this explicitly, unless you are paranoid about
  // bad things happening *during* the lua garbage collection process. Closing
  // the state closes all objects which are marked with finalizers, so the weak
  // refs will be closed by that, during the course of lua_close execution.
  static void close_weak_refs_to_state(lua_State * L) {
    PRIMER_ASSERT_STACK_NEUTRAL(L);
    get_strong_ptr(L).reset();
    lua_pop(L, 1);
  }

private:
  // Get the strong ptr from the registry and push it onto the stack.
  // Also, get the userdata ptr thereof and return a reference.
  // If the strong ptr does not exist yet, then create it.
  static strong_ptr_type & get_strong_ptr(lua_State * L) {
    const char * key = get_strong_ptr_key();
    if (LUA_TUSERDATA != lua_getfield(L, LUA_REGISTRYINDEX, key)) {
      lua_pop(L, 1); // Clear the bad result, create a strong_ptr_type userdata.
      new (lua_newuserdata(L, sizeof(strong_ptr_type)))
        strong_ptr_type{std::make_shared<ptr_pod>(L)};

      lua_newtable(L); // Create the metatable
      lua_pushcfunction(L, &strong_ptr_gc);
      lua_setfield(L, -2, "__gc");
      lua_pushstring(L, key);
      lua_setfield(L, -2, "__metatable");
      lua_setmetatable(L, -2);

      lua_pushvalue(L, -1); // Duplicate the object
      lua_setfield(L, LUA_REGISTRYINDEX, key);
    }
    void * result = lua_touserdata(L, -1);
    PRIMER_ASSERT(result, "Failed to obtain strong ptr: got "
                            << describe_lua_value(L, -1));
    return *static_cast<strong_ptr_type *>(result);
  }

  static const char * get_strong_ptr_key() {
    return "primer_strong_ptr_reg_key";
  }

  static int strong_ptr_gc(lua_State * L) {
    PRIMER_ASSERT(lua_isuserdata(L, 1),
                  "strong_ptr_gc called with argument that is not userdata");
    strong_ptr_type * ptr = static_cast<strong_ptr_type *>(lua_touserdata(L, 1));

#ifdef PRIMER_DEBUG
    if (auto lock = *ptr) {
      PRIMER_ASSERT(L == lock->ptr_,
                    "lua_weak_ref: strong pointer was somehow corrupted");
    }
#endif

    ptr->~strong_ptr_type();
    return 0;
  }
};

// Forward facing interface

inline lua_weak_ref obtain_weak_ref(lua_State * L) {
  return lua_weak_ref::obtain_weak_ref_to_state(L);
}

inline void close_weak_refs(lua_State * L) {
  lua_weak_ref::close_weak_refs_to_state(L);
}

} // end namespace primer
