//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * In order to safely use lua, the user must use `lua_checkstack` to check that
 * there is enough stack space remaining to perform the desired operations.
 *
 * Lua does not do these checks internally because it would cause overhead in
 * every operation. They prefer to leave it up to the user.
 *
 * We also would prefer to leave it up to the user. However, it can be complex
 * to estimate how much stack space is needed for some complicated container
 * type or structure.
 *
 * To help with that, we provide a facility to estimate at compile time how much
 * stack space it will cost to read or write a given value.
 *
 * The tricky part is that we don't require the user to annotate their types
 * with this -- it may be variable or impossible to estimate.
 *
 * Therefore, we need a literal type which represents eithter an integer or
 * "unknown". We could use `std::optional`, which is `constexpr` in C++17, but
 * we assume only C++11.
 *
 * Therefore we just throw something suitable together.
 */

#include <type_traits>
#include <utility>

namespace primer {
namespace detail {

// Poor man's constexpr optional<int>
struct maybe_number {
  int value;
  bool unknown;

  constexpr maybe_number() noexcept : value(0), unknown(true) {}
  constexpr maybe_number(maybe_number &&) noexcept = default;
  constexpr maybe_number(const maybe_number &) noexcept = default;
  maybe_number & operator = (const maybe_number &) noexcept = default;
  maybe_number & operator = (maybe_number &&) noexcept = default;

  constexpr explicit maybe_number(int v) noexcept : value(v), unknown(false) {}

  constexpr int operator *() const noexcept { return value; }
  constexpr explicit operator bool() const noexcept { return !unknown; }

  // Right associate
  template <typename F>
  static constexpr maybe_number right_associate(F &&, maybe_number a) {
    return a;
  }

  template <typename F, typename... Args>
  static constexpr maybe_number right_associate(F && f, maybe_number a, Args && ... args) {
    return std::forward<F>(f)(a, right_associate(std::forward<F>(f), std::forward<Args>(args)...));
  }

  // Lift
  template <typename F>
  struct lifted {
    F f;
    constexpr maybe_number operator()(maybe_number a, maybe_number b) const noexcept {
      return (a && b) ? maybe_number{f(*a, *b)} : maybe_number{};
    }
  };

  template <typename F>
  static constexpr auto lift(F && f) -> lifted<F> { return lifted<F>{std::forward<F>(f)}; }

  // Liftees
  static constexpr int add_int(int a, int b) { return a + b; }
  static constexpr int max_int(int a, int b) { return a > b ? a : b; }
  static constexpr int min_int(int a, int b) { return a < b ? a : b; }

  // Add
    template<typename... Args>
  static constexpr maybe_number add(Args && ... args) noexcept {
    return right_associate(lift(add_int), std::forward<Args>(args)...);
  }

  // Max
  template<typename... Args>
  static constexpr maybe_number max(Args && ... args) noexcept {
    return right_associate(lift(max_int), std::forward<Args>(args)...);
  }

  // Min
  template <typename... Args>
  static constexpr maybe_number min(Args && ... args) noexcept {
    return right_associate(lift(min_int), std::forward<Args>(args)...);
  }

  // For convenience
  constexpr maybe_number operator + (int i) const {
    return maybe_number::add(*this, maybe_number{i});
  }

  constexpr maybe_number operator + (maybe_number i) const {
    return maybe_number::add(*this, i);
  }
};

constexpr inline maybe_number operator +(int a, maybe_number b) {
  return b + a;
}

/***
 * Trait which grabs from a structure the value "stack_space_needed", as a `maybe_number`.
 * If there is no such member, the trait returns "unknown".
 */

template <typename T, typename ENABLE = void>
struct stack_space_needed {
  static constexpr maybe_number value{};
};

template <typename T>
struct stack_space_needed<T, typename std::enable_if<std::is_same<decltype(T::stack_space_needed), decltype(T::stack_space_needed)>::value>::type> {
  static constexpr maybe_number value{T::stack_space_needed};
};

} // end namespace detail
} // end namespace primer
