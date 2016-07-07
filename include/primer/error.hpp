//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

//[ primer_error
/*` Primer error is a simple error type used by the library to represent
    run-time error signals.

 */

/*` It is not an exception, and it is not thrown.

*/

/*` It contains a `std::string` which holds the error message.

*/

/*` Primer generally translates lua errors into `primer::error` when it performs
   an
    operation which fails, and will translate `primer::error` into a lua error
   when
    adapting callbacks.

*/

//<- Don't put boiler-plate in the docu

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/detail/str_cat.hpp>

#include <new>
#include <string>
#include <utility>

//->
namespace primer {

//` `primer::error` is used to handle all errors in primer. Since it uses
//` `std::string` internally, it needs some extra support
//` for propagating `std::bad_alloc` signals successfully.
// Tag used to indicate a bad_alloc error
struct bad_alloc_tag {};

//<-
// clang-format off
//->

//` The main class has some helpful constructors so that you can more easily
//` format error messages.
//` For instance, the expression
//= primer::error("Bad doggie, '", dog_name_str, "'! You get ", biscuit_num, "biscuits!")
//` produces a `primer::error` object with error string equal to the result of
//= "Bad doggie, '" + dog_name_str + "'! You get " + std::to_string(biscuit_num) + " biscuits!"
//`
//` Main class definition:

//<-
// clang-format on
//->

class error {
  std::string msg_;

  /*<< This function sets the message for the `bad_alloc` state. >>*/
  //= void set_bad_alloc_state() noexcept;

  //<-
  void set_bad_alloc_state() noexcept {
    PRIMER_TRY { msg_ = "bad_alloc"; }
    PRIMER_CATCH(std::bad_alloc &) {
      // no small string optimization ?! O_o
      // try a really small string
      PRIMER_TRY { msg_ = "mem"; }
      PRIMER_CATCH(std::bad_alloc &) {
        // default ctor is noexcept in C++11
        // so is the move ctor
        msg_ = std::string{};
      }
    }
  }
  //->
public:
  // Defaulted special member functions
  error() = default;
  error(const error &) = default;
  error(error &&) = default;
  error & operator=(const error &) = default;
  error & operator=(error &&) = default;
  ~error() = default;

  // Bad alloc constructor
  /*<< Used to initialize the object in the `std::bad_alloc` state >>*/
  explicit error(bad_alloc_tag) noexcept { this->set_bad_alloc_state(); }

  // Primary constructor
  /*<< This constructor takes a sequence of strings, string literals, or numbers
      and concatenates them to form the message. >>*/
  template <typename... Args>
  explicit error(Args &&... args) noexcept {
    PRIMER_TRY { msg_ = primer::detail::str_cat(std::forward<Args>(args)...); }
    PRIMER_CATCH(std::bad_alloc &) { this->set_bad_alloc_state(); }
  }

  // Help to give context to errors
  /*<< This method takes a sequence of
      strings and concatenates them on their own line to the front of the error
      message. >>*/
  template <typename... Args>
  error & prepend_error_line(Args &&... args) noexcept {
    PRIMER_TRY {
      msg_ = primer::detail::str_cat(std::forward<Args>(args)...) + "\n" + msg_;
    }
    PRIMER_CATCH(std::bad_alloc &) { this->set_bad_alloc_state(); }
    return *this;
  }

  // Accessor
  const std::string & str() const noexcept { return msg_; }
};

//]

//[ primer_error_extra_notes

} // end namespace primer

//` The `prepend_error_line` method can be used to add context to an error
//` message as it comes up the callstack. For instance,
//= err.prepend_error_line("In index [", idx, "] of table:");

//]
