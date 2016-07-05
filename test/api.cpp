#include <primer/api/callback_manager.hpp>
#include <primer/api/base.hpp>
#include <primer/api/library.hpp>
#include <primer/api/userdata_registrar.hpp>
#include <primer/detail/make_array.hpp>
#include <primer/support/function.hpp>
#include <primer/support/userdata_dispatch.hpp>
#include <primer/lua.hpp>
#include <primer/std/vector.hpp>

#include "test_harness.hpp"
#include <iostream>
#include <string>
#include <initializer_list>

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

  NEW_LUA_CALLBACK(f, "this is the f help")(lua_State * L, int i, int j) -> primer::result {
    if (i < j) {
      lua_pushinteger(L, -1);
    } else if (i > j) {
      lua_pushinteger(L, 1);
    } else {
      return primer::error("bad bad");
    }
    return 1;
  }

  NEW_LUA_CALLBACK(g, "this is the g help")(lua_State * L, std::string i, std::string j) -> primer::result {
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

/***
 * Test userdata persistence
 */

struct tstring {
  std::vector<std::string> strs;

  // These Ctors should not be necessary but they are to help older compilers
  tstring() = default;
  tstring(const tstring &) = default;
  tstring(tstring &&) = default;

  explicit tstring(std::vector<std::string> s)
    : strs(std::move(s))
  {}

  // Methods
  std::string to_string() {
    std::string result;

    bool first = true;
    for (const auto & s : strs) {
      if (first) {
        first = false;
      } else {
        result += " .. ";
      }
      result += "_('" + s + "')";
    }

    return result;
  }

  tstring operator+(const tstring & other) const {
    std::vector<std::string> result{strs};
    result.insert(result.end(), other.strs.begin(), other.strs.end());
    return tstring{std::move(result)};
  }

  // Lua
  primer::result intf_to_string(lua_State * L) {
    primer::push(L, this->to_string());
    return 1;
  }

  primer::result intf_concat(lua_State * L, tstring & other) {
    primer::push_udata<tstring>(L, *this + other);
    return 1;
  }

  static primer::result intf_create(lua_State * L, std::string s) {
    primer::push_udata<tstring>(L, std::vector<std::string>{std::move(s)});
    return 1;
  }

  static int intf_reconstruct(lua_State * L) {
    if (auto strs =
          primer::read<std::vector<std::string>>(L, lua_upvalueindex(1))) {
      primer::push_udata<tstring>(L, std::move(*strs));
      return 1;
    } else {
      lua_pushstring(L, strs.err_c_str());
    }
    return lua_error(L);
  }

  int intf_persist(lua_State * L) {
    primer::push(L, strs);
    lua_pushcclosure(L, PRIMER_ADAPT(&intf_reconstruct), 1);
    return 1;
  }
};

namespace primer {
namespace traits {

template <>
struct userdata<tstring> {
  static constexpr const char * const name = "tstring";
  static const luaL_Reg * const metatable;
  static const luaL_Reg * const permanents;
};

constexpr auto metatable_array = std::array<luaL_Reg, 3>{
  {{"__concat", PRIMER_ADAPT_USERDATA(tstring, &tstring::intf_concat)},
   {"__persist", PRIMER_ADAPT_USERDATA(tstring, &tstring::intf_persist)},
   {"__tostring", PRIMER_ADAPT_USERDATA(tstring, &tstring::intf_to_string)}}};
constexpr auto permanents_array = std::array<luaL_Reg, 1>{
  {luaL_Reg{"tstring_reconstruct", PRIMER_ADAPT(&tstring::intf_reconstruct)}}};

const luaL_Reg * const userdata<tstring>::metatable = metatable_array.data();
const luaL_Reg * const userdata<tstring>::permanents = permanents_array.data();

} // end namespace traits
} // end namespace primer

static_assert(primer::detail::metatable<tstring>::value == 2,
              "primer didn't recognize our userdata methods!");
static_assert(primer::detail::permanents_helper<tstring>::value == 1,
              "primer didn't recognize our userdata permanents!");

struct test_api_three : primer::api::base<test_api_three> {
  lua_raii L_;

  API_FEATURE(primer::api::libraries<primer::api::lua_base_lib>, libs_);
  API_FEATURE(primer::api::callback_manager, cb_man_);
  API_FEATURE(primer::api::userdata_registrar<tstring>, udata_man_);

  USE_LUA_CALLBACK(_, "creates a translatable string", &tstring::intf_create);

  test_api_three()
    : L_()
    , cb_man_(this)
  {
    this->initialize_api(L_);
  }

  std::string save() {
    std::string result;
    this->persist(L_, result);
    return result;
  }

  void restore(const std::string & buffer) { this->unpersist(L_, buffer); }

  void do_first_script() {
    const char * script =
      "assert(type(_) == 'function')        \n"
      "string1 = _'foo'                     \n"
      "string2 = _'bar'                     \n"
      "assert(type(string1) == 'userdata')  \n"
      "assert(type(string2) == 'userdata')  \n"
      "string3 = string1 .. string2         \n";
    TEST_EQ(LUA_OK, luaL_loadstring(L_, script));
    auto result = primer::fcn_call_no_ret(L_, 0);
    TEST_EXPECTED(result);
  }

  void do_second_script() {
    const char * script =
      "assert(\"_('foo')\" == tostring(string1))                          \n"
      "assert(\"_('bar')\" == tostring(string2))                          \n"
      "assert(\"_('foo') .. _('bar')\" == tostring(string3))              \n";
    TEST_EQ(LUA_OK, luaL_loadstring(L_, script));
    auto result = primer::fcn_call_no_ret(L_, 0);
    TEST_EXPECTED(result);
  }
};

void test_api_userdata() {
  std::string buffer;

  {
    test_api_three a;
    a.do_first_script();
    buffer = a.save();
  }

  {
    test_api_three a;
    a.restore(buffer);
    a.do_second_script();
  }
}

int main() {
  conf::log_conf();

  std::cout << "Persistence tests:" << std::endl;
  test_harness tests{
    {"persist simple", &test_persist_simple},
    {"api base", &test_api_base},
    {"api help", &test_api_help},
    {"api userdata", &test_api_userdata},
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
