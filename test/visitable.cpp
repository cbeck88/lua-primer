#include <primer/primer.hpp>
#include <primer/visit_struct.hpp>

#include <primer/api/callback_registrar.hpp>
#include <primer/api/callbacks.hpp>

#include "test_harness/test_harness.hpp"
#include <iostream>
#include <string>

namespace test {

struct foo {
  bool b;
  int a;
  float c;
};

struct bar {
  BEGIN_VISITABLES(bar);
  VISITABLE(std::string, d);
  VISITABLE(foo, e);
  VISITABLE(foo, f);
  END_VISITABLES;
};

} // end namespace test

VISITABLE_STRUCT(test::foo, a, b, c);

UNIT_TEST(visitable_push) {

  lua_raii L;

  {
    test::foo s1{true, 5, 11.75f};
    primer::push(L, s1);

    test_top_type(L, LUA_TTABLE, __LINE__);

    {
      lua_getfield(L, -1, "a");
      test_top_type(L, LUA_TNUMBER, __LINE__);
      TEST_EQ(true, lua_isinteger(L, -1));
      lua_pop(L, 1);
    }

    {
      lua_getfield(L, -1, "b");
      test_top_type(L, LUA_TBOOLEAN, __LINE__);
      TEST_EQ(true, lua_toboolean(L, -1));
      lua_pop(L, 1);
    }

    {
      lua_getfield(L, -1, "c");
      test_top_type(L, LUA_TNUMBER, __LINE__);
      TEST_EQ(11.75f, lua_tonumber(L, -1));
      lua_pop(L, 1);
    }
    lua_pop(L, 1);

    CHECK_STACK(L, 0);

    test::bar s2{"asdf", s1, s1};
    s2.f.b = false;
    s2.f.c = -1.0f;

    primer::push(L, s2);
    test_top_type(L, LUA_TTABLE, __LINE__);

    {
      lua_getfield(L, -1, "d");
      test_top_type(L, LUA_TSTRING, __LINE__);
      TEST_EQ(lua_tostring(L, -1), std::string{"asdf"});
      lua_pop(L, 1);
    }
    {
      lua_getfield(L, -1, "e");
      test_top_type(L, LUA_TTABLE, __LINE__);

      {
        lua_getfield(L, -1, "b");
        test_top_type(L, LUA_TBOOLEAN, __LINE__);
        TEST_EQ(true, lua_toboolean(L, -1));
        lua_pop(L, 1);
      }

      {
        lua_getfield(L, -1, "c");
        test_top_type(L, LUA_TNUMBER, __LINE__);
        TEST_EQ(11.75f, lua_tonumber(L, -1));
        lua_pop(L, 1);
      }

      lua_pop(L, 1);
    }

    CHECK_STACK(L, 1);

    {
      lua_getfield(L, -1, "f");
      test_top_type(L, LUA_TTABLE, __LINE__);

      {
        lua_getfield(L, -1, "b");
        test_top_type(L, LUA_TBOOLEAN, __LINE__);
        TEST_EQ(false, lua_toboolean(L, -1));
        lua_pop(L, 1);
      }

      {
        lua_getfield(L, -1, "c");
        test_top_type(L, LUA_TNUMBER, __LINE__);
        TEST_EQ(-1.0f, lua_tonumber(L, -1));
        lua_pop(L, 1);
      }

      lua_pop(L, 1);
    }

    lua_pop(L, 1);

    CHECK_STACK(L, 0);
  }
}

UNIT_TEST(visitable_read) {
  lua_raii L;

  lua_newtable(L);
  lua_pushinteger(L, 10);
  lua_setfield(L, -2, "a");
  lua_pushboolean(L, false);
  lua_setfield(L, -2, "b");
  lua_pushnumber(L, 8.5f);
  lua_setfield(L, -2, "c");

  CHECK_STACK(L, 1);

  auto result = primer::read<test::foo>(L, 1);
  TEST(result, "Could not read result 'foo' at line: " << __LINE__);

  TEST_EQ(result->a, 10);
  TEST_EQ(result->b, false);
  TEST_EQ(result->c, 8.5f);
}

