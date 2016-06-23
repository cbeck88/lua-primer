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
  error() = default;
  error(const error &) = default;
  error(error &&) = default;
  error & operator=(const error &) = default;
  error & operator=(error &&) = default;
  ~error() = default;

  // Helper constructor
  template <typename... Args>
  explicit error(Args &&... args)
    : msg_(primer::detail::str_cat(std::forward<Args>(args)...))
  {}


  const std::string & str() const { return msg_; }

  virtual const char * what() const throw() override { return str().c_str(); }
};

} // end namespace primer
