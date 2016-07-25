//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * A class for managing errors without throwing exceptions.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/error.hpp>
#include <primer/expected_fwd.hpp>
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
template <typename T, typename E>
class expected {
  union {
    T ham_;
    E spam_;
  };
  bool have_ham_;

  PRIMER_STATIC_ASSERT(
    std::is_nothrow_move_constructible<T>::value,
    "This class can only be used with types that are no-throw "
    "move constructible and destructible.");

  PRIMER_STATIC_ASSERT(
    std::is_nothrow_move_constructible<E>::value,
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

  E & err() & noexcept;
  E && err() && noexcept;
  const E & err() const & noexcept;

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
      spam_.~E();
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
    new (&spam_) E(std::forward<As>(as)...);
    have_ham_ = false;
  }

  // Helpers for special member functions: Init from (potentially) another kind
  // of expected.
  template <typename X>
  void init_from_const_ref(const X & x) {
    if (x) {
      this->init_ham(*x);
    } else {
      this->init_spam(x.err());
    }
  }

  template <typename X>
  void init_from_rvalue_ref(X && x) {
    if (x) {
      this->init_ham(std::move(*x));
    } else {
      this->init_spam(std::move(x.err()));
    }
  }

  // Does the actual work. First check if we can avoid deinitializing,
  // and use move assignment operator of T or primer::error.
  // If not then deinitialize and reinitialize.
  template <typename X>
  void move_assign(X && x) {
    if (this->have_ham_) {
      if (x) {
        ham_ = std::move(*x);
        return;
      }
    } else {
      if (!x) {
        spam_ = std::move(x.err());
        return;
      }
    }

    this->deinitialize();
    this->init_from_rvalue_ref(std::move(x));
  }

public:
  //->
  // Special member functions

  // expected<T> is only default constructible if T is, and it throws if T does
  expected();

  expected(expected &&) noexcept;
  expected & operator=(expected &&) noexcept;

  expected(const expected &);
  expected & operator=(const expected &);

  ~expected() noexcept;

  // Conversion from T
  expected(const T & t);
  expected(T && t) noexcept;

  // Conversion from `E`.
  // This is to make it so that we can simply return `E` from
  // functions that return `expected<T>`.
  expected(const E & e);
  expected(E && e) noexcept;

  // Conversions to other `expected` types
  // This allows you to explicitly convert to expected<U> as long as U is
  // constructible from T. This unpacks and repacks the expected.
  template <typename U, typename EU = E>
  expected<U, EU> convert() const &;

  template <typename U, typename EU = E>
  expected<U, EU> convert() &&;

  // Map function.
  // This is like monadic bind.
  // If we have a value, apply this function to it and return the result.
  // If we have an error, just return the error.
  // If the function returns an expected type, collapse the
  // `expected<expected<...>>` return into a single `expected<...>`.
  template <typename F>
  auto map(F && f) & -> fold_expected_t<decltype(std::forward<F>(f)(
                                          *static_cast<T *>(nullptr))),
                                        E> {
    if (*this) {
      return std::forward<F>(f)(**this);
    } else {
      return this->err();
    }
  }

  template <typename F>
  auto map(F && f)
    const & -> fold_expected_t<decltype(std::forward<F>(f)(
                                 *static_cast<const T *>(nullptr))),
                               E> {
    if (*this) {
      return std::forward<F>(f)(**this);
    } else {
      return this->err();
    }
  }

  template <typename F>
  auto map(
    F &&
      f) && -> fold_expected_t<decltype(std::forward<F>(f)(std::declval<T>())),
                               E> {
    if (*this) {
      return std::forward<F>(f)(std::move(**this));
    } else {
      return std::move(this->err());
    }
  }

  // value_or function. Similar to what it does in std::optional.
  template <typename U>
  T value_or(U && u) const & {
    if (*this) {
      return **this;
    } else {
      return std::forward<U>(u);
    }
  }

  template <typename U>
  T value_or(U && u) && {
    if (*this) {
      return std::move(**this);
    } else {
      return std::forward<U>(u);
    }
  }
};
//]

