#include <primer/primer.hpp>
#include <primer/api.hpp>

#include <cassert>
#include <iostream>
#include <string>

using uint = unsigned int;

//[ primer_tutorial_api_example

// Good old token object again

struct token {
  int id;
};

primer::result
impl_token_index(lua_State * L, token & tok, const char * str) {
  if (0 == std::strcmp(str, "id")) {
    primer::push(L, tok.id);
    return 1;
  }
  return 0;
}

// "ctor" for token type, constructs it from an int, passed as first up-value.
// This isn't part of our API directly, but it's needed for persistence
primer::result
token_ctor(lua_State * L) {
  int id = luaL_checkinteger(L, lua_upvalueindex(1));
  primer::push_udata<token>(L, id);
  return 1;
}

// persistence method. Given a token, it pushes a closure formed by taking the
// constructor and closing it over the current id.
primer::result
impl_token_persist(lua_State * L, token & tok) {
  primer::push(L, tok.id);
  lua_pushcclosure(L, PRIMER_ADAPT(&token_ctor), 1);
  return 1;
}


// In this example we changed from a static constexpr data member `metatable`
// to a static constexpr function `metatable`
//
// Primer is okay with either form, but sometimes you need to use a function
// for the program to link. 
//
// (If I understand right, the compiler is not
// required to link the first version until C++17, due to changes in the ODR,
// but gcc and clang mostly do anyways...)
//
// The function format conceptually slightly less clear perhaps, but also this
// allows for better encapsulation in common cases, if you want to declare a
// userdata type in the header, and specialize the trait there, and conceal all
// the method implementations in a cpp file. 

namespace primer {
namespace traits {

template <>
struct userdata<token> {
  static constexpr const char * name = "token";

  static constexpr std::array<luaL_Reg, 2> metatable() { 
    return {{
      {"__index", PRIMER_ADAPT(&impl_token_index)},
      {"__persist", PRIMER_ADAPT(&impl_token_persist)}
    }};
  }

  static constexpr std::array<luaL_Reg, 1> permanents() {
    return {{{"ctor", PRIMER_ADAPT(&token_ctor)}}};
  };
};

} // end namespace traits
} // end namespace primer

// lua_raii is a simple raii object that manages a lua state, for this demo

struct lua_raii {
  lua_State * L_;
  
  lua_raii()
    : L_(luaL_newstate())
  {}
  
  ~lua_raii() { lua_close(L_); }
  
  operator lua_State * () const { return L_; }
};

// the api object encapsulates all the callbacks and state of the api that we
// expose, in this case it owns the lua state also.

namespace api = primer::api;

struct my_api :api::base<my_api> {

  lua_raii lua_;

  API_FEATURE(api::callbacks, callbacks_);
  API_FEATURE(api::persistent_value<int>, count_);
  API_FEATURE(api::userdatas<token>, userdata_types_);

  NEW_LUA_CALLBACK(new_token) (lua_State * L) -> primer::result {
    primer::push_udata<token>(L, count_.get()++);
    return 1;
  }

  NEW_LUA_CALLBACK(inspect_token) (lua_State *, token & tok) -> primer::result {
    std::cout << "Token id: " << tok.id << std::endl;
    return 0;
  }
  
  my_api()
    : lua_()
    , callbacks_(this)
    , count_{0}
  {
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

  void deserialize(const std::string & str) {
    this->unpersist(lua_, str);
  }
};

// Test that we can make two separate, encapsulated copies of the api object,
// and serialize and deserialize them.

int main() {
  my_api api1;
  api1.run_script("x = new_token() "
                  "y = new_token() "
                  "inspect_token(x) "
                  "inspect_token(y) ");
                  
  my_api api2;
  api2.run_script("z = new_token() "
                  "x = new_token() "
                  "inspect_token(x) ");
 

  api1.run_script("inspect_token(x)");
  
  std::string s = api1.serialize();

  api2.run_script("inspect_token(x)");
  api2.deserialize(s);
  api2.run_script("inspect_token(x)");
}

//` There are six calls to inspect here: they print respectively  
//= Token id: 0
//= Token id: 1
//= Token id: 1
//= Token id: 0
//= Token id: 1
//= Token id: 0
//`  

//` A few things of note in the implementation:  
//`  
//` * Callbacks are declared and defined using a macro `NEW_API_CALLBACK`.
//`   The C++11 trailing return type syntax is used so that you can control
//`   the return type without invading the macro.  
//`   The macro registers the callback at compile-time within the primer framework.
//` * Member variables of the API object which requirem persistence support
//`   are registered using the `API_FEATURE` macro. This takes the type and the
//`   name of the member, and registers it within the primer framework. By
//`   registering, it gets to participate in calls `serialize() ` and `deserialize()`.
//`   The `persistent_value` template is used instead of the local `static int`
//`   we had in the old, unserializable version of the API. Because we used `persistent_value`,
//`   the value is different for two different api objects `api1` and `api2` and they
//`   can get incremented independently. And, when the api is serialized and deserialized,
//`   the persistent value gets restored.  
//` * The `api::callbacks` feature is responsible for actually pushing our
//`   callback functions into the global environment, and ensuring that they get
//`   persisted properly. It provies some member functions so that you can make
//`   queries of it. It must be initialized with
//`  `this` in order to actually access the registered callbacks.

//]

// TODO: Maybe put this back if literal persistence is fixed, see tutorial5.cpp

//` Another thing worth noting in this implementation is that although we went through the
//` most general form of persisting userdata, with a `__persist` metamethod and a closure,
//` in the end those functions didn't really do any work, they just wrote down the int,
//` and reinstalled the int within the `token` struct. In other words, we just did "literal"
//` userdata persistence -- we could equally well have just `memcpy`'d the block into our
//` data file essentially.
//`  
//` Eris has a special mode to support literal userdata persistence. You simply set the
//` "__persist" entry to `true` instead of to a function.
//`
//` To do this in `primer` we need to drop out of the `luaL_Reg` sequence (since that locks us in
//` to using only functions.) Instead, we can use full manual control for the metatable, and
//` rewrite the above program like so:
