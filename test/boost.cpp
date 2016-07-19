#include <primer/primer.hpp>
#include <primer/boost.hpp>

#include "test_harness/test_harness.hpp"
#include <cassert>
#include <iostream>
#include <string>
#include <set>
#include <vector>

#include <boost/version.hpp>
#include <boost/optional/optional.hpp>

/***
 * Test boost::optional
 */
void test_optional_push() {
  lua_raii L;

  boost::optional<int> b;
  primer::push(L, b);
  CHECK_STACK(L, 1);
  test_top_type(L, LUA_TNIL, __LINE__);

  b = 10;
  primer::push(L, b);
  CHECK_STACK(L, 2);
  test_top_type(L, LUA_TNUMBER, __LINE__);
  TEST_EQ(lua_tonumber(L, 2), *b);
}

void test_optional_read() {
  lua_raii L;

  {
    lua_pushinteger(L, 4);
    auto maybe_opt = primer::read<boost::optional<int>>(L, 1);
    CHECK_STACK(L, 1);

    TEST(maybe_opt, "Expected success");
    TEST(*maybe_opt, "Expected a value");
    TEST_EQ(**maybe_opt, 4);
    lua_pop(L, 1);
  }

  {
    lua_pushinteger(L, 4);
    auto maybe_opt = primer::read<boost::optional<std::string>>(L, 1);
    CHECK_STACK(L, 1);

    TEST(!maybe_opt, "Expected failure");
    lua_pop(L, 1);
  }

  {
    lua_pushnil(L);
    auto maybe_opt = primer::read<boost::optional<std::string>>(L, 1);
    CHECK_STACK(L, 1);

    TEST(maybe_opt, "Expected success");
    TEST(!*maybe_opt, "Expected empty optional");
    lua_pop(L, 1);
  }
}

//[ primer_example_maximum_function_defn
primer::result maximum(lua_State * L,
                       boost::optional<int> x,
                       boost::optional<int> y) {
  if (x) {
    if (y) {
      lua_pushinteger(L, std::max(*x, *y));
    } else {
      lua_pushinteger(L, *x);
    }
  } else {
    if (y) {
      lua_pushinteger(L, *y);
    } else {
      lua_pushnil(L);
    }
  }
  return 1;
}
//]

void test_optional_example() {
  lua_raii L;

  luaL_requiref(L, "", &luaopen_base, 1);
  lua_pop(L, 1);

  //[ primer_example_optional_push
  boost::optional<int> x;
  primer::push(L, x);
  assert(lua_gettop(L) == 1);
  assert(lua_isnil(L, 1));

  x = 7;
  primer::push(L, x);
  assert(lua_gettop(L) == 2);
  assert(lua_isinteger(L, 2));
  assert(lua_tointeger(L, 2) == 7);
  //]

  lua_settop(L, 0);

  //[ primer_example_optional_read
  lua_pushnumber(L, 5.5f);
  assert(lua_gettop(L) == 1);

  auto result1 = primer::read<boost::optional<LUA_NUMBER>>(L, 1);
  assert(result1);

  boost::optional<LUA_NUMBER> my_number = *result1;
  assert(my_number);
  assert(*my_number == 5.5f);

  lua_pushnil(L);
  assert(lua_gettop(L) == 2);

  auto result2 = primer::read<boost::optional<LUA_NUMBER>>(L, 2);
  assert(result2);

  boost::optional<LUA_NUMBER> my_number2 = *result2;
  assert(!my_number2); // got an empty state (nil)

  lua_pushstring(L, "foo");
  assert(lua_gettop(L) == 3);

  auto result3 = primer::read<boost::optional<LUA_NUMBER>>(L, 3);
  assert(!result3); // string is not nil or a number
  //]

  //[ primer_example_maximum_function_test
  lua_pushcfunction(L, PRIMER_ADAPT(&maximum));
  lua_setglobal(L, "maximum");

  luaL_loadstring(L,
                  "local x = 5                              \n"
                  "local y = 7                              \n"
                  "assert(maximum(x, y) == 7)               \n"
                  "assert(maximum(x, nil) == 5)             \n"
                  "assert(maximum(nil, y) == 7)             \n"
                  "assert(not maximum(nil, nil))            \n"
                  "assert(not pcall(maximum, 'foo', 'bar')) \n"
                  "assert(not pcall(maximum, 5, 'bar'))     \n"
                  "assert(not pcall(maximum, 'foo', nil))   \n"
                  "assert(pcall(maximum, 6, -14))           \n"
                  "assert(pcall(maximum, 6))                \n"
                  "assert(maximum(6) == 6)                  \n");

  assert(LUA_OK == lua_pcall(L, 0, 0, 0));
  //]
}