// Implementation details, outside doc synopsis

// Accessors
template <typename T, typename E>
  T & expected<T, E>::operator*() & noexcept {
  PRIMER_BAD_ACCESS(have_ham_);
  return ham_;
}

template <typename T, typename E>
T const & expected<T, E>::operator*() const & noexcept {
  PRIMER_BAD_ACCESS(have_ham_);
  return ham_;
}

template <typename T, typename E>
  T && expected<T, E>::operator*() && noexcept {
  PRIMER_BAD_ACCESS(have_ham_);
  return std::move(ham_);
}

template <typename T, typename E>
  T * expected<T, E>::operator->()
  & noexcept {
  PRIMER_BAD_ACCESS(have_ham_);
  return &ham_;
}

template <typename T, typename E>
const T * expected<T, E>::operator->() const & noexcept {
  PRIMER_BAD_ACCESS(have_ham_);
  return &ham_;
}

template <typename T, typename E>
  E &
  expected<T, E>::err()
  & noexcept {
  PRIMER_BAD_ACCESS(!have_ham_);
  return spam_;
}

template <typename T, typename E>
E const &
expected<T, E>::err() const & noexcept {
  PRIMER_BAD_ACCESS(!have_ham_);
  return spam_;
}

template <typename T, typename E>
  E &&
  expected<T, E>::err()
  && noexcept {
  PRIMER_BAD_ACCESS(!have_ham_);
  return std::move(spam_);
}

// Special member functions
template <typename T, typename E>
expected<T, E>::expected() {
  this->init_ham();
}

template <typename T, typename E>
expected<T, E>::~expected() noexcept {
  this->deinitialize();
}

template <typename T, typename E>
expected<T, E>::expected(const expected & e) {
  this->init_from_const_ref(e);
}

template <typename T, typename E>
expected<T, E>::expected(expected && e) noexcept {
  this->init_from_rvalue_ref(std::move(e));
}

template <typename T, typename E>
expected<T, E> &
expected<T, E>::operator=(const expected & e) {
  expected temp{e};
  *this = std::move(temp);
  return *this;
}

template <typename T, typename E>
expected<T, E> &
expected<T, E>::operator=(expected && e) noexcept {
  this->move_assign(std::move(e));
  return *this;
}

// Converting from other expected

template <typename T, typename ET>
template <typename U, typename EU>
expected<U, EU>
expected<T, ET>::convert() const & {
  if (*this) {
    return U(**this);
  } else {
    return this->err();
  }
}

template <typename T, typename ET>
template <typename U, typename EU>
expected<U, EU>
expected<T, ET>::convert() && {
  if (*this) {
    return U(std::move(**this));
  } else {
    return EU(std::move(this->err()));
  }
}

// Converting from T
template <typename T, typename E>
expected<T, E>::expected(const T & t) {
  this->init_ham(t);
}

template <typename T, typename E>
expected<T, E>::expected(T && t) noexcept {
  this->init_ham(std::move(t));
}

// Converting from E
template <typename T, typename E>
expected<T, E>::expected(const E & e) {
  this->init_spam(e);
}

template <typename T, typename E>
expected<T, E>::expected(E && e) noexcept {
  this->init_spam(std::move(e));
}

//[ primer_expected_ref
//` `expected<T&>` is implemented as a special interface over `expected<T*>`.
// Define `T&` specialization
template <typename T, typename E>
class expected<T &, E> {
  expected<T *, E> internal_;

  //<-
  PRIMER_STATIC_ASSERT(
    std::is_nothrow_move_constructible<E>::value,
    "This class can only be used with types that are no-throw "
    "move constructible and destructible.");
  //->
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

  E & err() & { return internal_.err(); }
  const E & err() const & { return internal_.err(); }
  E && err() && { return std::move(internal_.err()); }

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

  expected(const E & e)
    : internal_(e) {}

  expected(E && e) noexcept   //
    : internal_(std::move(e)) //
  {}

