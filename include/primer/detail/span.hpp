//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * It is sometimes useful to have a 'type erased' object which is otherwise
 * like std::array. I.e. it can be iterated over, but it's size is not
 * necessarily known at compile-time, only the type of the entries.
 * This is usually called a "span", in e.g. the GSL, and is in the process
 * of being standardized.
 *
 * A span does not own what it is pointing at, it is a "slice" of an array.
 * It may often be const, but it does not have to be.
 */

#include <array>

namespace primer {

namespace detail {

template <typename T>
class span {
  T * begin_;
  T * end_;

public:
  constexpr span()
    : begin_(nullptr)
    , end_(nullptr)
  {}

  explicit constexpr span(T * b, T * e)
    : begin_(b)
    , end_(e)
  {}
  explicit constexpr span(T * b, std::size_t n)
    : begin_(b)
    , end_(b + n)
  {}

  template <std::size_t N>
  constexpr span(T(&a)[N])
    : begin_(a)
    , end_(begin_ + N)
  {}

  template <std::size_t N>
  constexpr span(std::array<T, N> & a)
    : begin_(a.data())
    , end_(a.data() + a.size())
  {}

  // Accessors

  constexpr T & operator[](int i) const { return *(begin_ + i); }
  constexpr std::size_t size() const {
    return static_cast<std::size_t>(end_ - begin_);
  }

  // Iterator

  typedef T * iterator;

  constexpr iterator begin() const { return begin_; }
  constexpr iterator end() const { return end_; }
};

} // end namespace detail

} // end namespace primer
