//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * How to push optionals like `boost::optional` and `std::optional`.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/expected.hpp>
#include <primer/lua.hpp>
#include <primer/support/asserts.hpp>
#include <primer/traits/push.hpp>
#include <primer/traits/read.hpp>
#include <primer/traits/util.hpp>

#include <utility>
#include <type_traits>

namespace primer {

//[ optional_access
namespace traits {

// This trait is used to tell primer in a basic way how to interact with an
// optional template type.
//
// optional_access<T> should provide
//
// - typedef value_type
// - static const value_type * as_ptr(const T &) noexcept;
// - static T make_empty() noexcept;
// - static T from_value_type(value_type &&) noexcept;
//
// as_ptr should return nullptr if the optional is empty, or a pointer to the
// contained value.
//
// make empty should create an empty optional.
// from_value_type should move-construct an occupied optional.
//
// It must be specialized if your optional does not look like `boost::optional`
// or `std::optional`.
template <typename T, typename ENABLE = void>
struct optional_access {
  using value_type = typename T::value_type;

  static const value_type * as_ptr(const T & t) noexcept {
    return t ? &*t : nullptr;
  }

  static T make_empty() noexcept { return {}; }
  static T from_value_type(value_type && v) { return T{std::move(v)}; }
};

} // end namespace traits
//]

namespace detail {

/***
 * Validate optional_access type definition
 */

template <typename T, typename ENABLE = void>
struct valid_optional_access : std::false_type {};

template <typename T>
struct valid_optional_access<T,
  traits::enable_if_t<std::is_same<typename traits::optional_access<T>::value_type, decltype(traits::optional_access<T>::make_empty())>::value
                    && std::is_same<typename traits::optional_access<T>::value_type, decltype(traits::optional_access<T>::from_value_type(std::declval<typename traits::optional_access<T>::value_type>()))>::value
#ifndef PRIMER_NO_STATIC_ASSERTS
                    // && noexcept(traits::optional_access<T>::as_ptr(*static_cast<const T *>(nullptr)))
                    // && noexcept(traits::optional_access<T>::make_empty())
                    // && noexcept(traits::optional_access<T>::from_base_type(std::declval<typename traits::optional_access<T>::value_type>()))
#endif
>>
  : std::true_type {};


/***
 * How to push an optional
 */
template <typename T>
struct optional_push : traits::optional_access<T> {

  using helper_t = traits::optional_access<T>;
  using value_t = traits::remove_cv_t<typename helper_t::value_type>;

  static void to_stack(lua_State * L, const T & t) {
    if (const typename helper_t::value_type * ptr = helper_t::as_ptr(t)) {
      traits::push<value_t>::to_stack(L, *ptr);
    } else {
      lua_pushnil(L);
    }
  }
};

/***
 * How to read an optional strictly
 */

template <typename T>
struct optional_strict_read : traits::optional_access<T> {
  using helper_t = traits::optional_access<T>;
  using value_t = traits::remove_cv_t<typename helper_t::value_type>;

  static expected<T> from_stack(lua_State * L, int index) {
    if (lua_isnoneornil(L, index)) {
      return helper_t::make_empty();
    } else if (auto result = traits::read<value_t>::from_stack(L, index)) {
      return helper_t::from_value_type(*std::move(result));
    } else {
      return std::move(result).err();
    }
  }
};

/***
 * How to read an optional in a relaxed manner (swallowing errors)
 */

template <typename T>
struct optional_relaxed_read : traits::optional_access<T> {

  using helper_t = traits::optional_access<T>;
  using value_t = traits::remove_cv_t<typename helper_t::value_type>;

  static expected<T> from_stack(lua_State * L, int index) {
    if (auto result = traits::read<value_t>::from_stack(L, index)) {
      return helper_t::from_value_type(*std::move(result));
    } else {
      return helper_t::make_empty();
    }
  }
};

} // end namespace detail

} // end namespace primer
