#include <primer/adapt.hpp>
#include <primer/push.hpp>
#include <primer/read.hpp>
#include <primer/result.hpp>
#include <primer/lua_state_ref.hpp>
#include <primer/set_funcs.hpp>

#include "test_harness.hpp"
#include <iostream>
#include <string>

#define CHECK_STACK(L, I) TEST_EQ(lua_gettop(L), I)

void test_top_type(lua_State * L, int expected, int line) {
  if (lua_type(L, 1) != expected) {
    TEST(false, "Expected '" << lua_typename(L, expected) << "', found '"
                             << lua_typename(L, lua_type(L, 1))
                             << "' line: " << line);
  }
}

template <typename T>
void test_push_type(lua_State * L, T t, int expected, int line) {
  CHECK_STACK(L, 0);
  primer::push(L, t);
  CHECK_STACK(L, 1);
  test_top_type(L, expected, line);
  lua_pop(L, 1);
}

void push_type_simple() {
  lua_raii L;

  test_push_type<int>(L, 1, LUA_TNUMBER, __LINE__);
  test_push_type<int>(L, -101, LUA_TNUMBER, __LINE__);
  test_push_type<uint>(L, 9999, LUA_TNUMBER, __LINE__);
  test_push_type<const char *>(L, "asdf", LUA_TSTRING, __LINE__);
  test_push_type<std::string>(L, "jkl;", LUA_TSTRING, __LINE__);
  test_push_type<primer::nil_t>(L, primer::nil_t{}, LUA_TNIL, __LINE__);
  test_push_type<float>(L, 1.5f, LUA_TNUMBER, __LINE__);
  test_push_type<float>(L, 10000.5f, LUA_TNUMBER, __LINE__);
  test_push_type<bool>(L, true, LUA_TBOOLEAN, __LINE__);
  test_push_type<bool>(L, false, LUA_TBOOLEAN, __LINE__);
}

template <typename T>
void test_roundtrip_value(lua_State * L, T t, int line) {
  CHECK_STACK(L, 0);

  primer::push(L, t);

  CHECK_STACK(L, 1);

  auto r = primer::read<T>(L, 1);
  TEST(r, "Could not recover value '" << t << "'. line: " << line);

  TEST_EQ(*r, t);

  CHECK_STACK(L, 1);

  lua_pop(L, 1);
}

void roundtrip_simple() {
  lua_raii L;

  test_roundtrip_value<int>(L, 1, __LINE__);
  test_roundtrip_value<int>(L, 2, __LINE__);
  test_roundtrip_value<int>(L, 3, __LINE__);
  test_roundtrip_value<int>(L, -19, __LINE__);
  test_roundtrip_value<int>(L, 657, __LINE__);

  test_roundtrip_value<std::string>(L, "foo", __LINE__);
  test_roundtrip_value<std::string>(L, "bar", __LINE__);
  test_roundtrip_value<std::string>(L, "", __LINE__);

  test_roundtrip_value<uint>(L, 0, __LINE__);
  test_roundtrip_value<uint>(L, 27, __LINE__);
  test_roundtrip_value<uint>(L, 3, __LINE__);
  test_roundtrip_value<uint>(L, std::numeric_limits<int>::max(), __LINE__);

  test_roundtrip_value<bool>(L, true, __LINE__);
  test_roundtrip_value<bool>(L, false, __LINE__);

  test_roundtrip_value<float>(L, 1, __LINE__);
  test_roundtrip_value<float>(L, 2, __LINE__);
  test_roundtrip_value<float>(L, 3, __LINE__);
  test_roundtrip_value<float>(L, 100.5f, __LINE__);
  test_roundtrip_value<float>(L, -97.75f, __LINE__);
}

template <typename U, typename T>
void test_type_safety(lua_State * L, T t, int line) {
  CHECK_STACK(L, 0);

  primer::push(L, t);

  CHECK_STACK(L, 1);

  auto r = primer::read<U>(L, 1);

  if (r) {
    throw test_exception{
      "Unexpected conversion was permitted by primer: line " +
      std::to_string(line)};
  }

  CHECK_STACK(L, 1);
  lua_pop(L, 1);
}

void typesafe_simple() {
  lua_raii L;

  test_type_safety<std::string>(L, int{1}, __LINE__);
  test_type_safety<int>(L, std::string{"foo"}, __LINE__);
  test_type_safety<bool>(L, "bar", __LINE__);
  test_type_safety<int>(L, 7.5f, __LINE__);
  test_type_safety<bool>(L, int{2}, __LINE__);
  test_type_safety<unsigned int>(L, int{-1}, __LINE__);
  test_type_safety<unsigned int>(L, int{-99}, __LINE__);
  test_type_safety<std::string>(L, 7.5f, __LINE__);
  test_type_safety<int>(L, primer::nil_t{}, __LINE__);
}

/***
 * Test primer adapt
 */

namespace {
int test_result_one;
float test_result_two;
std::string test_result_three;

primer::result test_func_one(lua_State * L, int i, float d, std::string s) {
  test_result_one = i;
  test_result_two = d;
  test_result_three = s;
  lua_pushboolean(L, true);
  return 1;
}
} // end anonymous namespace

