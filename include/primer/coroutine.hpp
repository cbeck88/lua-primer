//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * A coroutine is a thread of execution in a lua VM. Unlike a function call,
 * a coroutine can "pause", or "yield", see documentation. It can be then be
 * "resumed" and continue execution from the point of pause.
 *
 * When it yields, that is an opportunity to return some "intermediate" values
 * to the caller if desired, and when it is resumed, the caller can pass some
 * more values back to it. This is especially useful for GUI programming and
 * other things where you might want to invoke a dialog to get user input,
 * and pause yourself until the user comes back with an answer. Without the
 * benefit of coroutines, you need to make a polling / dispatch system for that,
 * and it ends up being a lot uglier to write the lua scripts usually.
 *
 * Coroutines can be used just fine from the C api. The primer::coroutine object
 * is a convenient helper for a certain style of coroutine usage.
 *
 * A primer::coroutine is constructed from a bound function. This is initilaizes
 * the actual "thread" object, pushes the function onto the thread stack, and
 * stores a reference to the thread in the registry.
 *
 * The coroutine can be "called", which passes it arguments and transfers
 * control to it, (calls `lua_resume`) whether starting for the first time,
 * or waking up again after a yield.
 *
 * The coroutine wrapper object has an `explicit operator bool()` which returns
 * true if the coroutine is legal to call. (Either, it has been properly
 * initialized and not called yet, or it was called already and it yielded.)
 *
 * The idea of the object is to abstract the difference between an uncalled
 * coroutine
 * and a yielded coroutine, so the interface doesn't let you access `lua_status`
 * of
 * the coroutine. If you need fine-grained control then you might want to do it
 * manually using the C api.
 *
 * Coroutine is also non-copyable. Hypothetically we could use eris to duplicate
 * a yielded coroutine, but it would be complex and potentially expensive, so
 * in current versions, we don't.
 *
 * If the coroutine finishes or reports an error, it is no longer legal to call
 * it.
 * You should construct a new wrapper object if you want to call the function
 * again.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/lua.hpp>
#include <primer/bound_function.hpp>
#include <primer/lua_ref.hpp>
#include <primer/support/function.hpp>

namespace primer {

//[ primer_coroutine
class coroutine {

  lua_ref ref_;
  lua_State * thread_stack_;


public:
  // Special member functions
  coroutine() noexcept : ref_(), thread_stack_(nullptr) {}

  coroutine(coroutine &&) noexcept = default;
  coroutine & operator=(coroutine &&) noexcept = default;
  ~coroutine() noexcept = default;

  // Noncopyable, delete these.
  coroutine(const coroutine &) = delete;
  coroutine & operator=(const coroutine &) = delete;

  // Construct from bound_function
  explicit coroutine(const bound_function & bf) noexcept //
    : coroutine()                                        //
  {
    if (lua_State * L = bf.push()) {
      thread_stack_ = lua_newthread(L);
      lua_insert(L, -2);              // put the thread below the function
      lua_xmove(L, thread_stack_, 1); // Move function to thread stack
      ref_ = lua_ref(L);              // Get ref to the thread
    }
  }


  /*<< Calls or resumes the coroutine, discards return values.
       (But not errors.)>>*/
  template <typename... Args>
  expected<void> call_no_ret(Args &&... args) noexcept {
    expected<void> result;

    if (thread_stack_ && ref_) {
      constexpr auto estimate = primer::stack_space_for_push_each<Args...>();
      if (estimate && !lua_checkstack(thread_stack_, *estimate)) {
        return primer::error("Insufficient stack space: needed ", *estimate);
      }

      int error_code;

      primer::push_each(thread_stack_, std::forward<Args>(args)...);
      std::tie(result, error_code) =
        primer::resume_no_ret(thread_stack_, sizeof...(args));

      if (error_code != LUA_YIELD) { this->reset(); }
    } else {
      result = primer::error("Could not lock lua state");
    }

    return result;
  }

  /*<< Calls or resumes the coroutine, returns one value, or an error >>*/
  template <typename... Args>
  expected<lua_ref> call_one_ret(Args &&... args) noexcept {
    expected<lua_ref> result;

    if (thread_stack_ && ref_) {
      constexpr auto estimate = primer::stack_space_for_push_each<Args...>();
      if (estimate && !lua_checkstack(thread_stack_, *estimate)) {
        return primer::error("Insufficient stack space: needed ", *estimate);
      }


      int error_code;

      primer::push_each(thread_stack_, std::forward<Args>(args)...);
      std::tie(result, error_code) =
        primer::resume_one_ret(thread_stack_, sizeof...(args));

      if (error_code != LUA_YIELD) { this->reset(); }
    } else {
      result = primer::error("Could not lock lua state");
    }

    return result;
  }

  /*<< Check if the coroutine is valid to call >>*/
  explicit operator bool() const noexcept { return thread_stack_ && ref_; }

  /*<< Reset to the empty state >>*/
  void reset() noexcept {
    ref_.reset();
    thread_stack_ = nullptr;
  }
};
//]

} // end namespace primer
