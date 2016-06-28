#include <primer/std.hpp>

#include <primer/adapt.hpp>
#include <primer/push.hpp>
#include <primer/read.hpp>
#include <primer/result.hpp>
#include <primer/support/function.hpp>
#include <primer/support/userdata.hpp>
#include <primer/support/userdata_dispatch.hpp>

#include "test_harness.hpp"
#include <iostream>
#include <string>

#include <array>
#include <map>
#include <set>
#include <vector>

void test_vector_push() {
  lua_raii L;

  std::vector<int> vec {{5, 6, 7, 8}};

  primer::push(L, vec);
  CHECK_STACK(L, 1);
  test_top_type(L, LUA_TTABLE, __LINE__);

  std::size_t n = lua_rawlen(L, -1);
  TEST_EQ(n, vec.size());

  lua_rawgeti(L, 1, 1);
  TEST(lua_isinteger(L, -1), "not an integer");
  TEST_EQ(lua_tointeger(L, -1), 5);
  lua_pop(L, 1);

  lua_rawgeti(L, 1, 2);
  TEST(lua_isinteger(L, -1), "not an integer");
  TEST_EQ(lua_tointeger(L, -1), 6);
  lua_pop(L, 1);

  lua_rawgeti(L, 1, 3);
  TEST(lua_isinteger(L, -1), "not an integer");
  TEST_EQ(lua_tointeger(L, -1), 7);
  lua_pop(L, 1);

  lua_rawgeti(L, 1, 4);
  TEST(lua_isinteger(L, -1), "not an integer");
  TEST_EQ(lua_tointeger(L, -1), 8);
  lua_pop(L, 1);

  lua_rawgeti(L, 1, 5);
  TEST(lua_isnil(L, -1), "expected nil");
  lua_pop(L, 1);
}

void test_array_push() {
  lua_raii L;

  std::array<int, 4> arr {{5, 6, 7, 8}};

  primer::push(L, arr);
  CHECK_STACK(L, 1);
  test_top_type(L, LUA_TTABLE, __LINE__);

  std::size_t n = lua_rawlen(L, -1);
  TEST_EQ(n, arr.size());

  lua_rawgeti(L, 1, 1);
  TEST(lua_isinteger(L, -1), "not an integer");
  TEST_EQ(lua_tointeger(L, -1), 5);
  lua_pop(L, 1);

  lua_rawgeti(L, 1, 2);
  TEST(lua_isinteger(L, -1), "not an integer");
  TEST_EQ(lua_tointeger(L, -1), 6);
  lua_pop(L, 1);

  lua_rawgeti(L, 1, 3);
  TEST(lua_isinteger(L, -1), "not an integer");
  TEST_EQ(lua_tointeger(L, -1), 7);
  lua_pop(L, 1);

  lua_rawgeti(L, 1, 4);
  TEST(lua_isinteger(L, -1), "not an integer");
  TEST_EQ(lua_tointeger(L, -1), 8);
  lua_pop(L, 1);

  lua_rawgeti(L, 1, 5);
  TEST(lua_isnil(L, -1), "expected nil");
  lua_pop(L, 1);
}

void test_map_push() {
  {
    std::map<std::string, std::string> my_map{{"a", "1"}, {"b", "2"}, {"c", "3"}};

    lua_raii L;

    primer::push(L, my_map);
    CHECK_STACK(L, 1);
    test_top_type(L, LUA_TTABLE, __LINE__);

    lua_pushnil(L);
    int counter = 0;
    while (lua_next(L, 1)) {
      ++counter;
      TEST(lua_isstring(L, 2), "expected a string key");
      std::string key = lua_tostring(L, 2);
      TEST(lua_isstring(L, 3), "expected a string val");
      std::string val = lua_tostring(L, 3);

      TEST_EQ(val, my_map[key]);
      my_map.erase(key);

      lua_pop(L, 1);
    }
    TEST_EQ(my_map.size(), 0);
    TEST_EQ(counter, 3);
  }

  {
    std::map<int, int> my_map{{'a', 1}, {'b', 2}, {'c', 3}};

    lua_raii L;

    primer::push(L, my_map);
    CHECK_STACK(L, 1);
    test_top_type(L, LUA_TTABLE, __LINE__);

    lua_pushnil(L);
    int counter = 0;
    while (lua_next(L, 1)) {
      ++counter;
      TEST(lua_isinteger(L, 2), "expected an int key");
      int key = lua_tointeger(L, 2);
      TEST(lua_isinteger(L, 3), "expected an int val");
      int val = lua_tointeger(L, 3);

      TEST_EQ(val, my_map[key]);
      my_map.erase(key);

      lua_pop(L, 1);
    }
    TEST_EQ(my_map.size(), 0);
    TEST_EQ(counter, 3);
  }
}