UNIT_TEST(visitable_round_trip) {
  lua_raii L;

  test::foo f{true, 5, 9};

  {
    primer::push(L, f);
    CHECK_STACK(L, 1);
    auto g = primer::read<test::foo>(L, 1);
    TEST(g, "failed to read back 'test::foo' struct");

    TEST_EQ(g->a, f.a);
    TEST_EQ(g->b, f.b);
    TEST_EQ(g->c, f.c);
  }

  lua_pop(L, 1);

  f.b = false;
  f.c = 17.75;

  {
    primer::push(L, f);
    CHECK_STACK(L, 1);
    auto g = primer::read<test::foo>(L, 1);
    TEST(g, "failed to read back 'test::foo' struct");

    TEST_EQ(g->a, f.a);
    TEST_EQ(g->b, f.b);
    TEST_EQ(g->c, f.c);

    lua_pop(L, 1);
  }

  f.a += 95;

  {
    primer::push(L, f);
    CHECK_STACK(L, 1);
    auto g = primer::read<test::foo>(L, 1);
    TEST(g, "failed to read back 'test::foo' struct");

    TEST_EQ(g->a, f.a);
    TEST_EQ(g->b, f.b);
    TEST_EQ(g->c, f.c);

    lua_pop(L, 1);
  }

  test::bar h{"wasd", f, f};
  h.f.a -= 77;
  h.f.b = true;

  {
    primer::push(L, h);
    CHECK_STACK(L, 1);
    auto i = primer::read<test::bar>(L, 1);
    TEST(i, "failed to read back 'test::bar' struct");

    TEST_EQ(i->d, h.d);
    TEST_EQ(i->e.a, h.e.a);
    TEST_EQ(i->e.b, h.e.b);
    TEST_EQ(i->e.c, h.e.c);
    TEST_EQ(i->f.a, h.f.a);
    TEST_EQ(i->f.b, h.f.b);
    TEST_EQ(i->f.c, h.f.c);

    lua_pop(L, 1);
  }
}

primer::result
test_func_one(lua_State * L, test::foo f, test::foo g) {
  test::foo result{f.b != g.b, f.a - g.a, f.c + g.c};
  primer::push(L, result);
  return 1;
}

UNIT_TEST(visitable_function_parameters) {
  lua_raii L;

  lua_CFunction f = PRIMER_ADAPT(&test_func_one);

  lua_pushcfunction(L, f);
  lua_setglobal(L, "f");

  TEST_LUA_OK(L, luaL_loadstring(L,
                                 "return f({ b = true, c = 5.5, a = 7}, {a= "
                                 "9, b = false, c = 2.5})"));
  TEST_LUA_OK(L, lua_pcall(L, 0, 1, 0));

  CHECK_STACK(L, 1);
  auto maybe_foo = primer::read<test::foo>(L, 1);
  TEST(maybe_foo, "expected to read back a foo");
  TEST_EQ(maybe_foo->b, true);
  TEST_EQ(maybe_foo->a, -2);
  TEST_EQ(maybe_foo->c, 8);
  lua_pop(L, 1);

  TEST_LUA_OK(L, luaL_loadstring(L,
                                 "return f({ b = true, c = 5.5, d = 7}, {a= "
                                 "9, b = false, c = 2.5})"));
  TEST_EQ(LUA_ERRRUN, lua_pcall(L, 0, 1, 0));
  CHECK_STACK(L, 1);
  lua_pop(L, 1);

  TEST_LUA_OK(L, luaL_loadstring(L,
                                 "return f({ b = true, c = 5.5, a = '7'}, {a= "
                                 "9, b = false, c = 2.5})"));
  TEST_EQ(LUA_ERRRUN, lua_pcall(L, 0, 1, 0));
  CHECK_STACK(L, 1);
  lua_pop(L, 1);

  TEST_LUA_OK(L, luaL_loadstring(L,
                                 "return f({ b = true, c = 5.5, a = -15.5}, "
                                 "{a= 9, b = false, c = 2.5})"));
  TEST_EQ(LUA_ERRRUN, lua_pcall(L, 0, 1, 0));
  CHECK_STACK(L, 1);
  lua_pop(L, 1);

  TEST_LUA_OK(L, luaL_loadstring(L,
                                 "return f({ b = true, c = {}, a = 3}, {a= 9, "
                                 "b = false, c = 2.5})"));
  TEST_EQ(LUA_ERRRUN, lua_pcall(L, 0, 1, 0));
  CHECK_STACK(L, 1);
  lua_pop(L, 1);

  TEST_LUA_OK(L, luaL_loadstring(L,
                                 "return f({ b = 1, c = 5.5, a = 3}, {a= 9, b "
                                 "= false, c = 2.5})"));
  TEST_EQ(LUA_ERRRUN, lua_pcall(L, 0, 1, 0));
  CHECK_STACK(L, 1);
  lua_pop(L, 1);

  TEST_LUA_OK(L, luaL_loadstring(L,
                                 "return f{{ b = true, c = {}, a = 3}, {a= 9, "
                                 "b = false, c = 2.5}}"));
  TEST_EQ(LUA_ERRRUN, lua_pcall(L, 0, 1, 0));
  CHECK_STACK(L, 1);
  lua_pop(L, 1);

  TEST_LUA_OK(L, luaL_loadstring(L,
                                 "return f({ b = true, c = 5.5, a = 7}, {q = "
                                 "9, b = false, c = 2.5})"));
  TEST_EQ(LUA_ERRRUN, lua_pcall(L, 0, 1, 0));
  CHECK_STACK(L, 1);
  lua_pop(L, 1);
}

