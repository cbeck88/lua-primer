//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

namespace primer {

class error;

template <typename T, typename E = primer::error>
class expected;

template <typename T, typename E>
class expected<T &, E>;

template <typename E>
class expected<void, E>;

} // end namespace primer