void test_set_push() {
  lua_raii L;

  std::set<std::string> s{"a", "k", "q", "j"};

  primer::push(L, s);

  CHECK_STACK(L, 1);
  test_top_type(L, LUA_TTABLE, __LINE__);

  lua_getfield(L, 1, "a");
  test_top_type(L, LUA_TBOOLEAN, __LINE__);
  TEST_EQ(true, lua_toboolean(L, 2));
  lua_pop(L, 1);

  lua_getfield(L, 1, "k");
  test_top_type(L, LUA_TBOOLEAN, __LINE__);
  TEST_EQ(true, lua_toboolean(L, 2));
  lua_pop(L, 1);

  lua_getfield(L, 1, "q");
  test_top_type(L, LUA_TBOOLEAN, __LINE__);
  TEST_EQ(true, lua_toboolean(L, 2));
  lua_pop(L, 1);

  lua_getfield(L, 1, "j");
  test_top_type(L, LUA_TBOOLEAN, __LINE__);
  TEST_EQ(true, lua_toboolean(L, 2));
  lua_pop(L, 1);

  lua_getfield(L, 1, "p");
  test_top_type(L, LUA_TNIL, __LINE__);
  lua_pop(L, 1);

  lua_getfield(L, 1, "b");
  test_top_type(L, LUA_TNIL, __LINE__);
  lua_pop(L, 1);

  lua_getfield(L, 1, "c");
  test_top_type(L, LUA_TNIL, __LINE__);
  lua_pop(L, 1);
}

/***
 * Stream ops for containers
 */

template <typename T>
std::ostream & operator << (std::ostream & o, const std::vector<T> & vec) {
  o << "{ ";
  for (const auto & i : vec) {
    o << i << ", ";
  }
  o << "}";
  return o;
}


template <typename T, std::size_t N>
std::ostream & operator << (std::ostream & o, const std::array<T, N> & vec) {
  o << "{ ";
  for (const auto & i : vec) {
    o << i << ", ";
  }
  o << "}";
  return o;
}

template <typename T>
std::ostream & operator << (std::ostream & o, const std::set<T> & s) {
  o << "{ ";
  for (const auto & i : s) {
    o << i << ", ";
  }
  o << "}";
  return o;
}

template <typename T, typename U>
std::ostream & operator << (std::ostream & o, const std::map<T, U> & m) {
  o << "{ ";
  for (const auto & p : m) {
    o << "( " << p.first << ", " << p.second << ") ";
  }
  o << "}";
  return o;
}

template <typename T, typename U>
std::ostream & operator << (std::ostream & o, const std::unordered_map<T, U> & m) {
  o << "{ ";
  for (const auto & p : m) {
    o << "( " << p.first << ", " << p.second << ") ";
  }
  o << "}";
  return o;
}

void test_vector_round_trip() {
  lua_raii L;

  round_trip_value(L, std::vector<int>{}, __LINE__);
  round_trip_value(L, std::vector<int>{1,5, -1, 2932}, __LINE__);
  round_trip_value(L, std::vector<int>{99999,99999}, __LINE__);
  round_trip_value(L, std::vector<float>{-0.5f, .5f, 6.5f}, __LINE__);
  round_trip_value(L, std::vector<std::string>{"asdf", "jkl;", "", "wer"}, __LINE__);
}

