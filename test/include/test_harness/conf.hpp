//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * Log the build configuration
 */

#include <primer/conf.hpp>
#include <primer/lua.hpp>

#include <iostream>

namespace conf {
static constexpr bool lua_32bits =
#ifdef LUA_32BITS
  true
#else
  false
#endif
  ;

static constexpr bool lua_usec89 =
#ifdef LUA_USE_C89
  true
#else
  false
#endif
  ;

static constexpr bool primer_debug =
#ifdef PRIMER_DEBUG
  true
#else
  false
#endif
  ;

static constexpr bool primer_lua_as_cpp =
#ifdef PRIMER_LUA_AS_CPP
  true
#else
  false
#endif
  ;

static constexpr bool primer_no_asserts =
#ifdef PRIMER_NO_STATIC_ASSERTS
  true
#else
  false
#endif
  ;

static constexpr bool primer_no_exceptions =
#ifdef PRIMER_NO_EXCEPTIONS
  true
#else
  false
#endif
  ;

static constexpr bool primer_no_mem_fail =
#ifdef PRIMER_NO_MEMORY_FAILURE
  true
#else
  false
#endif
  ;

static inline void log_conf() {
  std::cout << LUA_RELEASE << std::endl;
  std::cout << "  LUA_32BITS               = " << lua_32bits << "\n";
  std::cout << "  LUA_USE_C89              = " << lua_usec89 << "\n";
  std::cout << "  sizeof(LUA_INTEGER)      = " << sizeof(LUA_INTEGER) << "\n";
  std::cout << "  sizeof(LUA_NUMBER)       = " << sizeof(LUA_NUMBER) << "\n";
  std::cout << std::endl;
  std::cout << PRIMER_RELEASE << "\n";
  std::cout << "  PRIMER_DEBUG             = " << primer_debug << "\n";
  std::cout << "  PRIMER_LUA_AS_CPP        = " << primer_lua_as_cpp << "\n";
  std::cout << "  PRIMER_NO_STATIC_ASSERTS = " << primer_no_asserts << "\n";
  std::cout << "  PRIMER_NO_EXCEPTIONS     = " << primer_no_exceptions << "\n";
  std::cout << "  PRIMER_NO_MEMORY_FAILURE = " << primer_no_mem_fail << "\n";
  std::cout << std::endl;
}
} // end namespcae conf
