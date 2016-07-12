//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

//[ primer_error_handler_synopsis
namespace primer {

// Pushes the current error handler function onto the stack.
// (Default is debug.traceback)
inline int get_error_handler(lua_State * L) noexcept;

// Pops a function from the stack and sets it to be the error handler.
inline void set_error_handler(lua_State * L) noexcept;

} // end namespace primer
//]
