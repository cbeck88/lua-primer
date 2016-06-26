#include <primer/adapt.hpp>
#include <primer/expected.hpp>
#include <primer/push.hpp>
#include <primer/read.hpp>
#include <primer/result.hpp>
#include <primer/lua_state_ref.hpp>
#include <primer/lua_ref.hpp>
#include <primer/set_funcs.hpp>
#include <primer/support/function.hpp>

#include "test_harness.hpp"
#include <iostream>
#include <string>

#ifdef PRIMER_LUA_AS_CPP
#include <lualib.h>
#else
extern "C" {
#include <lualib.h>
}
#endif

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

void primer_adapt_test_one() {
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
  lua_pushnumber(L, 17.5f);

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

namespace {

primer::result test_func_two(lua_State *) { return primer::error("foo"); }
}

void primer_adapt_test_two() {
  lua_raii L;

  lua_CFunction func = PRIMER_ADAPT(&test_func_two);

  lua_pushcfunction(L, func);
  TEST_EQ(LUA_ERRRUN, lua_pcall(L, 0, 0, 0));

  CHECK_STACK(L, 1);
  lua_pop(L, 1);

  luaL_requiref(L, "", &luaopen_base, 1);
  lua_pop(L, 1);

  lua_pushcfunction(L, func);
  lua_setglobal(L, "f");

  TEST_EQ(LUA_OK, luaL_loadstring(L, "pcall(f()); return true"));
  TEST_EQ(LUA_ERRRUN, lua_pcall(L, 0, 1, 0));

  TEST_EQ(lua_type(L, 1), LUA_TSTRING);
  TEST_EQ(std::string{"foo"}, lua_tostring(L, 1));

  CHECK_STACK(L, 1);
  lua_pop(L, 1);

  TEST_EQ(LUA_OK, luaL_loadstring(L, "pcall(f); return true"));
  TEST_EQ(LUA_OK, lua_pcall(L, 0, 1, 0));

  CHECK_STACK(L, 1);
  TEST_EQ(lua_type(L, 1), LUA_TBOOLEAN);
  TEST_EQ(true, lua_toboolean(L, 1));
}

namespace {

primer::result test_func_three(lua_State * L, int i, int j, bool b) {
  if (i != 5) {
    primer::push(L, i);
    return 1;
  }

  if (j != 7) {
    primer::push(L, j);
    return primer::yield{1};
  }

  if (b) {
    return primer::error{"foo"};
  } else {
    const char * msg = "bar";
    primer::push(L, msg);
    return 1;
  }
}

} // end anonymous namespace

void primer_adapt_test_three() {
  lua_raii L;

  lua_CFunction func = PRIMER_ADAPT(&test_func_three);

  {
    lua_State * T = lua_newthread(L);

    lua_pushcfunction(T, func);
    lua_pushinteger(T, 5);
    lua_pushinteger(T, 7);
    lua_pushboolean(T, false);

    TEST_EQ(LUA_OK, lua_resume(T, nullptr, 3));
    CHECK_STACK(T, 1);
    test_top_type(T, LUA_TSTRING, __LINE__);
    TEST_EQ(std::string{"bar"}, lua_tostring(T, 1));

    lua_pop(L, 1);
  }

  CHECK_STACK(L, 0);

  {
    lua_State * T = lua_newthread(L);

    lua_pushcfunction(T, func);
    lua_pushinteger(T, 3);
    lua_pushinteger(T, 7);
    lua_pushboolean(T, false);

    TEST_EQ(LUA_OK, lua_resume(T, nullptr, 3));
    CHECK_STACK(T, 1);
    test_top_type(T, LUA_TNUMBER, __LINE__);
    TEST_EQ(3, lua_tointeger(T, 1));

    lua_pop(L, 1);
  }

  CHECK_STACK(L, 0);

  {
    lua_State * T = lua_newthread(L);

    lua_pushcfunction(T, func);
    lua_pushinteger(T, 5);
    lua_pushinteger(T, 6);
    lua_pushboolean(T, false);

    TEST_EQ(LUA_YIELD, lua_resume(T, nullptr, 3));
    CHECK_STACK(T, 1);
    test_top_type(T, LUA_TNUMBER, __LINE__);
    TEST_EQ(6, lua_tointeger(T, 1));

    lua_pop(L, 1);
  }

  CHECK_STACK(L, 0);

  {
    lua_State * T = lua_newthread(L);

    lua_pushcfunction(T, func);
    lua_pushinteger(T, 5);
    lua_pushinteger(T, 7);
    lua_pushboolean(T, true);

    TEST_EQ(LUA_ERRRUN, lua_resume(T, nullptr, 3));

    CHECK_STACK(L, 1);
    CHECK_STACK(T, 5);

    test_type(L, 1, LUA_TTHREAD, __LINE__);
    test_type(T, 1, LUA_TNUMBER, __LINE__);
    test_type(T, 2, LUA_TNUMBER, __LINE__);
    test_type(T, 3, LUA_TBOOLEAN, __LINE__);
    test_type(T, 4, LUA_TSTRING, __LINE__);
    test_type(T, 5, LUA_TSTRING, __LINE__);

    TEST_EQ(lua_tostring(T, 4), std::string{"foo"});
    // Last item is the stack trace... this is a property of lua_resume.

    lua_pop(L, 1);
  }

  CHECK_STACK(L, 0);
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

void primer_ref_test() {
  lua_raii L;

  {
    primer::lua_ref dummy;
    TEST(!dummy, "expected empty state to evaluate false");
  }

  lua_pushstring(L, "asdf");
  primer::lua_ref foo{L};
  CHECK_STACK(L, 0);
  TEST(foo, "expected ref to be engaged");

  lua_pushstring(L, "jkl;");
  lua_pushnumber(L, 15);
  primer::lua_ref bar{L};
  CHECK_STACK(L, 1);
  primer::lua_ref baz{L};

  CHECK_STACK(L, 0);

  TEST(bar, "Expected ref to be engaged");
  TEST(baz, "Expected ref to be engaged");

  TEST(baz.push(L), "Expected push to succeed");
  CHECK_STACK(L, 1);
  test_top_type(L, LUA_TSTRING, __LINE__);
  TEST_EQ(std::string{"jkl;"}, lua_tostring(L, -1));
  lua_pop(L, 1);

  TEST(bar.push(), "Expected push to succeed");
  CHECK_STACK(L, 1);
  test_top_type(L, LUA_TNUMBER, __LINE__);
  TEST_EQ(15, lua_tonumber(L, -1));
  lua_pop(L, 1);

  TEST(foo.push(), "Expected push to succeed");
  CHECK_STACK(L, 1);
  test_top_type(L, LUA_TSTRING, __LINE__);
  TEST_EQ(std::string{"asdf"}, lua_tostring(L, -1));
  lua_pop(L, 1);

  // Test reset
  bar.reset();
  TEST(!bar.push(), "Expected ref to be in the empty state!");
  CHECK_STACK(L, 0);

  TEST(baz.push(), "Expected push to succeed");
  CHECK_STACK(L, 1);
  test_top_type(L, LUA_TSTRING, __LINE__);
  TEST_EQ(std::string{"jkl;"}, lua_tostring(L, -1));
  lua_pop(L, 1);

  // Move assign
  bar = std::move(baz);

  TEST(!baz.push(), "Expected ref that has been moved from to be empty!");
  CHECK_STACK(L, 0);

  TEST(bar.push(), "Expected push to succeed");
  CHECK_STACK(L, 1);
  test_top_type(L, LUA_TSTRING, __LINE__);
  TEST_EQ(std::string{"jkl;"}, lua_tostring(L, -1));
  lua_pop(L, 1);

  // Copy assign a disengaged
  baz = bar;

  TEST(bar.push(), "Expected push to succeed");
  CHECK_STACK(L, 1);
  test_top_type(L, LUA_TSTRING, __LINE__);
  TEST_EQ(std::string{"jkl;"}, lua_tostring(L, -1));
  lua_pop(L, 1);

  TEST(baz.push(), "Expected push to succeed");
  CHECK_STACK(L, 1);
  test_top_type(L, LUA_TSTRING, __LINE__);
  TEST_EQ(std::string{"jkl;"}, lua_tostring(L, -1));
  lua_pop(L, 1);

  // Copy assign an engaged
  bar = foo;

  TEST(bar.push(), "Expected push to succeed");
  CHECK_STACK(L, 1);
  test_top_type(L, LUA_TSTRING, __LINE__);
  TEST_EQ(std::string{"asdf"}, lua_tostring(L, -1));
  lua_pop(L, 1);

  TEST(foo.push(), "Expected push to succeed");
  CHECK_STACK(L, 1);
  test_top_type(L, LUA_TSTRING, __LINE__);
  TEST_EQ(std::string{"asdf"}, lua_tostring(L, -1));
  lua_pop(L, 1);


  // Close all refs
  primer::close_state_refs(L);
  TEST(!foo, "Expected all refs to be closed now!");
  TEST(!bar, "Expected all refs to be closed now!");
  TEST(!baz, "Expected all refs to be closed now!");
}

primer::result test_func_four(lua_State * L, int i, int j) {
  lua_pushinteger(L, i + j);
  lua_pushinteger(L, i - j);
  return 2;
}

void primer_call_test() {
  lua_raii L;

  lua_CFunction f1 = PRIMER_ADAPT(&test_func_one);

  {
    lua_pushcfunction(L, f1);
    auto result = primer::fcn_call_no_ret(L, 0);
    CHECK_STACK(L, 0);
    TEST(!result, "expected an error");
  }

  {
    lua_pushcfunction(L, f1);
    lua_pushinteger(L, 3);
    lua_pushnumber(L, 4.5f);
    lua_pushstring(L, "asdf");

    auto result = primer::fcn_call_no_ret(L, 3);
    CHECK_STACK(L, 0);
    TEST(result, "expected success");
  }

  lua_CFunction f2 = PRIMER_ADAPT(&test_func_two);

  {
    lua_pushcfunction(L, f2);
    lua_pushinteger(L, 3);
    auto result = primer::fcn_call_no_ret(L, 1);
    CHECK_STACK(L, 0);
    TEST(!result, "expected failure");
  }

  lua_CFunction f4 = PRIMER_ADAPT(&test_func_four);

  {
    lua_pushcfunction(L, f4);
    lua_pushinteger(L, 5);
    lua_pushinteger(L, 7);

    auto result = primer::fcn_call_one_ret(L, 2);

    CHECK_STACK(L, 0);
    TEST(result, "expected success");
    TEST(result->push(), "expected to be able to push");
    CHECK_STACK(L, 1);
    test_top_type(L, LUA_TNUMBER, __LINE__);
    TEST_EQ(lua_tointeger(L, 1), 12);
    lua_pop(L, 1);
  }

  CHECK_STACK(L, 0);
}

void primer_resume_test() {
  lua_raii L;

  using primer::expected;
  using primer::lua_ref;

  lua_CFunction f1 = PRIMER_ADAPT(&test_func_three);

  lua_State * T = lua_newthread(L);
  {
    TEST_EQ(LUA_OK, lua_status(T));

    lua_pushcfunction(T, f1);
    lua_pushinteger(T, 4);
    lua_pushinteger(T, 7);
    lua_pushboolean(T, true);

    expected<lua_ref> result;
    int code;
    std::tie(result, code) = primer::resume_one_ret(T, 3);

    TEST_EQ(code, LUA_OK);
    TEST(result, "expected success");
    CHECK_STACK(T, 0);
    TEST(result->push(), "expected to be able to push");
    CHECK_STACK(T, 1);
    test_top_type(T, LUA_TNUMBER, __LINE__);
    TEST_EQ(lua_tonumber(T, 1), 4);
    lua_pop(T, 1);
  }

  {
    TEST_EQ(LUA_OK, lua_status(T));

    lua_pushcfunction(T, f1);
    lua_pushinteger(T, 5);
    lua_pushinteger(T, 6);
    lua_pushboolean(T, true);

    expected<lua_ref> result;
    int code;
    std::tie(result, code) = primer::resume_one_ret(T, 3);

    TEST_EQ(code, LUA_YIELD);
    TEST(result, "expected success");
    CHECK_STACK(T, 0);
    TEST(result->push(), "expected to be able to push");
    CHECK_STACK(T, 1);
    test_top_type(T, LUA_TNUMBER, __LINE__);
    TEST_EQ(lua_tonumber(T, 1), 6);
    lua_pop(T, 1);

    TEST_EQ(LUA_YIELD, lua_status(T));

    lua_pushstring(T, "asdf");
    std::tie(result, code) = primer::resume_one_ret(T, 1);
    TEST_EQ(code, LUA_OK);

    CHECK_STACK(T, 0);
  }

  {
    TEST_EQ(LUA_OK, lua_status(T));

    lua_pushcfunction(T, f1);
    lua_pushinteger(T, 5);
    lua_pushinteger(T, 7);
    lua_pushboolean(T, true);

    expected<lua_ref> result;
    int code;
    std::tie(result, code) = primer::resume_one_ret(T, 3);

    TEST_EQ(code, LUA_ERRRUN);
    TEST(!result, "expected an error message");

    CHECK_STACK(T, 0);
  }
}

int main() {
  conf::log_conf();

  std::cout << "Core tests:" << std::endl;
  test_harness tests{
    {"push type simple values", &push_type_simple},
    {"roundtrip simple values", &roundtrip_simple},
    {"simple type safety", &typesafe_simple},
    {"primer adapt test one", &primer_adapt_test_one},
    {"primer adapt test two", &primer_adapt_test_two},
    {"primer adapt test three", &primer_adapt_test_three},
    {"lua state ref validity", &lua_state_ref_validity},
    {"lua value ref validity", &primer_ref_test},
    {"primer call test", &primer_call_test},
    {"primer resume test", &primer_resume_test},
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