void test_array_round_trip() {
  lua_raii L;

  round_trip_value(L, std::array<int,0>{{}}, __LINE__);
  round_trip_value(L, std::array<int,4>{{1,5, -1, 2932}}, __LINE__);
  round_trip_value(L, std::array<int,2>{{99999,99999}}, __LINE__);
  round_trip_value(L, std::array<float,3>{{-0.5f, .5f, 6.5f}}, __LINE__);
  round_trip_value(L, std::array<std::string, 4>{{"asdf", "jkl;", "", "wer"}}, __LINE__);
}

void test_map_round_trip() {
  lua_raii L;

  round_trip_value(L, std::map<std::string, std::string>{}, __LINE__);
  round_trip_value(L, std::map<std::string, std::string>{{"a", "jk"}, {"l", "wer"}, {"_@34", "wasd"}}, __LINE__);
  round_trip_value(L, std::map<std::string, int>{{"a", 1}, {"b", 5}, {"", 42}}, __LINE__);
  round_trip_value(L, std::map<std::string, int>{{"a", 1}, {"b", 5}, {"", 42}}, __LINE__);
  round_trip_value(L, std::map<int, std::vector<std::string>>{{1, {}}, {2, {}}, {3, {"a", "b", "c"}}, {9, {"asdf", "jkl;"}}}, __LINE__); 
}

void test_set_round_trip() {
  lua_raii L;

  round_trip_value(L, std::set<int>{}, __LINE__);
  round_trip_value(L, std::set<int>{1, 5, 6, 10, 234}, __LINE__);
  round_trip_value(L, std::set<std::string>{"wer", "qWQE", "asjdkljweWERWERE", "", "foo"}, __LINE__);
  round_trip_value(L, std::set<bool>{true}, __LINE__);
}

/***
 * Userdata tests
 */

struct userdata_test {
  std::vector<std::string> list;
};

primer::result call_method(lua_State * L, userdata_test & u, std::string arg) {
  u.list.emplace_back(std::move(arg));
  primer::push(L, static_cast<unsigned int>(u.list.size()));
  return 1;
}

primer::result dump_method(lua_State * L, userdata_test & u) {
  primer::push(L, u.list);
  return 1;
}

constexpr const luaL_Reg method_list[] = {
  {"__call", PRIMER_ADAPT(&call_method)},
  {"dump", PRIMER_ADAPT(&dump_method)},
  {nullptr, nullptr}
};

// Register it
namespace primer {
namespace traits {

template <>
struct userdata<userdata_test> {
  static constexpr const char * name = "userdata_test_type";
  static constexpr const luaL_Reg * const methods = static_cast<const luaL_Reg*>(method_list);
};

} // end namespace traits
} // end namespace primer

static_assert(primer::traits::is_userdata<userdata_test>::value, "our test userdata type did not count as userdata!");

void test_userdata() {
  lua_raii L;

  // Install base library
  luaL_requiref(L, "", luaopen_base, 1);
  lua_pop(L, 1); // remove lib

  CHECK_STACK(L, 0);

  const char * const script = ""
  "obj = ...                                      \n"
  "assert(1 == obj('asdf'))                       \n"
  "assert(2 == obj('jkl;'))                       \n"
  "assert(3 == obj('awad'))                       \n"
  "assert(4 == obj('waka'))                       \n"
  "local d = obj:dump()                           \n"
  "assert(4 == #d)                                \n"
  "assert('asdf' == d[1])                         \n"
  "assert('jkl;' == d[2])                         \n"
  "assert('awad' == d[3])                         \n"
  "assert('waka' == d[4])                         \n";

  primer::push_udata<userdata_test>(L);
  TEST_EQ(LUA_OK, luaL_loadstring(L, script));
  lua_pushvalue(L, 1);

  CHECK_STACK(L, 3);

  auto result = primer::fcn_call_no_ret(L, 1);
  TEST_EXPECTED(result);

  CHECK_STACK(L, 1);

  TEST(primer::test_udata<userdata_test>(L, 1), "did not recover userdata from stack");

  auto ref = primer::read<userdata_test &>(L, 1);
  TEST_EXPECTED(ref);

  TEST_EQ(ref->list.size(), 4);
  TEST_EQ(ref->list[0], "asdf");
  TEST_EQ(ref->list[1], "jkl;");
  TEST_EQ(ref->list[2], "awad");
  TEST_EQ(ref->list[3], "waka");
}

