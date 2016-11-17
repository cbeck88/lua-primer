#include <primer/error.hpp>
#include <cassert>
#include <iostream>
#include <string>

/***
 * This is a mininmal test of primer::error, meant to help
 * isolate a bug on msvc.
 */

using primer::error;

error
foo(int e) {
  if (e >= 7) {
    return primer::error{"woof!"};
  } else {
    return primer::error{"bad doggie!"};
  }
}

int main() {
  auto result = foo(6);
  assert(result.str() == "bad doggie!");

  auto result2 = foo(7);
  assert(result2.str() == "woof!");

  std::cout << "OK!" << std::endl;
}
