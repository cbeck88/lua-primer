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

/*` It is implemented using a union. Sometimes it refers to a fixed static
    string, and sometimes it holds a `std::string` which contains the error
    message.

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

  class impl {
    enum class state { uninitialized, bad_alloc, cant_lock_vm, invalid_coroutine, dynamic_text };

    using string_t = std::string;


    string_t str_;
    state state_;

    // Helpers
    template <typename T>
    void initialize_string(T && t) {
      state_ = state::dynamic_text;
      str_ = std::move(t);
    }
 

public:
    impl() : str_(), state_(state::uninitialized) {}
    impl(impl && other) = default;
    impl(const impl & other) = default;
    impl & operator = (impl && other) = default;
    impl & operator = (const impl & other) = default;
    ~impl() = default;

    // Construct with fixed error messages
    struct bad_alloc_tag{ static constexpr state value = state::bad_alloc; };
    struct cant_lock_vm_tag{ static constexpr state value = state::cant_lock_vm; };
    struct invalid_coroutine_tag{ static constexpr state value = state::invalid_coroutine; };

    template <typename T, typename = decltype(T::value)>
    explicit impl(T) noexcept : str_(), state_(T::value) {}

    // Construct from string
    explicit impl(std::string s) noexcept : impl() {
      this->initialize_string(std::move(s));
    }

    // Access error message
    const char * c_str() const {
      switch (state_) {
        case state::uninitialized:
          return "uninitialized error message";
        case state::bad_alloc:
          return "bad_alloc";
        case state::cant_lock_vm:
          return "couldn't access the lua VM";
        case state::invalid_coroutine:
          return "invalid coroutine";
        case state::dynamic_text:
          return str_.c_str();
        default:
          return "invalid error message state";
      }
    }

    // Get reference to dynamic string, or convert it to such.
    std::string & str() {
      if (state_ != state::dynamic_text) {
        this->initialize_string(this->c_str());
      }
      return str_;
    }
  };

  impl msg_;

  explicit error(impl m) : msg_(std::move(m)) {}

  //->
public:
  // Defaulted special member functions
  error() noexcept = default;
  error(const error &) = default;
  error(error &&) noexcept = default;
  error & operator=(const error &) = default;
  error & operator=(error &&) noexcept = default;
  ~error() noexcept = default;

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

inline error error::bad_alloc() noexcept {
  return error{impl{impl::bad_alloc_tag{}}};
}

inline error error::expired_coroutine() noexcept {
  return error{impl{impl::invalid_coroutine_tag{}}};
}

inline error error::cant_lock_vm() noexcept {
  return error{impl{impl::cant_lock_vm_tag{}}};
}

template <typename... Args>
inline error::error(Args &&... args) noexcept {
  PRIMER_TRY_BAD_ALLOC {
    msg_ = impl{primer::detail::str_cat(std::forward<Args>(args)...)};
  }
  PRIMER_CATCH_BAD_ALLOC { msg_ = impl{impl::bad_alloc_tag{}}; }
}

template <typename... Args>
inline error & error::prepend_error_line(Args &&... args) noexcept {
  PRIMER_TRY_BAD_ALLOC {
    msg_.str() = primer::detail::str_cat(std::forward<Args>(args)...) + "\n" + msg_.str();
  }
  PRIMER_CATCH_BAD_ALLOC { /* msg_ = impl{impl::bad_alloc_tag{}}; */ }
  // Note: Assigning bad_alloc there may be detrimental to error report quality
  // prepend error line is used to give context, better to keep the previous
  // error and skip the addition of context than lose all of the original.
  return *this;
}

inline const char * error::what() const noexcept { return msg_.c_str(); }

//[ primer_error_extra_notes

} // end namespace primer

//` The `prepend_error_line` method can be used to add context to an error
//` message as it comes up the callstack. For instance,
//= err.prepend_error_line("In index [", idx, "] of table:");

//]
