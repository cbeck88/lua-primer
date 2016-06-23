#include <visit_struct/visit_struct.hpp>
#include <visit_struct/visit_struct_intrusive.hpp>

#include <primer/push.hpp>
#include <primer/read.hpp>
#include <primer/traits/push_visitable.hpp>

#include "test_harness.hpp"
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


void visitable_push_test() {

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

int main() {
  conf::log_conf();

  std::cout << "Visitable structure tests:" << std::endl;
  test_harness tests{
    {"visitable push test", &visitable_push_test},
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
