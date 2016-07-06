#include <primer/expected.hpp>
#include <primer/lua.hpp>
#include <primer/push.hpp>
#include <primer/read.hpp>

#include <conf.hpp>

#include <cassert>
#include <string>

using primer::expected;

expected<std::string> foo(expected<int> e) {
  if (e) {
    if (*e >= 7) {
      return std::string{"woof!"};
    } else {
      return primer::error{"bad doggie!"};
    }
  } else {
    return e.err();
  }
}

void test_foo() {
  auto result = foo(6);
  assert(!result);

  auto result2 = foo(7);
  assert(result2);
  assert(*result2 == "woof!");

  auto result3 = foo(primer::error("404"));
  assert(!result3);
  assert(result3.err_str() == "404");

}


void test_bar() {
  lua_State * L = luaL_newstate();

  primer::push(L, 17);
  auto result = primer::read<primer::stringy>(L, 1);
  assert(result);

  assert("17" == result->value);

  auto result2 = primer::read<int>(L, 1);
  assert(result2);

  auto result3 = foo(result2);
  assert(result3);
  assert("woof!" == *result3);

  lua_close(L);
}

int main() {
  conf::log_conf();
  test_foo();
  test_bar();
}
