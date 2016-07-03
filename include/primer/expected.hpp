//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/expected_fwd.hpp>
#include <primer/error.hpp>
#include <primer/support/asserts.hpp>

#include <new>
#include <string>
#include <type_traits>
#include <utility>

/***
 * A class for managing errors without throwing exceptions.
 * The idea is, expected<T> represents either an instance of T, or an error
 * message explaining why T could not be produced.
 * Based (loosely) on a talk by Andrei Alexandrescu
 * https://channel9.msdn.com/Shows/Going+Deep/C-and-Beyond-2012-Andrei-Alexandrescu-Systematic-Error-Handling-in-C
 */

//[ primer_bad_access
// This assertion is only active when PRIMER_DEBUG is defined.
// See <primer/support/asserts.hpp>
#define PRIMER_BAD_ACCESS(X)                                                   \
  PRIMER_ASSERT((X), "Bad access to primer::expected")
//]

namespace primer {

//[ primer_expected
// Tag used in tag dispactch
struct default_construct_in_place_tag {};

// Expected class template
template <typename T>
class expected {
  union {
    T ham_;
    error spam_;
  };
  bool have_ham_;

  static_assert(std::is_nothrow_move_constructible<T>::value,
                "This class can only be used with types that are no-throw "
                "move constructible and destructible.");

public:
  // Accessors and dereference semantics
  explicit operator bool() const noexcept { return have_ham_; }

  T & operator*() & {
    PRIMER_BAD_ACCESS(have_ham_);
    return ham_;
  }

  const T & operator*() const & {
    PRIMER_BAD_ACCESS(have_ham_);
    return ham_;
  }

  T && operator*() && {
    PRIMER_BAD_ACCESS(have_ham_);
    return std::move(ham_);
  }

  T * operator->() & {
    PRIMER_BAD_ACCESS(have_ham_);
    return &ham_;
  }

  const T * operator->() const & {
    PRIMER_BAD_ACCESS(have_ham_);
    return &ham_;
  }

  primer::error & err() & {
    PRIMER_BAD_ACCESS(!have_ham_);
    return spam_;
  }

  const primer::error & err() const & {
    PRIMER_BAD_ACCESS(!have_ham_);
    return spam_;
  }

  primer::error && err() && {
    PRIMER_BAD_ACCESS(!have_ham_);
    return std::move(spam_);
  }

  // Succinct access to error string
  const std::string & err_str() const noexcept { return this->err().str(); }
  const char * err_c_str() const noexcept { return this->err_str().c_str(); }

  //<- Don't put internals in the docu
private:
  // Internal manipulation: deinitialize, init_ham, init_spam

  void deinitialize() noexcept {
    if (have_ham_) {
      ham_.~T();
    } else {
      spam_.~error();
    }
  }

  // Both init_ham and init_spam assume that this is a fresh union or that
  // deiniitalize() was called!
  template <typename... As>
  void init_ham(As &&... as) {
    new (&ham_) T(std::forward<As>(as)...);
    have_ham_ = true;
  }

  template <typename... As>
  void init_spam(As &&... as) {
    new (&spam_) error(std::forward<As>(as)...);
    have_ham_ = false;
  }


  // Helpers for special member functions: Init from (potentially) another kind
  // of expected.
  template <typename E>
  void init_from_const_ref(const E & e) {
    if (e) {
      this->init_ham(*e);
    } else {
      this->init_spam(e.err());
    }
  }

  template <typename E>
  void init_from_rvalue_ref(E && e) {
    if (e) {
      this->init_ham(std::move(*e));
    } else {
      this->init_spam(std::move(e.err()));
    }
  }

  // Does the actual work. First check if we can avoid deinitializing,
  // and use move assignment operator of T or primer::error.
  // If not then deinitialize and reinitialize.
  template <typename E>
  void move_assign(E && e) {
    if (this->have_ham_) {
      if (e) {
        ham_ = std::move(*e);
        return;
      }
    } else {
      if (!e) {
        spam_ = std::move(e.err());
        return;
      }
    }

    this->deinitialize();
    this->init_from_rvalue_ref(std::move(e));
  }

public:
  //->
  // Special member functions

  expected() noexcept { this->init_spam(primer::error{"uninit"}); }

  ~expected() noexcept { this->deinitialize(); }

  expected(const expected & e) { this->init_from_const_ref(e); }

  expected(expected && e) noexcept { this->init_from_rvalue_ref(std::move(e)); }

  expected & operator=(const expected & e) {
    expected temp{e};
    *this = std::move(temp);
    return *this;
  }

