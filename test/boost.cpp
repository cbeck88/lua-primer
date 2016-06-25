#include <primer/boost.hpp>

#include <primer/push.hpp>
#include <primer/read.hpp>

#include "test_harness.hpp"
#include <iostream>
#include <string>

#include <boost/optional.hpp>

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


int main() {
  conf::log_conf();

  std::cout << "Boost tests:" << std::endl;
  test_harness tests{
    {"test optional push", &test_optional_push},
    {"test optional read", &test_optional_read},
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
