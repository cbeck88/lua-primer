#include <primer/boost.hpp>

#include <primer/adapt.hpp>
#include <primer/bound_function.hpp>
#include <primer/coroutine.hpp>
#include <primer/push.hpp>
#include <primer/read.hpp>

#include "test_harness.hpp"
#include <iostream>
#include <string>

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

/***
 * Test flat set
 */

template <typename T>
std::ostream & operator << (std::ostream & o, const boost::container::flat_set<T> & s) {
  o << "{ ";
  for (const auto & i : s) {
    o << i << ", ";
  }
  o << "}";
  return o;
}


void test_flat_set_round_trip() {
  lua_raii L;

  using boost::container::flat_set;

  round_trip_value(L, flat_set<int>{}, __LINE__);
  round_trip_value(L, flat_set<int>{1, 5, 6, 10, 234}, __LINE__);
  round_trip_value(L, flat_set<std::string>{"wer", "qWQE", "asjdkljweWERWERE", "", "foo"}, __LINE__);
  round_trip_value(L, flat_set<bool>{true}, __LINE__);
}


/***
 * Test optional adapted, bound function, coroutine
 */

primer::result test_func(lua_State * L, int i, int j, boost::optional<std::string> k, boost::optional<std::string> l) {
  if (k && l && (*k != *l)) { return primer::error("Expected equal strings"); }
  lua_pushboolean(L, (i==j));
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
}

int yield_helper(lua_State * L) {
  return lua_yield(L, 1);
}

void test_coroutine() {
  lua_raii L;

  lua_pushcfunction(L, PRIMER_ADAPT(&yield_helper));
  lua_setglobal(L, "yield_helper");
  CHECK_STACK(L, 0);

  TEST_EQ(LUA_OK, luaL_loadstring(L, ""
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
  TEST_EQ(LUA_OK, lua_pcall(L, 0, 1, 0));
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

int main() {
  conf::log_conf();

  std::cout << "Boost tests:" << std::endl;
  test_harness tests{
    {"test optional push", &test_optional_push},
    {"test optional read", &test_optional_read},
    {"test flat set", &test_flat_set_round_trip},
    {"test bound function", &test_bound_function},
    {"test coroutine", &test_coroutine},
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