struct test_api : public primer::callback_registrar<test_api> {
  lua_raii L_;
  int internal_state_;
  primer::api::callbacks cb_man_;

  test_api()
    : L_()
    , internal_state_(0)
    , cb_man_(this) {
    cb_man_.on_init(L_);
  }

  NEW_LUA_CALLBACK(f1, "test function one")
  (lua_State *, int i)->primer::result {
    internal_state_ += i;
    return 0;
  }

  NEW_LUA_CALLBACK(f2, "test function two")
  (lua_State * L, int i, int j)->primer::result {
    internal_state_ -= i;
    internal_state_ *= j;
    lua_pushinteger(L, internal_state_);
    return 1;
  }

  struct f3_arg {
    BEGIN_VISITABLES(f3_arg);
    VISITABLE(int, plus);
    VISITABLE(int, minus);
    VISITABLE(int, times);
    END_VISITABLES;
  };

  NEW_LUA_CALLBACK(f3, "test function three")
  (lua_State * L, f3_arg arg)->primer::result {
    internal_state_ += arg.plus;
    internal_state_ -= arg.minus;
    internal_state_ *= arg.times;
    lua_pushinteger(L, internal_state_);
    return 1;
  }
};

UNIT_TEST(api_callbacks) {
  test_api a;

  lua_State * L = a.L_;

  luaL_requiref(L, "", luaopen_base, 1);
  lua_pop(L, 1);
  CHECK_STACK(L, 0);

  const char * script =
    ""
    "f1(2)                                           \n"
    "f1(3)                                           \n"
    "assert(5 == f2(0, 1))                           \n"
    "assert(12 == f2(-1, 2))                         \n"
    "f1(-6)                                          \n"
    "assert(-3 == f2(3, -1))                         \n"
    "assert(0 == f3{plus = 1, minus = 1, times = 0}) \n"
    "assert(4 == f3{plus = 3, minus = 1, times = 2}) \n";

  TEST_LUA_OK(L, luaL_loadstring(L, script));

  auto result = primer::fcn_call_no_ret(L, 0);
  TEST_EXPECTED(result);

  CHECK_STACK(L, 0);

  TEST_EQ(4, a.internal_state_);
}

int
main() {
  conf::log_conf();

  std::cout << "Visitable structure tests:" << std::endl;
  test_registrar::run_tests();
}
