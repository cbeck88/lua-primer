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
#include <primer/detail/maybe_number.hpp>
#include <primer/support/asserts.hpp>
#include <primer/traits/push.hpp>
#include <primer/traits/read.hpp>
#include <primer/traits/util.hpp>

#include <utility>
#include <type_traits>

namespace primer {

//[ primer_optional_access
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

  PRIMER_STATIC_ASSERT(std::is_nothrow_constructible<T>::value,
                       "optional must be nothrow constructible");

  // TODO: boost::optional doesn't do this...
  // PRIMER_STATIC_ASSERT(noexcept(T{std::declval<value_type>()}),
  //                      "optional must be nothrow constructible from r-value
  //                      reference to value type");


  static const value_type * as_ptr(const T & t) noexcept {
    return t ? &*t : nullptr;
  }

  static T make_empty() noexcept { return {}; }
  static T from_value_type(value_type && v) noexcept { return T{std::move(v)}; }
};

} // end namespace traits
//]

namespace detail {

/***
 * How to push an optional
 */
template <typename T>
struct optional_push {

  using helper_t = traits::optional_access<T>;
  using value_t = traits::remove_cv_t<typename helper_t::value_type>;

  PRIMER_STATIC_ASSERT(
    noexcept(helper_t::as_ptr(*static_cast<const T *>(nullptr))),
    "as_ptr method of optional_access must be noexcept");

  static void to_stack(lua_State * L, const T & t) {
    if (const typename helper_t::value_type * ptr = helper_t::as_ptr(t)) {
      traits::push<value_t>::to_stack(L, *ptr);
    } else {
      lua_pushnil(L);
    }
  }
  static constexpr detail::maybe_number stack_space_needed{detail::stack_space_needed<traits::push<value_t>>::value};
};

/***
 * How to read an optional strictly
 */

template <typename T>
struct optional_strict_read {
  using helper_t = traits::optional_access<T>;
  using value_t = typename helper_t::value_type;

  PRIMER_STATIC_ASSERT(noexcept(helper_t::make_empty()),
                       "make_empty method of optional_access must be noexcept");

  PRIMER_STATIC_ASSERT(
    noexcept(helper_t::from_value_type(std::declval<value_t>())),
    "from_value_type method of optional_access must be noexcept");

  using clean_val_t = traits::remove_cv_t<value_t>;

  static expected<T> from_stack(lua_State * L, int index) {
    if (lua_isnoneornil(L, index)) {
      return helper_t::make_empty();
    } else if (auto result = traits::read<clean_val_t>::from_stack(L, index)) {
      return helper_t::from_value_type(*std::move(result));
    } else {
      return std::move(result).err();
    }
  }
  static constexpr detail::maybe_number stack_space_needed{detail::stack_space_needed<traits::read<clean_val_t>>::value};
};

/***
 * How to read an optional in a relaxed manner (swallowing errors)
 */

template <typename T>
struct optional_relaxed_read : traits::optional_access<T> {

  using helper_t = traits::optional_access<T>;
  using value_t = typename helper_t::value_type;

  PRIMER_STATIC_ASSERT(noexcept(helper_t::make_empty()),
                       "make_empty method of optional_access must be noexcept");

  PRIMER_STATIC_ASSERT(
    noexcept(helper_t::from_value_type(std::declval<value_t>())),
    "from_value_type method of optional_access must be noexcept");

  using clean_val_t = traits::remove_cv_t<value_t>;

  static expected<T> from_stack(lua_State * L, int index) {
    if (auto result = traits::read<clean_val_t>::from_stack(L, index)) {
      return helper_t::from_value_type(*std::move(result));
    } else {
      return helper_t::make_empty();
    }
  }
  static constexpr detail::maybe_number stack_space_needed{detail::stack_space_needed<traits::read<clean_val_t>>::value};
};

} // end namespace detail

} // end namespace primer
