//  (C) Copyright 2015 - 2017 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * Helper to work around "sorry, unimplemented: mangling noexcept_expr"
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <new>
#include <utility>

namespace primer {

namespace detail {

template <typename T, typename... Args>
struct nothrow_newable {
  static constexpr bool value =
    noexcept(new (nullptr) T{std::forward<Args>(std::declval<Args>())...});
};

} // end namespace detail

} // end namespace primer
