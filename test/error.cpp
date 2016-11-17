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
foo(int e) {
  if (e >= 7) {
    return std::string{"woof!"};
  } else {
    return primer::error{"bad doggie!"};
  }
}

int main() {
  auto result = foo(6);
  assert(!result);

  auto result2 = foo(7);
  assert(result2);
  assert(*result2 == "woof!");

  std::cout << "OK!" << std::endl;
}
