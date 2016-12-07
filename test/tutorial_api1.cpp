#include <primer/api.hpp>
#include <primer/primer.hpp>

#include <cstdlib>
#include <iostream>
#include <string>

using uint = unsigned int;

#define ASSERT(X)                                                              \
  if (!(X)) {                                                                  \
    std::cerr << "Assertion failed [" << __FILE__ << __LINE__ << "]: " << #X   \
              << std::endl;                                                    \
    std::abort();                                                              \
  }

//[ primer_tutorial_api_example_1

//` Now, let's develop this example and illustrate a way of making the AI
//` have persistent state outside of lua.

struct lua_raii {
  lua_State * L_;

  lua_raii()
    : L_(luaL_newstate()) {}

  ~lua_raii() { lua_close(L_); }

  operator lua_State *() const { return L_; }
};

namespace api = primer::api;

struct my_api : api::base<my_api> {

  lua_raii lua_;

  API_FEATURE(api::callbacks, callbacks_);
  API_FEATURE(api::persistent_value<int>, count_);

  NEW_LUA_CALLBACK(next_number)(lua_State * L)->primer::result {
    primer::push(L, count_.get()++);
    return 1;
  }

  NEW_LUA_CALLBACK(print_number)(lua_State *, int i)->primer::result {
    std::cout << "The number was: " << i << std::endl;
    return 0;
  }

  my_api()
    : lua_()
    , callbacks_(this)
    , count_{0} {
    ASSERT(this->initialize_api(lua_));
  }

  void run_script(const char * script) {
    lua_settop(lua_, 0);
    if (LUA_OK != luaL_loadstring(lua_, script)) {
      std::cerr << lua_tostring(lua_, -1);
      std::abort();
    }

    if (LUA_OK != lua_pcall(lua_, 0, 0, 0)) {
      std::cerr << lua_tostring(lua_, -1);
      std::abort();
    }
  }

  std::string serialize() {
    std::string result;
    ASSERT(this->persist(lua_, result));
    return result;
  }

  void deserialize(const std::string & str) {
    ASSERT(this->unpersist(lua_, str));
  }
};

// Test that we can make two separate, encapsulated copies of the api object,
// and serialize and deserialize them.

int
main() {
  my_api api1;
  api1.run_script(
    "x = next_number() "
    "y = next_number() "
    "print_number(x) "
    "print_number(y) ");

  my_api api2;
  api2.run_script(
    "z = next_number() "
    "x = next_number() "
    "print_number(x) ");

  api1.run_script("print_number(x)");

  std::string s = api1.serialize();

  api2.run_script("print_number(x)");
  api2.deserialize(s);
  api2.run_script("print_number(x)");
}

//` There are six calls to `print_number` here: they print respectively
//= The number was: 0
//= The number was: 1
//= The number was: 1
//= The number was: 0
//= The number was: 1
//= The number was: 0
//`

//` Recall that in our "token" api, the `next_token` function had a static
//` local variable to serve as the counter. when we use primer usually we
//` don't want that -- we want everything about the VM to be encapsulated in
//` an object, and we want `api1` and `api2` in the example above to have
//` independent
//` counters. In primer, one way to do that is to make such state be a member of
//` the API object. Here, we used `api::persistent_value<int>` for the counter.
//`
//` The `api::persistent_value` template creates a structure with one data
//` member,
//` which can be accessed via `get()`. When it is registered using
//` `API_FEATURE`,
//` it gets to participate in serialization and deserialization. It does this by
//` pushing its name and its value, using `primer::push`, so that it gets
//` serialized
//` by eris along with the global table. On deserialization, it recovers the
//` value.
//` This is sometimes a convenient way of creating C++ objects associated to
//` your
//` API that get serialized. Any data structure which can be used with both
//` `primer::push` and `primer::read` can be used with `persistent_value`.

//]
