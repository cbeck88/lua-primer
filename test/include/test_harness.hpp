//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <primer/base.hpp>
#include <primer/lua.hpp>
#include <primer/error.hpp>
#include <primer/expected.hpp>
#include <primer/push.hpp>
#include <primer/read.hpp>
#include <primer/support/error_capture.hpp>

#include <cstring>
#include <exception>
#include <initializer_list>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

/***
 * Test harness
 */

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

static void log_conf() {
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
  std::cout << std::endl;
}
}

// RAII object for a lua state

struct lua_raii {
  lua_State * const L_;

  lua_raii()
    : L_(luaL_newstate())
  {
    // std::cerr << "opened state: L = " << L_ << std::endl;
    luaL_checkversion(L_);
  }

  ~lua_raii() {
    // std::cerr << "closing state: L = " << L_ << std::endl;
    lua_close(L_);
  }

  lua_raii(const lua_raii &) = delete;
  lua_raii(lua_raii &&) = delete;

  operator lua_State *() const { return L_; }
};

// Test exception type. Test assertion failures throw this.

struct test_exception : std::exception {
  std::string message_;

  explicit test_exception(const std::string & m)
    : message_(m)
  {}

  virtual const char * what() const throw() override {
    return message_.c_str();
  }
};

/***
 * Macros which create assertions
 */

#define TEST(C, X)                                                             \
  do {                                                                         \
    if (!(C)) {                                                                \
      std::ostringstream ss__;                                                 \
      ss__ << "Condition " #C " failed (line " << __LINE__ << ") : " << X;     \
      throw test_exception{ss__.str()};                                        \
    }                                                                          \
  } while (0)

#define TEST_EQ(A, B)                                                          \
  do {                                                                         \
    if (!((A) == (B))) {                                                       \
      std::ostringstream ss__;                                                 \
      ss__ << "Condition " #A " == " #B " failed: (line " << __LINE__          \
           << ")\n        (LHS) = (" << (A) << ") , (RHS) = (" << (B) << ")";  \
      throw test_exception{ss__.str()};                                        \
    }                                                                          \
  } while (0)

#define CHECK_STACK(L, I) TEST_EQ(lua_gettop(L), I)

#define TEST_EXPECTED(C)                                                       \
  do {                                                                         \
    const auto & result__ = C;                                                 \
    if (!result__) {                                                           \
      std::ostringstream ss__;                                                 \
      ss__ << "An operation failed '" #C "':\n" << result__.err_str();         \
      throw test_exception(ss__.str());                                        \
    }                                                                          \
  } while (0)

/***
 * Test types on the stack
 */

inline void test_type(lua_State * L, int idx, int expected, int line) {
  if (lua_type(L, idx) != expected) {
    TEST(false, "Expected '" << lua_typename(L, expected) << "', found '"
                             << lua_typename(L, lua_type(L, idx))
                             << "' line: " << line);
  }
}

inline void test_top_type(lua_State * L, int expected, int line) {
  test_type(L, -1, expected, line);
}

/***
 * Test round tripping any given C++ value with lua stack
 */

template <typename T>
void round_trip_value(lua_State * L, const T & t, int line) {
  PRIMER_ASSERT_STACK_NEUTRAL(L);

  primer::push(L, t);
  auto r = primer::read<T>(L, -1);
  TEST(r, "Failed to recover when roundtripping a value. line: " +
            std::to_string(line));
  TEST_EQ(t, *r);
  lua_pop(L, 1);
}

/***
 * Load a script, and if it cannot be loaded, throw an error.
 */

primer::expected<void> try_load_script(lua_State * L, const char * script) {
  int code = luaL_loadstring(L, script);
  if (LUA_OK != code) {
    return primer::detail::pop_error(L, code).prepend_error_line(script);
  }
  return {};
}

/***
 * Test harness object
 */

typedef void (*test_func_t)();

struct test_record {
  const char * name;
  test_func_t func;
};

struct test_harness {
  std::vector<test_record> tests_;

  explicit test_harness(std::initializer_list<test_record> list)
    : tests_(list)
  {}

  int run() const {
    int num_fails = 0;
    for (const auto & t : tests_) {
      bool okay = false;
      try {
        std::cout << "  TEST " << t.name << ": ... ";
        {
          int l = 50 - static_cast<int>(strlen(t.name));
          for (int i = 0; i < l; ++i) {
            std::cout << ' ';
          }
        }
        std::cout.flush();

        t.func();
        okay = true;
      } catch (test_exception & te) {
        std::cout << " FAILED\n      A test condition was not met.\n      "
                  << te.what() << std::endl;
      } catch (std::exception & e) {
        std::cout << " FAILED\n      A standard exception was thrown.\n      "
                  << e.what() << std::endl;
      } catch (...) {
        std::cout
          << " FAILED\n      An unknown (exception?) was thrown.\n      !!!"
          << std::endl;
      }
      if (okay) {
        std::cout << " passed" << std::endl;
      } else {
        ++num_fails;
      }
    }
    return num_fails;
  }
};