/***
 * Second test
 */

struct vec2_test {
  float x;
  float y;

  primer::result add(lua_State *, vec2_test & other) {
    x += other.x;
    y += other.y;
    return 0;
  }

  primer::result subtract(lua_State *, vec2_test & other) {
    x -= other.x;
    y -= other.y;
    return 0;
  }

  primer::result negate(lua_State *) {
    x = -x;
    y = -y;
    return 0;
  }

  primer::result less_than(lua_State * L, vec2_test & other) {
    lua_pushboolean(L, (x < other.x) || (x == other.x && y < other.y));
    return 1;
  }

  primer::result norm(lua_State * L) {
    lua_pushnumber(L, (x * x) + (y * y));
    return 1;
  }

  primer::result dump(lua_State * L) {
    lua_pushnumber(L, x);
    lua_pushnumber(L, y);
    return 2;
  }
};

constexpr const luaL_Reg vec2_methods[] = {
  { "__add", PRIMER_ADAPT_USERDATA(vec2_test, &vec2_test::add)},
  { "__sub", PRIMER_ADAPT_USERDATA(vec2_test, &vec2_test::subtract)},
  { "__unm", PRIMER_ADAPT_USERDATA(vec2_test, &vec2_test::negate)},
  { "__lt",  PRIMER_ADAPT_USERDATA(vec2_test, &vec2_test::less_than)},
  { "norm",  PRIMER_ADAPT_USERDATA(vec2_test, &vec2_test::norm)},
  { "dump",  PRIMER_ADAPT_USERDATA(vec2_test, &vec2_test::dump)},
  { "__gc",  nullptr },
  { nullptr, nullptr }
};

namespace primer {
namespace traits {

template <>
struct userdata<vec2_test> {
  static constexpr const char * name = "vec2";
  static constexpr const luaL_Reg * const methods = static_cast<const luaL_Reg * const>(vec2_methods);
};

} // end namespace traits
} // end namespace primer

static_assert(primer::traits::is_userdata<vec2_test>::value, "our test userdata type did not count as userdata!");

primer::result vec2_ctor(lua_State * L, float x, float y) {
  primer::push_udata<vec2_test>(L, x, y);
  return 1;
}

void test_userdata_two() {
  lua_raii L;

  // Install base library
  luaL_requiref(L, "", luaopen_base, 1);
  lua_pop(L, 1); // remove lib

  // Install vec2 ctor
  lua_pushcfunction(L, PRIMER_ADAPT(&vec2_ctor));
  lua_setglobal(L, "vec2");

  CHECK_STACK(L, 0);

  const char * const script = ""
  "local v1 = vec2(1, 1)                          \n"
  "v1 + vec2(3, 4)                                \n"
  "local x,y = v1:dump()                          \n"
  "assert(x == 4)                                 \n"
  "assert(y == 5)                                 \n"
  "assert(v1:norm() == 41)                        \n"
  "v1 + - vec2(3, 4)                              \n"
  "assert(v1:norm() == 2)                         \n"
  "v1 - vec2(3, 3)                                \n"
  "assert(v1:norm() == 8)                         \n"
  "assert(v1 < vec2(5, 5))                        \n";

  TEST_EQ(LUA_OK, luaL_loadstring(L, script));

  CHECK_STACK(L, 1);

  auto result = primer::fcn_call_no_ret(L, 0);
  TEST_EXPECTED(result);

  CHECK_STACK(L, 0);
}



int main() {
  conf::log_conf();

  std::cout << "Std container tests:" << std::endl;
  test_harness tests{
    {"vector push", &test_vector_push},
    {"array push", &test_array_push},
    {"map push", &test_map_push},
    {"set push", &test_set_push},
    {"vector roundtrip", &test_vector_push},
    {"array roundtrip", &test_array_push},
    {"map roundtrip", &test_map_push},
    {"set roundtrip", &test_set_push},
    {"userdata", &test_userdata},
    {"userdata two", &test_userdata},
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
