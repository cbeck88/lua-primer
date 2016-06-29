#include <primer/api/persistable.hpp>
#include <primer/lua.hpp>

#include "test_harness.hpp"
#include <iostream>
#include <string>

struct test_api_one : primer::api::persistable<test_api_one> {
  lua_raii L;

  std::string save() {
    std::string result;
    this->persist(L, result);
    return result;
  }

  void restore(const std::string & buffer) {
    this->unpersist(L, buffer);
  }

  void create_mock_state() {
    PRIMER_ASSERT_STACK_NEUTRAL(L);

    lua_newtable(L);
    lua_pushinteger(L, 5);
    lua_setfield(L, -2, "a");

    lua_pushinteger(L, 7);
    lua_setfield(L, -2, "b");

    lua_setglobal(L, "bah");

    lua_pushboolean(L, true);
    lua_setglobal(L, "humbug");
  }

  bool test_mock_state() {
    //PRIMER_ASSERT_STACK_NEUTRAL(L);

    lua_getglobal(L, "bah");
    if (!lua_istable(L, -1)) { return false; }

    lua_getfield(L, -1, "a");
    if (!lua_isinteger(L, -1)) { return false; }
    if (5 != lua_tointeger(L, -1)) { return false; }
    lua_pop(L, 1);

    lua_getfield(L, -1, "b");
    if (!lua_isinteger(L, -1)) { return false; }
    if (7 != lua_tointeger(L, -1)) { return false; }
    lua_pop(L, 1);

    lua_pop(L, 1);

    lua_getglobal(L, "humbug");
    if (!lua_isboolean(L, -1)) { return false; }
    if (!lua_toboolean(L, -1)) { return false; }
    lua_pop(L, 1);

    return true;
  }
};


void test_persist_simple() {
  std::string buffer;

  {
    test_api_one a;
    TEST_EQ(false, a.test_mock_state());

    a.create_mock_state();

    TEST_EQ(true, a.test_mock_state());

    buffer = a.save();

    TEST_EQ(true, a.test_mock_state());
  }

  {
    test_api_one a;
    TEST_EQ(false, a.test_mock_state());

    a.restore(buffer);    

    TEST_EQ(true, a.test_mock_state());
  }

  {
    test_api_one a;
    TEST_EQ(false, a.test_mock_state());

    a.restore(buffer);    

    TEST_EQ(true, a.test_mock_state());

    buffer = a.save();
  }

  {
    test_api_one a;
    TEST_EQ(false, a.test_mock_state());

    a.restore(buffer);    

    TEST_EQ(true, a.test_mock_state());
  }
}


int main() {
  conf::log_conf();

  std::cout << "Persistence tests:" << std::endl;
  test_harness tests{
    {"persist simple", &test_persist_simple},
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
