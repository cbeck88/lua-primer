//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

// Forward declarations

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/expected_fwd.hpp>
#include <primer/traits/userdata.hpp>

namespace primer {

class bound_function;
class coroutine;
class lua_ref;
struct lua_ref_seq;
class lua_state_ref;
class result;

namespace traits {

template <typename T, typename ENABLE = void>
struct push;

template <typename T, typename ENABLE = void>
struct read;

} // end namespace traits
} // end namespace primer
