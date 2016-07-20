#include <primer/primer.hpp>

#include "test_harness/conf.hpp"
#include "test_harness/lua_raii.hpp"
#include "test_harness/test.hpp"

#include <cassert>
#include <initializer_list>
#include <string>

using primer::expected;

expected<std::string>
foo(expected<int> e) {
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

void
test_foo() {
  auto result = foo(6);
  assert(!result);

  auto result2 = foo(7);
  assert(result2);
  assert(*result2 == "woof!");

  auto result3 = foo(primer::error("404"));
  assert(!result3);
  assert(result3.err_str() == "404");
}

void
test_bar() {
  lua_raii L;

  primer::push(L, 17);
  auto result = primer::read<primer::stringy>(L, 1);
  assert(result);

  assert("17" == result->value);

  auto result2 = primer::read<int>(L, 1);
  assert(result2);

  auto result3 = foo(result2);
  assert(result3);
  assert("woof!" == *result3);
}

int
main() {
  conf::log_conf();
  std::cout << "Noexcept tests:" << std::endl;

  std::initializer_list<test_record> tests{{"first", &test_foo},
                                           {"second", &test_bar}};

  for (const auto & t : tests) {
    t.run();
    t.report_okay();
  }

  std::cout << "\nAll tests passed!\n";
  std::cout << std::endl;
}
