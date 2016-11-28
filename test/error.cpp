#include <cassert>
#include <iostream>
#include <primer/error.hpp>
#include <string>

/***
 * This is a small test of primer::error, meant to help
 * isolate a bug on msvc.
 */

using primer::error;

int
main() {
  primer::error e1{"aa"};
  assert(e1.str() == "aa");

  primer::error e2{"bb"};
  assert(e2.str() == "bb");

  std::cout << "OK!" << std::endl;
}
