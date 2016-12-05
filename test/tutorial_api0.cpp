#include <primer/api.hpp>
#include <primer/primer.hpp>

#include <cassert>
#include <iostream>
#include <string>

using uint = unsigned int;

//[ primer_tutorial_api_example_0

//` First, as a warmup, we'll give an api which only has some callback functions,
//` and show how the save / restore semantics works exactly.

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

  NEW_LUA_CALLBACK(announce)(lua_State *, const char * str)->primer::result {
    std::cout << "lua: " << str << std::endl;
    return 0;
  }

  my_api()
    : lua_()
    , callbacks_(this) {
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
    "msg1 = 'hello world!' "
    "msg2 = 'good bye world!' "
    "msg = msg1 "
    "announce(msg)");
  
  std::string s = api1.serialize();

  api1.run_script("msg = msg2");
  
  api1.deserialize(s);
  
  api1.run_script("announce(msg)");
  api1.run_script("msg = msg2");
  api1.run_script("announce(msg)");
}

//`

//` A few things of note in the implementation:
//`
//` * Callbacks are declared and defined using a macro `NEW_API_CALLBACK`.
//`   The C++11 trailing return type syntax is used so that you can control
//`   the return type without invading the macro.
//`   The macro registers the callback at compile-time within the primer
//`   framework.
//` * Member variables of the API object which need to hook into the persistence
//`   system
//`   are registered using the `API_FEATURE` macro. This takes the type and the
//`   name of the member, and registers it within the primer framework. By
//`   registering, it gets to participate in calls `serialize() ` and
//`   `deserialize()`.
//` * The `api::callbacks` feature is responsible for actually pushing our
//`   callback functions into the global environment, and ensuring that they get
//`   persisted properly. It provies some member functions so that you can make
//`   queries of it. It must be initialized with
//`  `this` in order to actually access the registered callbacks.
//`
//` This simple program calls "announce" three times, and prints the following:
//=  lua: hello world!
//=  lua: hello world!
//=  lua: goodbye world!
//`  The second line is `hello world` and not `goodbye world` because when the
//`  state gets serialized, `msg = msg2` hasn't happened yet. When it gets restored,
//`  the entire global environment gets reset to the saved state. Only the second
//`  line `msg = msg2` affects the state at the time of `announce.
//`
//` Another thing worth mentioning -- when callbacks are registered, they are
//` automatically pushed into the global environment by `initialize_api`. However,
//` the functions can be renamed or removed after this, and upon unpersisting,
//` those renames will still be in effect. The global enviornment -- that is,
//` the values of all variables visible to scripts -- are supposed to be exactly
//` the same after a state is deserialized.


//]
