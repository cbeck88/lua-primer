#include <cassert>
#include <iostream>
#include <primer/detail/str_cat.hpp>
#include <string>

/***
 * This is a small test of primer str_cat function, meant to help
 * isolate a bug on msvc.
 */

int
main() {
  std::string s1{primer::detail::str_cat("a")};
  assert(s1 == "a");

  std::string s2{primer::detail::str_cat("a", "b")};
  assert(s2 == "ab");

  std::string s3{primer::detail::str_cat("a", 5)};
  assert(s3 == "a5");

  std::string s4{primer::detail::str_cat("a", 5, "b")};
  assert(s4 == "a5b");

  std::cout << "OK!" << std::endl;
}