/***
 * Test boost::vector
 */

namespace boost {

namespace container {

template <typename T>
std::ostream & operator<<(std::ostream & o,
                          const boost::container::vector<T> & s) {
  o << "{ ";
  for (const auto & i : s) {
    o << i << ", ";
  }
  o << "}";
  return o;
}

} // end namespace container

} // end namespace boost

namespace {

template <typename U>
using T = boost::container::vector<U>;

void test_vector_round_trip() {
  lua_raii L;

  round_trip_value(L, T<int>{}, __LINE__);

  {
    std::vector<int> temp{1, 5, 6, 23, 1231};
    round_trip_value(L, T<int>{temp.begin(), temp.end()}, __LINE__);
  }
  {
    std::vector<std::string> temp{"wer", "qWQE", "asjdkljweWERWERE", "", "foo"};
    round_trip_value(L, T<std::string>{temp.begin(), temp.end()}, __LINE__);
  }
  {
    std::set<bool> temp{true};
    round_trip_value(L, T<bool>{temp.begin(), temp.end()}, __LINE__);
  }
}

} // end anonymous namespace

/***
 * Test optional adapted, bound function, coroutine
 */

primer::result test_func(lua_State * L,
                         int i,
                         int j,
                         boost::optional<std::string> k,
                         boost::optional<std::string> l) {
  if (k && l && (*k != *l)) { return primer::error("Expected equal strings"); }
  lua_pushboolean(L, (i == j));
  return 1;
}

void test_bound_function() {
  lua_raii L;

  lua_CFunction f1 = PRIMER_ADAPT(&test_func);

  lua_pushcfunction(L, f1);
  primer::bound_function func(L);
  CHECK_STACK(L, 0);
  TEST(func, "expected func to be valid");

  {
    auto maybe_result = func.call_one_ret(4, 5);
    CHECK_STACK(L, 0);
    TEST_EXPECTED(maybe_result);
    TEST(maybe_result->push(), "expected to be able to push");
    test_top_type(L, LUA_TBOOLEAN, __LINE__);
    TEST_EQ(false, lua_toboolean(L, 1));
    lua_pop(L, 1);
  }

  {
    auto maybe_result = func.call_one_ret(5, 5, "asdf");
    CHECK_STACK(L, 0);
    TEST_EXPECTED(maybe_result);
    TEST(maybe_result->push(), "expected to be able to push");
    test_top_type(L, LUA_TBOOLEAN, __LINE__);
    TEST_EQ(true, lua_toboolean(L, 1));
    lua_pop(L, 1);
  }

  {
    auto maybe_result = func.call_one_ret(6, 5, "Asdf", "Asdf");
    CHECK_STACK(L, 0);
    TEST_EXPECTED(maybe_result);
    TEST(maybe_result->push(), "expected to be able to push");
    test_top_type(L, LUA_TBOOLEAN, __LINE__);
    TEST_EQ(false, lua_toboolean(L, 1));
    lua_pop(L, 1);
  }

  {
    auto maybe_result = func.call_one_ret(4, 5, "asdf", "jkl;");
    CHECK_STACK(L, 0);
    TEST(!maybe_result, "expected an error message");
  }

  {
    auto maybe_result = func.call_no_ret(4, 5, "asdf", "jkl;");
    CHECK_STACK(L, 0);
    TEST(!maybe_result, "expected an error message");
  }

  {
    auto maybe_result = func.call_no_ret(6, 5, "Asdf", "Asdf");
    CHECK_STACK(L, 0);
    TEST_EXPECTED(maybe_result);
  }
}

