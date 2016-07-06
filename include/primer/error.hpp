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

#include <new>
#include <string>
#include <utility>

namespace primer {

//[ primer_error

// Tag used to indicate a bad_alloc error
struct bad_alloc_tag{};

class error {
  std::string msg_;

  void set_bad_alloc_state() noexcept {
    PRIMER_TRY {
      msg_ = "bad_alloc";
    } PRIMER_CATCH(std::bad_alloc &) {
      // no small string optimization ?! O_o
      // try a really small string
      PRIMER_TRY {
        msg_ = "mem";
      } PRIMER_CATCH(std::bad_alloc &) {
        // default ctor is noexcept in C++11
        // so is the move ctor
        msg_ = std::string{};
      }
    }
  }

public:
  // Defaulted special member functions
  error() = default;
  error(const error &) = default;
  error(error &&) = default;
  error & operator=(const error &) = default;
  error & operator=(error &&) = default;
  ~error() = default;

  // Bad alloc constructor
  explicit error(bad_alloc_tag) noexcept {
    this->set_bad_alloc_state();
  }

  // Helper constructor
  /*<< This constructor takes a sequence of strings, string literals, or numbers
      and concatenates them to form the message. The function-try block is
      ommitted when "PRIMER_NO_EXCEPTIONS" is defined, see <primer/base.hpp> >>*/
  template <typename... Args>
  explicit error(Args &&... args) noexcept PRIMER_TRY
    : msg_(primer::detail::str_cat(std::forward<Args>(args)...))
  {}
#ifndef PRIMER_NO_EXCEPTIONS
  catch (std::bad_alloc &) { this->set_bad_alloc_state(); }
#endif

  // Help to give context to errors
  /*<< This method can be used to give context to errors. It takes a sequence of
      strings and concatenates them on their own line to the front of the error
      message. >>*/
  template <typename... Args>
  error & prepend_error_line(Args &&... args) noexcept {
    PRIMER_TRY {
      msg_ = primer::detail::str_cat(std::forward<Args>(args)...) + "\n" + msg_;
    } PRIMER_CATCH(std::bad_alloc &) {
      this->set_bad_alloc_state();
    }
    return *this;
  }

  // Accessor
  const std::string & str() const noexcept { return msg_; }
};
//]

} // end namespace primer
