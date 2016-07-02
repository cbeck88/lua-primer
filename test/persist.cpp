#include <primer/api/callback_manager.hpp>
#include <primer/api/base.hpp>
#include <primer/api/library.hpp>
#include <primer/support/function.hpp>
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

  void restore(const std::string & buffer) { this->unpersist(L, buffer); }

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
    // PRIMER_ASSERT_STACK_NEUTRAL(L);

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

struct test_api_two : primer::api::base<test_api_two> {
  lua_raii L_;

  API_FEATURE(primer::api::libraries<primer::api::lua_base_lib>, libs_);
  API_FEATURE(primer::api::callback_manager, cb_man_);

  NEW_LUA_CALLBACK(f, "this is the f help")(lua_State * L, int i, int j) {
    if (i < j) {
      lua_pushinteger(L, -1);
    } else if (i > j) {
      lua_pushinteger(L, 1);
    } else {
      return primer::error("bad bad");
    }
    return 1;
  }

  NEW_LUA_CALLBACK(g, "this is the g help")(lua_State * L, std::string i, std::string j) {
    if (i < j) {
      lua_pushinteger(L, -1);
    } else if (i > j) {
      lua_pushinteger(L, 1);
    } else {
      return primer::error("bad bad");
    }
    return 1;
  }

  USE_LUA_CALLBACK(help,
                   "get help for a built-in function",
                   &primer::api::intf_help_impl);

  test_api_two()
    : L_()
    , cb_man_(this)
  {
    this->initialize_api(L_);

    luaL_requiref(L_, "", luaopen_base, 1);
    lua_pop(L_, 1);

    // Set debugging mode for eris
    lua_pushboolean(L_, true);
    eris_set_setting(L_, "path", -1);
    lua_pop(L_, 1);
  }

  std::string save() {
    std::string result;
    this->persist(L_, result);
    return result;
  }

  void restore(const std::string & buffer) { this->unpersist(L_, buffer); }
};

void test_api_base() {
  std::string buffer;

  const char * script =
    ""
    "x = 4;                                          \n"
    "y = 5;                                          \n"
    "assert(f(x,y) == -1);                           \n"
    "x = x + 4                                       \n"
    "assert(f(x,y) == 1);                            \n"
    "                                                \n"
    "assert(g('asdf', 'jkl;') == -1)                 \n"
    "assert(pcall(g, 'asdf', 'afsd'))                \n"
    "assert(not pcall(g, 'asdf', 'asdf'))            \n";


  {
    test_api_two a;

    lua_State * L = a.L_;

    CHECK_STACK(L, 0);

    TEST_EQ(LUA_OK, luaL_loadstring(L, script));

    auto result = primer::fcn_call_no_ret(L, 0);
    TEST_EXPECTED(result);

    CHECK_STACK(L, 0);

    buffer = a.save();
  }

  {
    test_api_two a;
    a.restore(buffer);

    lua_State * L = a.L_;

    CHECK_STACK(L, 0);

    TEST_EQ(LUA_OK, luaL_loadstring(L, script));

    auto result = primer::fcn_call_no_ret(L, 0);
    TEST_EXPECTED(result);

    CHECK_STACK(L, 0);
  }
}

void test_api_help() {
  test_api_two a;

  lua_State * L = a.L_;

  const char * script =
    ""
    "assert(help(f) == 'this is the f help')         \n"
    "assert(help(g) == 'this is the g help')         \n";

  TEST_EQ(LUA_OK, luaL_loadstring(L, script));

  auto result = primer::fcn_call_no_ret(L, 0);
  TEST_EXPECTED(result);

  CHECK_STACK(L, 0);
}

int main() {
  conf::log_conf();

  std::cout << "Persistence tests:" << std::endl;
  test_harness tests{
    {"persist simple", &test_persist_simple},
    {"api base", &test_api_base},
    {"api help", &test_api_help},
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
