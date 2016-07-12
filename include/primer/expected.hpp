//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * A class for managing errors without throwing exceptions.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/expected_fwd.hpp>
#include <primer/error.hpp>
#include <primer/support/asserts.hpp>

#include <new>
#include <string>
#include <type_traits>
#include <utility>

// This assertion is only active when PRIMER_DEBUG is defined.
// See <primer/support/asserts.hpp>
#define PRIMER_BAD_ACCESS(X)                                                   \
  PRIMER_ASSERT((X), "Bad access to primer::expected")

namespace primer {

//[ primer_expected
// Define primary class template
template <typename T>
class expected {
  union {
    T ham_;
    error spam_;
  };
  bool have_ham_;

  PRIMER_STATIC_ASSERT(
    std::is_nothrow_move_constructible<T>::value,
    "This class can only be used with types that are no-throw "
    "move constructible and destructible.");

public:
  // Accessors and dereference semantics
  explicit operator bool() const noexcept { return have_ham_; }

  T & operator*() & noexcept;
  T && operator*() && noexcept;
  const T & operator*() const & noexcept;

  T * operator->() & noexcept;
  const T * operator->() const & noexcept;

  primer::error & err() & noexcept;
  primer::error && err() && noexcept;
  const primer::error & err() const & noexcept;

  // Succinct access to error string
  const char * err_c_str() const noexcept { return this->err().what(); }
  std::string err_str() const { return this->err_c_str(); }

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

  // expected<T> is only default constructible if T is, and it throws if T does
  expected();

  expected(expected && e) noexcept;
  expected & operator=(expected && e) noexcept;

  expected(const expected & e);
  expected & operator=(const expected & e);

  ~expected() noexcept;

  // Conversion from T
  expected(const T & t);
  expected(T && t) noexcept;

  // Conversion from `primer::error`.
  // This is to make it so that we can simply return `primer::error` from
  // functions that return `expected<T>`.
  expected(const primer::error & e);
  expected(primer::error && e) noexcept;

  // Conversions to other `expected` types
  // This allows you to explicitly convert to expected<U> as long as U is
  // constructible from T, without unpacking and repacking the expected.
  template <typename U>
  expected<U> convert() const &;

  template <typename U>
  expected<U> convert()&&;
};
//]

// Implementation details, outside doc synopsis

// Accessors
template <typename T>
  T & expected<T>::operator*() & noexcept {
  PRIMER_BAD_ACCESS(have_ham_);
  return ham_;
}

template <typename T>
T const & expected<T>::operator*() const & noexcept {
  PRIMER_BAD_ACCESS(have_ham_);
  return ham_;
}

template <typename T>
  T && expected<T>::operator*() && noexcept {
  PRIMER_BAD_ACCESS(have_ham_);
  return std::move(ham_);
}

template <typename T>
  T * expected<T>::operator->() &
  noexcept {
  PRIMER_BAD_ACCESS(have_ham_);
  return &ham_;
}

template <typename T>
const T * expected<T>::operator->() const & noexcept {
  PRIMER_BAD_ACCESS(have_ham_);
  return &ham_;
}

template <typename T>
  primer::error & expected<T>::err() & noexcept {
  PRIMER_BAD_ACCESS(!have_ham_);
  return spam_;
}

template <typename T>
primer::error const & expected<T>::err() const & noexcept {
  PRIMER_BAD_ACCESS(!have_ham_);
  return spam_;
}

template <typename T>
  primer::error && expected<T>::err() && noexcept {
  PRIMER_BAD_ACCESS(!have_ham_);
  return std::move(spam_);
}


// Special member functions
template <typename T>
expected<T>::expected() {
  this->init_ham();
}

template <typename T>
expected<T>::~expected() noexcept {
  this->deinitialize();
}

template <typename T>
expected<T>::expected(const expected & e) {
  this->init_from_const_ref(e);
}

template <typename T>
expected<T>::expected(expected && e) noexcept {
  this->init_from_rvalue_ref(std::move(e));
}

template <typename T>
expected<T> & expected<T>::operator=(const expected & e) {
  expected temp{e};
  *this = std::move(temp);
  return *this;
}

template <typename T>
expected<T> & expected<T>::operator=(expected && e) noexcept {
  this->move_assign(std::move(e));
  return *this;
}

// Converting from other expected

template <typename T>
template <typename U>
expected<U> expected<T>::convert() const & {
  if (*this) {
    return U(**this);
  } else {
    return this->err();
  }
}

template <typename T>
template <typename U>
expected<U> expected<T>::convert() && {
  if (*this) {
    return U(std::move(**this));
  } else {
    return std::move(this->err());
  }
}

// Converting from T
template <typename T>
expected<T>::expected(const T & t) {
  this->init_ham(t);
}

template <typename T>
expected<T>::expected(T && t) noexcept {
  this->init_ham(std::move(t));
}

// Converting from primer::error
template <typename T>
expected<T>::expected(const primer::error & e) {
  this->init_spam(e);
}

template <typename T>
expected<T>::expected(primer::error && e) noexcept {
  this->init_spam(std::move(e));
}



//[ primer_expected_ref
//` `expected<T&>` is implemented as a special interface over `expected<T*>`.
// Define `T&` specialization
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

  const char * err_c_str() const noexcept { return this->err().what(); }
  std::string err_str() const { return this->err_c_str(); }

  // Not default constructible
  expected() = delete;

  // Defaulted special member functions
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
};
//]

//[ primer_expected_void
// define void specialization
template <>
class expected<void> {
  bool no_error_;
  primer::error error_;

public:
  // Accessors
  explicit operator bool() const noexcept;

  const primer::error & err() const & noexcept;
  primer::error && error() && noexcept;

  const char * err_c_str() const noexcept { return this->err().what(); }
  std::string err_str() const { return this->err_c_str(); }

  // Special member functions
  expected() noexcept;
  expected(const expected &) = default;
  expected(expected &&) noexcept = default;
  expected & operator=(const expected &) = default;
  expected & operator=(expected &&) = default;

  // Additional Constructors
  // Allow implicit conversion from `primer::error`, so we can return those
  // from functions that return `expected<void>`.
  expected(const primer::error & e);
  expected(primer::error && e) noexcept;
};
//]

expected<void>::operator bool() const noexcept { return no_error_; }


const primer::error & expected<void>::err() const & noexcept {
  PRIMER_BAD_ACCESS(!no_error_);
  return error_;
}


primer::error && expected<void>::error() && noexcept {
  PRIMER_BAD_ACCESS(!no_error_);
  return std::move(error_);
}

// Ctors
expected<void>::expected() noexcept : no_error_(true), error_() {}

// Converting from primer::error
expected<void>::expected(const primer::error & e)
  : no_error_(false)
  , error_(e) //
{}

expected<void>::expected(primer::error && e) noexcept //
  : no_error_(false)                                  //
    ,
    error_(std::move(e)) //
{}

#undef PRIMER_BAD_ACCESS

} // end namespace primer
