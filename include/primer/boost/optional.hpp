//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * How to transfer `boost::optional` to and from the stack, using strict
 * optional semantics
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/container/optional_base.hpp>
#include <boost/optional/optional.hpp>

namespace primer {
namespace traits {

template <typename T>
struct push<boost::optional<T>> : detail::optional_push<boost::optional<T>> {};

template <typename T>
struct read<boost::optional<T>>
  : detail::optional_strict_read<boost::optional<T>> {};

} // end namespace traits
} // end namespace primer
