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

/*` Primer generally translates lua errors into `primer::error` when it
   performs an operation which fails, and will translate `primer::error` into
   a lua error when adapting callbacks.

*/

//<- Don't put boiler-plate in the docu

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/detail/str_cat.hpp>

#include <new>
#include <string>
#include <utility>

//->


//<-
// clang-format off
//->

//` The constructor can be used to easily
//` format error messages.
//` For instance,
//= primer::error("Bad doggie, '", dog_name_str, "'! You get ", biscuit_num, "biscuits!")
//` produces a `primer::error` object with error string equal to
//= "Bad doggie, '" + dog_name_str + "'! You get " + std::to_string(biscuit_num) + " biscuits!"
//`
//` Synopsis:

//<-
// clang-format on
//->
namespace primer {

class error {
  //<-
  std::string msg_;
  void set_bad_alloc_state() noexcept;
  //->
public:
  // Defaulted special member functions
  error() = default;
  error(const error &) = default;
  error(error &&) = default;
  error & operator=(const error &) = default;
  error & operator=(error &&) = default;
  ~error() = default;

  // General constructor
  // Takes a sequence of strings, string literals, or numbers
  // and concatenates them to form the message.
  template <typename... Args>
  explicit error(Args &&... args) noexcept;

  // Help to give context to errors
  /*<< This method takes a sequence of
      strings and concatenates them on their own line to the front of the error
      message. >>*/
  template <typename... Args>
  error & prepend_error_line(Args &&... args) noexcept;

  //
  // Preformatted errors
  //

  // Used to indicate an out-of-memory error like `std::bad_alloc`
  static error bad_alloc() noexcept;

  // "Integer overflow occured: X"
  template <typename T>
  static error integer_overflow(const T & t) noexcept;

  // "Insufficient stack space: needed n"
  static error insufficient_stack_space(int n) noexcept;

  // "Expected foo, found 'bar'"
  template <typename T>
  static error unexpected_value(const char * expected, T && found) noexcept;

  // "Can't lock VM".
  /*<< Used with coroutines / bound_functions that are called but the VM could
       not be accessed. >>*/
  static error cant_lock_vm() noexcept;

  // "Expired coroutine"
  /*<< Used with coroutines that are called while in an empty state >>*/
  static error expired_coroutine() noexcept;


  // Accessor
  const char * what() const noexcept;
};

//]

inline error error::bad_alloc() noexcept {
  error result;
  result.set_bad_alloc_state();
  return result;
}

template <typename T>
inline error error::integer_overflow(const T & t) noexcept {
  return error("Integer overflow occurred: ", t);
}

template <typename T>
inline error error::unexpected_value(const char * expected, T && t) noexcept {
  return error("Expected ", expected, " found: '", std::forward<T>(t), "'");
}

inline error error::insufficient_stack_space(int n) noexcept {
  return error("Insufficient stack space: needed ", n);
}

inline error error::expired_coroutine() noexcept {
  return error("Expired coroutine");
}

inline error error::cant_lock_vm() noexcept {
  return error("Can't lock VM");
}

template <typename... Args>
inline error::error(Args &&... args) noexcept {
  PRIMER_TRY_BAD_ALLOC {
    msg_ = primer::detail::str_cat(std::forward<Args>(args)...);
  }
  PRIMER_CATCH_BAD_ALLOC { this->set_bad_alloc_state(); }
}

template <typename... Args>
inline error & error::prepend_error_line(Args &&... args) noexcept {
  PRIMER_TRY_BAD_ALLOC {
    msg_ = primer::detail::str_cat(std::forward<Args>(args)...) + "\n" + msg_;
  }
  PRIMER_CATCH_BAD_ALLOC { this->set_bad_alloc_state(); }
  return *this;
}

inline const char * error::what() const noexcept { return msg_.c_str(); }

inline void error::set_bad_alloc_state() noexcept {
    PRIMER_TRY_BAD_ALLOC { msg_ = "bad_alloc"; }
    PRIMER_CATCH_BAD_ALLOC {
      // no small string optimization ?! O_o
      // try a really small string
      PRIMER_TRY_BAD_ALLOC { msg_ = "mem"; }
      PRIMER_CATCH_BAD_ALLOC {
        // default ctor is noexcept in C++11
        // so is the move ctor
        msg_ = std::string{};
      }
    }
  }

//[ primer_error_extra_notes

} // end namespace primer

//` The `prepend_error_line` method can be used to add context to an error
//` message as it comes up the callstack. For instance,
//= err.prepend_error_line("In index [", idx, "] of table:");

//]
