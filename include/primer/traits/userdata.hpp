//  (C) Copyright 2015 - 2017 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * Trait used to register a type as userdata.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

namespace primer {
namespace traits {

template <typename T>
struct userdata;

} // end namespace traits
} // end namespace primer
