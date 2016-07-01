//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * Primer error is a simple error type used by the library. It is not an
 * exception, and it is not thrown.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/detail/str_cat.hpp>

#include <string>
#include <utility>

namespace primer {

//[ primer_error
class error {
  std::string msg_;

public:
  // Defaulted special member functions
  error() = default;
  error(const error &) = default;
  error(error &&) = default;
  error & operator=(const error &) = default;
  error & operator=(error &&) = default;
  ~error() = default;

  // Helper constructor
  template <typename... Args>
  explicit error(Args &&... args) /*< This constructor takes a sequence of
                                     strings or string literals and concatenates
                                     them to form the message. >*/
    : msg_(primer::detail::str_cat(std::forward<Args>(args)...)) {}

  // Help to give context to errors
  template <typename... Args>
  error & prepend_error_line(Args &&... args) /*< This method can be used to
      give context to errors. It takes a sequence of strings and concatenates
      them on their own line to the front of the error message. >*/
  {
    msg_ = primer::detail::str_cat(std::forward<Args>(args)...) + "\n" + msg_;
    return *this;
  }

  // Accessor
  const std::string & str() const noexcept { return msg_; }
};
//]

} // end namespace primer