void adapt_test_one() {
  lua_raii L;

  lua_CFunction func = PRIMER_ADAPT(&test_func_one);

  lua_pushcfunction(L, func);
  lua_pushinteger(L, 3);
  lua_pushnumber(L, 5.5f);
  lua_pushstring(L, "asdf");

  TEST_EQ(LUA_OK, lua_pcall(L, 3, 1, 0));
  CHECK_STACK(L, 1);
  test_top_type(L, LUA_TBOOLEAN, __LINE__);
  TEST_EQ(lua_toboolean(L, 1), true);
  TEST_EQ(test_result_one, 3);
  TEST_EQ(test_result_two, 5.5f);
  TEST_EQ(test_result_three, "asdf");

  lua_pop(L, 1);
  CHECK_STACK(L, 0);

  lua_pushcfunction(L, func);
  TEST_EQ(LUA_ERRRUN, lua_pcall(L, 0, 1, 0));

  CHECK_STACK(L, 1);
  lua_pop(L, 1);

  lua_pushcfunction(L, func);
  lua_pushinteger(L, 7);
  lua_pushnumber(L, -17.5f);
  lua_pushstring(L, "jkl;");

  TEST_EQ(LUA_OK, lua_pcall(L, 3, 1, 0));
  CHECK_STACK(L, 1);
  TEST_EQ(test_result_one, 7);
  TEST_EQ(test_result_two, -17.5f);
  TEST_EQ(test_result_three, "jkl;");
  lua_pop(L, 1);

  lua_pushcfunction(L, func);
  lua_pushstring(L, "jkl;");
  lua_pushinteger(L, 7);
  lua_pushnumber(L, -17.5f);

  TEST_EQ(LUA_ERRRUN, lua_pcall(L, 3, 1, 0));

  CHECK_STACK(L, 1);
  lua_pop(L, 1);

  lua_pushcfunction(L, func);
  lua_pushnumber(L, 7);
  lua_pushstring(L, "jkl;");
  lua_pushinteger(L, 17.5f);

  TEST_EQ(LUA_ERRRUN, lua_pcall(L, 3, 1, 0));

  CHECK_STACK(L, 1);
  lua_pop(L, 1);

  lua_pushcfunction(L, func);
  lua_pushnumber(L, 7);
  lua_pushstring(L, "jkl;");

  TEST_EQ(LUA_ERRRUN, lua_pcall(L, 2, 1, 0));

  CHECK_STACK(L, 1);
  lua_pop(L, 1);

  lua_pushcfunction(L, func);
  lua_pushnumber(L, 7);
  lua_pushnil(L);
  lua_pushstring(L, "jkl;");

  TEST_EQ(LUA_ERRRUN, lua_pcall(L, 3, 1, 0));

  CHECK_STACK(L, 1);
  lua_pop(L, 1);
}

#define WEAK_REF_TEST(X)                                                       \
  TEST(X, "Unexpected value for lua_state_ref. line: " << __LINE__)

void lua_state_ref_validity() {
  using primer::lua_state_ref;

  lua_state_ref r;
  lua_state_ref s;
  lua_state_ref t;

  WEAK_REF_TEST(!r);
  WEAK_REF_TEST(!s);
  WEAK_REF_TEST(!t);

  {
    lua_raii L;

    r = primer::obtain_state_ref(L);

    WEAK_REF_TEST(r);
    WEAK_REF_TEST(!s);
    WEAK_REF_TEST(!t);

    s = r;

    WEAK_REF_TEST(r);
    WEAK_REF_TEST(s);
    WEAK_REF_TEST(!t);

    lua_State * l = L;
    TEST_EQ(l, r.lock());
    TEST_EQ(l, s.lock());
  }

  WEAK_REF_TEST(!r);
  WEAK_REF_TEST(!s);
  WEAK_REF_TEST(!t);

  {
    lua_raii L;

    t = primer::obtain_state_ref(L);

    lua_State * l = L;
    TEST_EQ(l, t.lock());

    WEAK_REF_TEST(!r);
    WEAK_REF_TEST(!s);
    WEAK_REF_TEST(t);

    s = t;

    WEAK_REF_TEST(!r);
    WEAK_REF_TEST(s);
    WEAK_REF_TEST(t);

    // TEST_EQ(L, s.lock());

    primer::close_state_refs(L);

    WEAK_REF_TEST(!r);
    WEAK_REF_TEST(!s);
    WEAK_REF_TEST(!t);
  }

  WEAK_REF_TEST(!r);
  WEAK_REF_TEST(!s);
  WEAK_REF_TEST(!t);

  {
    lua_raii L;

    primer::close_state_refs(L);

    s = primer::obtain_state_ref(L);

    WEAK_REF_TEST(!r);
    WEAK_REF_TEST(!s);
    WEAK_REF_TEST(!t);

    r = s;

    WEAK_REF_TEST(!r);
    WEAK_REF_TEST(!s);
    WEAK_REF_TEST(!t);
  }

  WEAK_REF_TEST(!r);
  WEAK_REF_TEST(!s);
  WEAK_REF_TEST(!t);
}

int main() {
  conf::log_conf();

  std::cout << "Core tests:" << std::endl;
  test_harness tests{
    {"push type simple values", &push_type_simple},
    {"roundtrip simple values", &roundtrip_simple},
    {"simple type safety", &typesafe_simple},
    {"adapt test one", &adapt_test_one},
    {"lua weak ref validity", &lua_state_ref_validity},
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
