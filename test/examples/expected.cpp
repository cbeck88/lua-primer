#include <primer/expected.hpp>

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

int main() {
  auto result = foo(6);
  assert(!result);

  auto result2 = foo(7);
  assert(result2);
  assert(*result2 == "woof!");

  auto result3 = foo(primer::error("404"));
  assert(!result3);
  assert(result3.err_str() == "404");
}
