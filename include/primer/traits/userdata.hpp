//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

namespace primer {
namespace traits {

/***
 * Trait used to register a type as userdata.
 *
 * A valid partial specialization has members:
 *   static const char * name;        // name used in lua registry for this userdata type
 *   static const luaL_Reg * methods; // methods registered in the metatable for this userdata type.
 *
 * luaL_Reg is defined in lua headers as a struct:
 *
 *   struct luaL_Reg {
 *     const char * name;
 *     int (*func)(lua_State *);
 *   };
 *
 * It is sufficient to use any type which has at least these members with these names and types, it need not
 * be a type related to luaL_Reg.
 * 
 * See traits/is_userdata.hpp for the static_asserts which we apply.
 */

template <typename T>
struct userdata;

} // end namespace traits
} // end namespace primer