void test_bound_function_binding() {
  lua_raii L;

  lua_CFunction f1 = PRIMER_ADAPT(&test_func);

  lua_pushcfunction(L, f1);
  primer::bound_function func(L);
  CHECK_STACK(L, 0);
  TEST(func, "expected to bind");

  func.debug_string();
  CHECK_STACK(L, 0);

  lua_pushinteger(L, 5);
  func = primer::bound_function{L};
  CHECK_STACK(L, 0);
  TEST(!func, "expected a dead state");

  func.debug_string();
  CHECK_STACK(L, 0);

  func = primer::bound_function{L};
  CHECK_STACK(L, 0);
  TEST(!func, "expected a dead state");

  func.debug_string();
  CHECK_STACK(L, 0);

  lua_pushcfunction(L, f1);
  func = primer::bound_function{L};
  CHECK_STACK(L, 0);
  TEST(func, "expected a good state");

  func.debug_string();
  CHECK_STACK(L, 0);

  func = primer::bound_function{nullptr};
  CHECK_STACK(L, 0);
  TEST(!func, "expected a dead state");

  func.debug_string();
  CHECK_STACK(L, 0);
}

int yield_helper(lua_State * L) { return lua_yield(L, 1); }

void test_coroutine() {
  lua_raii L;

  lua_pushcfunction(L, PRIMER_ADAPT(&yield_helper));
  lua_setglobal(L, "yield_helper");
  CHECK_STACK(L, 0);

  TEST_LUA_OK(L, luaL_loadstring(L,
                                 ""
                                 " local function make_closure()         \n"
                                 "   local counter = 0;                  \n"
                                 "   local function f(input)             \n"
                                 "     while true do                     \n"
                                 "       counter = counter + input;      \n"
                                 "       input = yield_helper(counter);  \n"
                                 "     end                               \n"
                                 "   end                                 \n"
                                 "   return f;                           \n"
                                 " end                                   \n"
                                 " return make_closure()                 \n"));
  TEST_LUA_OK(L, lua_pcall(L, 0, 1, 0));
  CHECK_STACK(L, 1);
  TEST_EQ(true, lua_isfunction(L, 1));
  primer::bound_function func{L};
  CHECK_STACK(L, 0);
  TEST(func, "expected a valid bound function");

  primer::coroutine c{func};
  CHECK_STACK(L, 0);
  TEST(c, "expected c to be valid");
  TEST(func, "expected func to be valid");

  auto result1 = c.call_one_ret(6);
  CHECK_STACK(L, 0);
  TEST_EXPECTED(result1);
  TEST(result1->push(L), "expected to be able to push");
  CHECK_STACK(L, 1);
  test_top_type(L, LUA_TNUMBER, __LINE__);
  TEST_EQ(lua_tonumber(L, 1), 6);
  lua_pop(L, 1);

  auto result2 = c.call_one_ret(7);
  CHECK_STACK(L, 0);
  TEST_EXPECTED(result2);
  TEST(result2->push(L), "expected to be able to push");
  CHECK_STACK(L, 1);
  test_top_type(L, LUA_TNUMBER, __LINE__);
  TEST_EQ(lua_tonumber(L, 1), 13);
  lua_pop(L, 1);

  auto result3 = c.call_one_ret(8);
  CHECK_STACK(L, 0);
  TEST_EXPECTED(result3);
  TEST(result3->push(L), "expected to be able to push");
  CHECK_STACK(L, 1);
  test_top_type(L, LUA_TNUMBER, __LINE__);
  TEST_EQ(lua_tonumber(L, 1), 21);
  lua_pop(L, 1);
}

