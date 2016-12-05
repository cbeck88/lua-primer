#include <primer/api.hpp>
#include <primer/primer.hpp>

#include <cassert>
#include <iostream>
#include <string>

using uint = unsigned int;

//[ primer_tutorial_api_example

//` First, as a warmup, we'll skip the userdata, and just have an API with
//` callbacks and some internal state.

// lua_raii is a simple raii object that manages a lua state, for this demo

struct lua_raii {
  lua_State * L_;

  lua_raii()
    : L_(luaL_newstate()) {}

  ~lua_raii() { lua_close(L_); }

  operator lua_State *() const { return L_; }
};

// the api object encapsulates all the callbacks and state of the api that we
// expose, in this case it owns the lua state also.

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
    this->initialize_api(lua_);
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
    this->persist(lua_, result);
    return result;
  }

  void deserialize(const std::string & str) { this->unpersist(lua_, str); }
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

//` A few things of note in the implementation:
//`
//` * Callbacks are declared and defined using a macro `NEW_API_CALLBACK`.
//`   The C++11 trailing return type syntax is used so that you can control
//`   the return type without invading the macro.
//`   The macro registers the callback at compile-time within the primer
//`   framework.
//` * Member variables of the API object which requirem persistence support
//`   are registered using the `API_FEATURE` macro. This takes the type and the
//`   name of the member, and registers it within the primer framework. By
//`   registering, it gets to participate in calls `serialize() ` and
//`   `deserialize()`.
//`   The `persistent_value` template is used instead of the local `static int`
//`   we had in the old, unserializable version of the API. Because we used
//`   `persistent_value`,
//`   the value is different for two different api objects `api1` and `api2` and
//`   they
//`   can get incremented independently. And, when the api is serialized and
//`   deserialized,
//`   the persistent value gets restored.
//` * The `api::callbacks` feature is responsible for actually pushing our
//`   callback functions into the global environment, and ensuring that they get
//`   persisted properly. It provies some member functions so that you can make
//`   queries of it. It must be initialized with
//`  `this` in order to actually access the registered callbacks.

//]
