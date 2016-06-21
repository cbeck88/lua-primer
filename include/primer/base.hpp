//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cstddef> // for std::size_t
#include <primer/support/assert_filescope.hpp>

PRIMER_ASSERT_FILESCOPE;

#define PRIMER_VERSION_MAJOR    "0"
#define PRIMER_VERSION_MINOR    "0"
#define PRIMER_VERSION_NUM      000
#define PRIMER_VERSION_RELEASE  "1"

#define PRIMER_VERSION          "Lua Primer " PRIMER_VERSION_MAJOR "." PRIMER_VERSION_MINOR
#define PRIMER_RELEASE          PRIMER_VERSION "." PRIMER_VERSION_RELEASE

/* #define PRIMER_DEBUG */
/* #define PRIMER_LUA_AS_CPP */
/* #define PRIMER_NO_EXCEPTIONS */

// Forward declare some lua types
struct lua_State;
typedef int (*lua_CFunction)(lua_State *);

namespace primer {

typedef unsigned int uint;

// Use this type if you want to push nil onto the stack using primer::push interface.
struct nil_t{};

} // end namespace primer
