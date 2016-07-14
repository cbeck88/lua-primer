//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * PRIMER_FATAL_ERROR, PRIMER_ASSERT are pretty much what you expect.
 *
 * PRIMER_ASSERT_STACK_NEUTRAL is a debugging aide.
 * It creates a stack object which records the top of the stack in its ctor.
 * It checks it again in the dtor, if they don't match then it causes an
 * assertion failure.
 *
 * This macro is only active if PRIMER_DEBUG is defined. If not, then it does
 * nothing.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#ifndef PRIMER_DEBUG

#define PRIMER_DEBUG_MSG(X)
#define PRIMER_FATAL_ERROR(X)
#define PRIMER_ASSERT(C, X) static_asert(noexcept(C) == noexcept(C), "")
#define PRIMER_ASSERT_STACK_NEUTRAL(L)                                         \
  static_cast<const void>(L);                                                  \
  static_assert(true, "")
#define PRIMER_ASSERT_TABLE(L)                                                 \
  static_cast<const void>(L);                                                  \
  static_assert(true, "")

#else // PRIMER_DEBUG

#include <primer/lua.hpp>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

namespace primer {

// Assertions

inline void fatal_error(const std::string & message) {
  std::cerr << message << std::endl;
  std::abort();
}

#define PRIMER_FATAL_ERROR(X)                                                  \
  do {                                                                         \
    std::stringstream ss;                                                      \
    ss << X;                                                                   \
    ::primer::fatal_error(ss.str());                                           \
  } while (0)


#define PRIMER_ASSERT(C, X)                                                    \
  do {                                                                         \
    if (!(C)) {                                                                \
      std::stringstream ss;                                                    \
      ss << "Assertion " #C " failed!\n" << X;                                 \
      ::primer::fatal_error(ss.str());                                         \
    }                                                                          \
  } while (0)


// Help to catch lua stack discipline problems

class stack_neutrality_assertion {
  lua_State * L_;
  const char * file_;
  int line_;
  int top_;

public:
  explicit stack_neutrality_assertion(lua_State * L, const char * file, int line)
    : L_(L)
    , file_(file)
    , line_(line)
    , top_(lua_gettop(L))
  {}

  ~stack_neutrality_assertion() {
    int end = lua_gettop(L_);
    PRIMER_ASSERT(top_ == end,
                  "[" << file_ << ":" << line_
                      << "] PRIMER_ASSERT_STACK_NEUTRAL failed. start " << top_
                      << " end " << end);
  }
};

#define PRIMER_ASSERT_STACK_NEUTRAL(L)                                         \
  primer::stack_neutrality_assertion PRIMER_TOKEN_PASTE(anonymous_variable_,   \
                                                        __LINE__) {            \
    L, __FILE__, __LINE__                                                      \
  }

#define PRIMER_DEBUG_MSG(X)                                                    \
  do {                                                                         \
    std::cerr << "      [" << __FILE__ << ":" << __LINE__ << "] " << X         \
              << std::endl;                                                    \
  } while (0)

inline void assert_table(lua_State * L, const char * fname) {
  auto t = lua_type(L, -1);
  PRIMER_ASSERT(t == LUA_TTABLE || t == LUA_TUSERDATA || t == LUA_TLIGHTUSERDATA,
                "In " << fname << ", no table or table-like thing was found!");
}

#define PRIMER_ASSERT_TABLE(L)                                                 \
  primer::assert_table(L, __func__);                                           \
  static_assert(true, "")

} // end namespace primer

#endif // PRIMER_DEBUG
