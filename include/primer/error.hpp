//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * Primer error is a simple exception type used by the library. It is not thrown
 * to user generated code, it is only used internally.
 *
 * It contains a standard string with the error message.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/support/str_cat.hpp>

#include <exception>
#include <string>
#include <utility>

namespace primer {

class error final : public std::exception {
  std::string msg_;

public:
  error() noexcept = default;
  error(const error &) = default;
  error(error &&) noexcept = default;
  error & operator=(const error &) = default;
  error & operator=(error &&) noexcept = default;
  ~error() noexcept = default;

  // Helper constructor
  template <typename... Args>
  explicit error(Args &&... args)
    : msg_(primer::detail::str_cat(std::forward<Args>(args)...))
  {}

  // Help to give context to errors
  error & prepend_error_line(const std::string & line) {
    msg_ = line + "\n" + msg_;
    return *this;
  }

  const std::string & str() const noexcept { return msg_; }

  virtual const char * what() const throw() override {
    return this->str().c_str();
  }
};

} // end namespace primer
