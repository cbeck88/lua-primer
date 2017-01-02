//  (C) Copyright 2015 - 2017 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

namespace primer {

class error;

// Primary template
template <typename T, typename E = primer::error>
class expected;

template <typename T, typename E>
class expected<T &, E>;

template <typename E>
class expected<void, E>;

// fold_expected_t is used to implement map
template <typename X, typename E>
struct fold_expected {
  using type = expected<X, E>;
};

template <typename T, typename E, typename E2>
struct fold_expected<expected<T, E>, E2> {
  using type = expected<T, E>;
};

template <typename T, typename E>
using fold_expected_t = typename fold_expected<T, E>::type;

} // end namespace primer