//[ primer_example_ref_read_func
primer::result ref_test_func(lua_State * L, primer::lua_ref ref) {
  if (!ref) {
    lua_pushstring(L, "nil");
    return 1;
  } else if (ref.as<bool>()) {
    lua_pushstring(L, "bool");
    return 1;
  } else if (ref.as<int>()) {
    lua_pushstring(L, "int");
    return 1;
  } else if (ref.as<std::string>()) {
    primer::push(L, ref);
    return 1;
  }
  return primer::error("I can't handle it!");
}
//]

void test_ref_read() {
  lua_raii L;

  luaL_requiref(L, "", luaopen_base, 1);
  lua_pop(L, 1);

  //[ primer_example_ref_read_test
  lua_CFunction f = PRIMER_ADAPT(&ref_test_func);
  lua_pushcfunction(L, f);
  lua_setglobal(L, "ref_test_func");

  const char * script =
    ""
    "assert(ref_test_func() == 'nil')           \n"
    "assert(ref_test_func(5) == 'int')          \n"
    "assert(ref_test_func('foo') == 'foo')      \n"
    "assert(ref_test_func(true) == 'bool')      \n"
    "assert(not pcall(ref_test_func, 5.5))      \n"
    "assert(not pcall(ref_test_func, {}))       \n"
    "assert(pcall(ref_test_func, nil))          \n";

  assert(LUA_OK == luaL_loadstring(L, script));
  assert(LUA_OK == lua_pcall(L, 0, 0, 0));
  //]
  static_cast<void>(script);
}

void test_bound_function_mult_ret() {
  lua_raii L;

  const char * script =
    ""
    "return function(x, y)                      \n"
    "  return (x > y), x - y                    \n"
    "end                                        \n";

  TEST_LUA_OK(L, luaL_loadstring(L, script));
  TEST_LUA_OK(L, lua_pcall(L, 0, 1, 0));

  primer::bound_function func{L};
  TEST(func, "expected to have bound a function");
  CHECK_STACK(L, 0);

  {
    auto result = func.call(3, 4);
    TEST_EXPECTED(result);
    TEST_EXPECTED(result->at(0).as<bool>());
    TEST_EQ(false, *result->at(0).as<bool>());
    TEST_EXPECTED(result->at(1).as<int>());
    TEST_EQ(-1, *result->at(1).as<int>());
  }

  {
    auto result = func.call(7, 4);
    TEST_EXPECTED(result);
    TEST_EXPECTED(result->at(0).as<bool>());
    TEST_EQ(true, *result->at(0).as<bool>());
    TEST_EXPECTED(result->at(1).as<int>());
    TEST_EQ(3, *result->at(1).as<int>());
  }
}

int main() {
  conf::log_conf();
  std::cout << "Boost:\n";
  std::cout << "  BOOST_LIB_VERSION        = " << BOOST_LIB_VERSION << "\n";
  std::cout << std::endl;

  std::cout << "Boost tests:" << std::endl;
  test_harness tests{
    {"optional push", &test_optional_push},
    {"optional read", &test_optional_read},
    {"optional example", &test_optional_example},
    {"vector", &test_vector_round_trip},
    {"bound function", &test_bound_function},
    {"bound function binding", &test_bound_function},
    {"bound function multiple returns", &test_bound_function_mult_ret},
    {"coroutine", &test_coroutine},
    {"ref_fcn_read", &test_ref_read},
  };
  int num_fails = tests.run();
  std::cout << "\n";
  if (num_fails) {
    std::cout << num_fails << " tests failed!" << std::endl;
    return 1;
  } else {
    std::cout << "All tests passed!" << std::endl;
    return 0;
  }
}
