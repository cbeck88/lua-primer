//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * Include eris headers as C or C++ depending on a macro
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/lua.hpp>

#ifndef PRIMER_LUA_AS_CPP

extern "C" {
#include <eris.h>
}

#else

#include <eris.h>

#endif
