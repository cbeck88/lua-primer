#include <cassert>
#include <iostream>
#include <primer/expected.hpp>
#include <string>
#include <utility>

/***
 * This is a small test of primer::expected, meant to help
 * isolate a bug on msvc.
 */

using primer::expected;

int
main() {
  expected<int> a;
  expected<float> b;
  expected<std::string> c;

  c = std::string{"foo"};
  c = std::string{"bar"};

  expected<std::string> d{"baz"};

  c = d;

  d = std::move(c);
  c = d;
  d = std::move(c);
  c = d;
  d = std::string{"qaz"};
  c = std::move(d);

  std::cout << "OK!" << std::endl;
}