  // Map function.
  // This is like monadic bind.
  // If we have a value, apply this function to it and return the result.
  // If we have an error, just return the error.
  // If the function returns an expected type, collapse the
  // `expected<expected<...>>` return into a single `expected<...>`.
  template <typename F>
  auto map(F && f) const
    -> fold_expected_t<decltype(std::forward<F>(f)(*static_cast<T *>(nullptr))),
                       E> {
    if (*this) {
      return std::forward<F>(f)(**this);
    } else {
      return this->err();
    }
  }

  // value_or function. Similar to what it does in std::optional.
  template <typename U>
  T & value_or(U && u) const {
    if (*this) {
      return **this;
    } else {
      return static_cast<T &>(std::forward<U>(u));
    }
  }
};
//]

//[ primer_expected_void
// define void specialization
template <typename E>
class expected<void, E> {
  bool no_error_;

  union {
    E error_;
    char dummy_;
  };

  //<-
  PRIMER_STATIC_ASSERT(
    std::is_nothrow_move_constructible<E>::value,
    "This class can only be used with types that are no-throw "
    "move constructible and destructible.");

  void deinitialize() {
    if (!no_error_) {
      error_.~E();
      no_error_ = true;
    }
  }
  void move(expected && o) {
    if (no_error_) {
      if (!o.no_error_) {
        new (&error_) E(std::move(o.error_));
        no_error_ = false;
      }
    } else {
      if (o.no_error_) {
        this->deinitialize();
      } else {
        error_ = std::move(o.error_);
      }
    }
  }
  //->
public:
  // Accessors
  explicit operator bool() const noexcept;

  E & err() & noexcept;
  const E & err() const & noexcept;
  E && err() && noexcept;

  const char * err_c_str() const noexcept { return this->err().what(); }
  std::string err_str() const { return this->err_c_str(); }

  // Default-construct in the "ok" / `true` state
  constexpr expected() noexcept;

  // Special member functions
  expected(const expected &);
  expected(expected &&) noexcept;
  expected & operator=(const expected &);
  expected & operator=(expected &&) noexcept;
  ~expected() noexcept;

  // Additional Constructors
  // Allow implicit conversion from `E`, so that we can return
  // `E` from functions that return `expected<void>`.
  expected(const E & e);
  expected(E && e) noexcept;
};
//]

template <typename E>
expected<void, E>::operator bool() const noexcept {
  return no_error_;
}

template <typename E>
E &
expected<void, E>::err() & noexcept {
  PRIMER_BAD_ACCESS(!no_error_);
  return error_;
}

template <typename E>
const E &
expected<void, E>::err() const & noexcept {
  PRIMER_BAD_ACCESS(!no_error_);
  return error_;
}

template <typename E>
  E &&
  expected<void, E>::err()
  && noexcept {
  PRIMER_BAD_ACCESS(!no_error_);
  return std::move(error_);
}

// Ctors
template <typename E>
constexpr expected<void, E>::expected() noexcept
  : no_error_(true)
  , dummy_() {}

template <typename E>
expected<void, E>::~expected() noexcept {
  this->deinitialize();
}

template <typename E>
expected<void, E>::expected(const expected & o)
  : no_error_(o.no_error_) {
  if (!no_error_) { new (&error_) E{o.error_}; }
}

template <typename E>
expected<void, E>::expected(expected && o) noexcept
  : expected() {
  this->move(std::move(o));
}

template <typename E>
expected<void, E> &
expected<void, E>::operator=(expected && o) noexcept {
  this->move(std::move(o));
  return *this;
}

template <typename E>
expected<void, E> &
expected<void, E>::operator=(const expected & o) {
  expected temp{o};
  this->move(std::move(temp));
  return *this;
}

// Converting from E
template <typename E>
expected<void, E>::expected(const E & e)
  : no_error_(false)
  , error_(e) //
{}

template <typename E>
expected<void, E>::expected(E && e) noexcept //
  : no_error_(false)                         //
    ,
    error_(std::move(e)) //
{}

#undef PRIMER_BAD_ACCESS

} // end namespace primer
