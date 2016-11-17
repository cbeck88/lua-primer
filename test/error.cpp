#include <primer/expected.hpp>
#include <cassert>
#include <iostream>
#include <string>

/***
 * This is a mininmal test of primer::error, primer::expected, meant to help
 * isolate a bug on msvc.
 */

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

int main() {
  auto result = foo(6);
  assert(!result);

  auto result2 = foo(7);
  assert(result2);
  assert(*result2 == "woof!");

  auto result3 = foo(primer::error("404"));
  assert(!result3);
  assert(result3.err().str() == "404");

  std::cout << "OK!" << std::endl;
}
