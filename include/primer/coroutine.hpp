//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * primer::coroutine is a safe and convenient wrapper over lua's coroutines.
 *
 * The idea is to abstract the difference between an uncalled coroutine and a
 * yielded coroutine, and make calling it have the same interface as a bound
 * function.
 *
 * Coroutines are either "valid to call" or "invalid" to call. You can tell by
 * testing it using operator bool.
 *
 * It is called using methods `call_no_ret`, `call_one_ret` or `call`.
 * This boils down to a `lua_resume` call. You can pass it a variable number of
 * `pushables` or pass it a `lua_ref_seq` as arguments to the call.
 *
 * Calling the coroutine object will not raise a lua error or throw an
 * exception.
 *
 * It can only be constructed from a primer::bound_function.
 *
 * If a coroutine returns, or raises an error, then the coroutine object will
 * become invalid to call. A new coroutine can be made from the bound_function.
 *
 * Not everything that you can do with coroutines can be done with this object.
 * If you need fine-grained control then you should do it manually using the C
 * api.
 *
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/lua.hpp>
#include <primer/bound_function.hpp>
#include <primer/expected.hpp>
#include <primer/lua_ref.hpp>
#include <primer/lua_ref_seq.hpp>
#include <primer/support/function.hpp>
#include <primer/support/function_check_stack.hpp>
#include <primer/support/cpp_pcall.hpp>

namespace primer {

//[ primer_coroutine
class coroutine {

  lua_ref ref_;
  lua_State * thread_stack_;

  //<-

  // Takes one of the structures `detail::return_none`, `detail::return_one`,
  // `detail::return_many` as first parameter, and all args as second.
  // Pushes, calls, and pops them.
  // TODO: Would fix some leakage here if it passes the return type reference
  // down the callstack, even if its less "functional".
  template <typename return_type, typename... Args>
  static void call_impl(return_type & ret, lua_State * L, Args &&... args) {
    primer::push_each(L, std::forward<Args>(args)...);
    detail::resume_call(ret, L, sizeof...(Args));
  }


  // Takes one of the structures `detail::return_none`, `detail::return_one`,
  // `detail::return_many` as first parameter
  template <typename return_type, typename... Args>
  expected<return_type> protected_call(Args &&... args) noexcept {
    expected<return_type> result{primer::error{"Invalid coroutine"}};

    if (thread_stack_) {
      if (lua_State * L = ref_.lock()) {
        if (auto check = detail::check_stack_push_each<Args...>(thread_stack_)) {
          auto ok = primer::mem_pcall(L, &call_impl<expected<return_type>, Args...>,
                            result, thread_stack_, std::forward<Args>(args)...);
          if (!ok) { result = std::move(ok.err()); }

          if (lua_status(thread_stack_) != LUA_YIELD) { this->reset(); }
        } else {
          result = std::move(check.err());
        }
      } else {
        result = primer::error{"Can't lock VM"};
        thread_stack_ = nullptr;
      }
    }

    return result;
  }

  // Another version, using `lua_ref_seq` as input instead of a parameter pack.
  template <typename return_type>
  static void call_impl2(return_type & ret,
                         lua_State * L,
                         const lua_ref_seq & inputs) {
    inputs.push_each(L);
    detail::resume_call(ret, L, inputs.size());
  }

  // Calls the call_impl2 in a protected context. This is no fail.
  template <typename return_type>
  expected<return_type> protected_call2(const lua_ref_seq & inputs) noexcept {
    expected<return_type> result{primer::error{"Invalid coroutine"}};
    if (thread_stack_) {
      if (lua_State * L = ref_.lock()) {
        if (auto check = detail::check_stack_push_n(thread_stack_, inputs.size())) {
          auto ok = primer::mem_pcall(L, &call_impl2<expected<return_type>>, result,
                            thread_stack_, inputs);
          if (!ok) { result = ok.err(); }
        } else {
          result = std::move(check.err());
        }
      } else {
        result = primer::error{"Can't lock VM"};
        thread_stack_ = nullptr;
      }
    }
    return result;
  }


  //->
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
  // Note: Can cause lua memory allocation failure
  explicit coroutine(const bound_function & bf) //
    : coroutine()                               //
  {
    if (lua_State * L = bf.push()) {
      thread_stack_ = lua_newthread(L);
      lua_insert(L, -2);              // put the thread below the function
      lua_xmove(L, thread_stack_, 1); // Move function to thread stack
      ref_ = lua_ref(L);              // Get ref to the thread
    }
  }

  /*<< Check if the coroutine is valid to call >>*/
  explicit operator bool() const noexcept { return thread_stack_ && ref_; }

  /*<< Reset to the empty state >>*/
  void reset() noexcept {
    ref_.reset();
    thread_stack_ = nullptr;
  }

  void swap(coroutine & other) noexcept {
    ref_.swap(other.ref_);
    std::swap(thread_stack_, other.thread_stack_);
  }

  /*<< Calls or resumes the coroutine, discards return values.
       (But not errors.)>>*/
  template <typename... Args>
  expected<void> call_no_ret(Args &&... args) noexcept {
    return this->protected_call<void>(std::forward<Args>(args)...);
  }

  /*<< Calls or resumes the coroutine, returns one value, or an error >>*/
  template <typename... Args>
  expected<lua_ref> call_one_ret(Args &&... args) noexcept {
    return this->protected_call<lua_ref>(std::forward<Args>(args)...);
  }

  /*<< Calls or resumes the coroutine, returns all return values, or an error
    >>*/
  template <typename... Args>
  expected<lua_ref_seq> call(Args &&... args) noexcept {
    return this->protected_call<lua_ref_seq>(std::forward<Args>(args)...);
  }

// Same thing but using lua_ref_seq as argument
// Use a macro so that we can get const &, &&, and & qualifiers defined.
#define CALL_REF_SEQ_HELPER(N, T, QUAL)                                        \
  expected<T> N(lua_ref_seq QUAL inputs) noexcept {                            \
    return this->protected_call2<T>(inputs);                                   \
  }

#define CALL_REF_SEQ(N, T)                                                     \
  CALL_REF_SEQ_HELPER(N, T, &)                                                 \
  CALL_REF_SEQ_HELPER(N, T, const &)                                           \
  CALL_REF_SEQ_HELPER(N, T, &&)

  // Actual declarations

  CALL_REF_SEQ(call_no_ret, void)
  CALL_REF_SEQ(call_one_ret, lua_ref)
  CALL_REF_SEQ(call, lua_ref_seq)

#undef CALL_REF_SEQ
#undef CALL_REF_SEQ_HELPER
};
//]

inline void swap(coroutine & one, coroutine & other) noexcept {
  one.swap(other);
}

} // end namespace primer