  expected & operator=(expected && e) noexcept {
    this->move_assign(std::move(e));
    return *this;
  }

  // Conversions from other `expected` types

  template <typename U>
  expected(const expected<U> & u) /*< When `U` is convertible to `T`, we allow
                                      converting `expected<U>` to `expected<T>`.
                                    >*/
  {
    this->init_from_const_ref(u);
  }

  template <typename U>
  expected(expected<U> && u) {
    this->init_from_rvalue_ref(std::move(u));
  }

  template <typename U>
  expected & operator=(const expected<U> & u) {
    expected temp{u};
    *this = std::move(temp);
    return *this;
  }

  template <typename U>
  expected & operator=(expected<U> && e) {
    this->move_assign(std::move(e));
    return *this;
  }

  // Additional constructors
  explicit expected(default_construct_in_place_tag) { this->init_ham(); }

  expected(const T & t) { /*< Implicitly construct from T >*/
    this->init_ham(t);
  }

  expected(T && t) noexcept { this->init_ham(std::move(t)); }

  expected(const primer::error & e) /*< Implicitly construct from
                                        `primer::error`. This is to make it so
                                        that we can simply return
                                        `primer::error` from functions that
                                        return `expected<T>`. >*/
  {
    this->init_spam(e);
  }

  expected(primer::error && e) noexcept { this->init_spam(std::move(e)); }
};
//]

//[ primer_expected_ref
//` `expected<T&>` is implemented as a special interface over `expected<T*>`.
template <typename T>
class expected<T &> {
  expected<T *> internal_;

public:
  // Accessors
  explicit operator bool() const noexcept {
    return static_cast<bool>(internal_);
  }

  T & operator*() & { return **internal_; }
  T & operator*() const & { return **internal_; }
  T & operator*() && { return **internal_; }

  T * operator->() & { return *internal_; }
  T * operator->() const & { return *internal_; }

  primer::error & err() & { return internal_.err(); }
  const primer::error & err() const & { return internal_.err(); }
  primer::error && err() && { return std::move(internal_.err()); }

  const std::string & err_str() const noexcept { return this->err().str(); }
  const char * err_c_str() const noexcept { return this->err_str().c_str(); }

  // Defaulted special member functions
  expected() noexcept = default;
  expected(const expected &) = default;
  expected(expected &&) noexcept = default;
  expected & operator=(const expected &) = default;
  expected & operator=(expected &&) noexcept = default;
  ~expected() noexcept = default;

  // Additional ctors
  expected(T & t) noexcept //
    : internal_(&t)        //
  {}

  expected(const primer::error & e)
    : internal_(e)
  {}

  expected(primer::error && e) noexcept //
    : internal_(std::move(e))           //
  {}

  // Don't allow converting from other kinds of expected, since could get a
  // dangling reference.
};
//]

//[ primer_expected_void
template <>
class expected<void> {
  bool no_error_;
  primer::error error_;

public:
  // Accessors
  explicit operator bool() const noexcept { return no_error_; }

  const primer::error & err() const & noexcept {
    PRIMER_BAD_ACCESS(!no_error_);
    return error_;
  }
  primer::error && error() && noexcept {
    PRIMER_BAD_ACCESS(!no_error_);
    return std::move(error_);
  }

  const std::string & err_str() const noexcept { return this->err().str(); }
  const char * err_c_str() const noexcept { return this->err_str().c_str(); }


  // Special member functions
  expected() noexcept : no_error_(true),
                        error_("uninit")
  // ^ note the difference! default constructing this means "no error",
  //   for other expected types it means error
  {}

  expected(const expected &) = default;
  expected(expected &&) noexcept = default;
  expected & operator=(const expected &) = default;
  expected & operator=(expected &&) = default;

  // Additional Constructors
  expected(const primer::error & e) /*< Allow implicit conversion from
                                       `primer::error`, so we can return those
                                       from functions that return
                                       `expected<void>`. >*/
    : no_error_(false),
      error_(e) //
  {}

  expected(primer::error && e) noexcept : no_error_(false),
                                          error_(std::move(e))
  // This is noexcept correct as primer::error is no-throw moveable
  {}


  // Conversion from other expected types
  // Note this ASSUMES that the other one has an error! Does not discard a
  // value.
  template <typename U>
  expected(const expected<U> & u)
    : expected(u.err())
  {}

  template <typename U>
  expected(expected<U> && u) noexcept //
    : expected(std::move(u.err()))    //
  {}
};
//]

#undef PRIMER_BAD_ACCESS

} // end namespace primer
