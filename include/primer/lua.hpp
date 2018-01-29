//  (C) Copyright 2015 - 2018 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * Include lua headers as C or C++ depending on a macro
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#ifndef PRIMER_LUA_AS_CPP

extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}

#else

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

#endif
