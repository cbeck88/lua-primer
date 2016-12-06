#include <primer/api.hpp>
#include <primer/primer.hpp>

#include <cstdlib>
#include <iostream>
#include <string>

using uint = unsigned int;

#define ASSERT(X)                                                              \
  if (!(X)) {                                                                  \
    std::cerr << "Assertion failed [" << __FILE__ << __LINE__ << "]: "         \
              << #X << std::endl;                                              \
    std::abort();                                                              \
  }

//[ primer_tutorial_api_example2

//` Now, we'll show how userdata works in conjunction with serialization,
//` by bringing back the `token` userdata type.
//`
//` The main thing that's new is, we need to register the userdata using an
//` API feature, and we need to provide a `__persist` metamethod.

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

// persistence method
// Given a token, it pushes a lua closure which *yields* the
// same token back when called. Eris can't serialize the token
// directly, but it can serialize this closure, because it knows how to serialize
// the `int` up-value, and `token_ctor` goes into the permanent objects table.
primer::result
impl_token_persist(lua_State * L, token & tok) {
  primer::push(L, tok.id);
  lua_pushcclosure(L, PRIMER_ADAPT(&token_ctor), 1);
  return 1;
}

// userdata declaration, with both metatable and permanent ojects
namespace primer {
namespace traits {

template <>
struct userdata<token> {
  static constexpr const char * name = "token";

  static constexpr std::array<luaL_Reg, 2> metatable() {
    return {{{"__index", PRIMER_ADAPT(&impl_token_index)},
             {"__persist", PRIMER_ADAPT(&impl_token_persist)}}};
  }

  static constexpr std::array<luaL_Reg, 1> permanents() {
    return {{{"ctor", PRIMER_ADAPT(&token_ctor)}}};
  };
};

} // end namespace traits
} // end namespace primer

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
  API_FEATURE(api::userdatas<token>, userdata_types_);

  NEW_LUA_CALLBACK(new_token)(lua_State * L)->primer::result {
    primer::push_udata<token>(L, count_.get()++);
    return 1;
  }

  NEW_LUA_CALLBACK(inspect_token)(lua_State *, token & tok)->primer::result {
    std::cout << "Token id: " << tok.id << std::endl;
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

  void deserialize(const std::string & str) { ASSERT(this->unpersist(lua_, str)); }
};

// Test that we can make two separate, encapsulated copies of the api object,
// and serialize and deserialize them.

int
main() {
  my_api api1;
  api1.run_script(
    "x = new_token() "
    "y = new_token() "
    "inspect_token(x) "
    "inspect_token(y) ");

  my_api api2;
  api2.run_script(
    "z = new_token() "
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

//` The two new functions we have which we didn't have before are the
//` `token_ctor`
//` and the `persist` implementation. `token_ctor` is like a constructor or
//` factory function for `token`'s, but adapted for lua.
//`  
//` Notably, it takes it's argument from `lua_upvalueindex(1)`
//` rather than from the stack. This is because it is always used as a closure.
//`  
//` When eris serializes the lua state and encounters a `token` userdata, it
//` inspects the `__persist` metamethod. Since the value is a function, it calls
//` that function, expecting to find a closure object which will be serialized.
//` That closure object, when called, yields the original `token` userdata value.
//`  
//` This approach reduces the problem of serializing userdata to serializing a
//` closure. To serialize a closure, we need to be able to serialize the
//` function, and all the up-values.
//` 
//` We can serialize the function because `token_ctor` is in the permanents list.
//` If it wasn't, we would get a runtime error on failing to serialize the
//` function.
//`
//` Note that nothing says we have to use a "C closure", we could
//` concievably use a function written in lua rather than a function written
//` in C++ -- the lua closure could concievably call other parts of our API,
//` since it needs to somehow produce a new userdata value. Eris can serialize
//` functions written in lua just fine, without any assistance.  
//`
//` In the typical
//` case, though, it's easier for these things to be implemented in C++ and to
//` just put them in the permanent object's table.
//`
//` We can serialize the up-values here because it's just an integer. In general, the
//` up-values could be userdata also, and then the process would be repeated
//` recursively. Eventually, every table, closure, or other composite object
//` gets broken down into either primitive values which can be directly
//` serialized, or into values in the permanent objects table.
//`
//` When deserializing, eris requires you to provide the "reverse" permanent
//` objects table, which maps the "stand-in" values back to the "true" values.
//` When using `primer`, you don't have to do this explicitly, it does it
//` automatically based on what API features are registered and what permanent
//` objects they declare.

//`  
//` One thing other thing you might notice in the above code -- in earlier
//` examples we used a `static constexpr`
//` data member `metatable` in the userdata declaration. In this example, we
//` used a `static constexpr` function instead.
//`
//` Primer is okay with either form, but sometimes you need to use a function
//` for the program to link.
//`
//` (If I understand right, the standard doesn't actually say that
//` the first version should link until C++17, due to changes in the ODR,
//` and the introduction of "inline variables", but gcc and clang mostly do
//` anyways...)
//`
//` The function format conceptually slightly less clear perhaps, but it's
//` usually better and more useful. For instance, it
//` allows for better encapsulation in common cases, when you want to declare a
//` userdata type in the header, and specialize the trait there, and conceal all
//` the method implementations in a cpp file.


//]

// TODO: Maybe put this back if literal persistence is fixed, see tutorial5.cpp

//` Another thing worth noting in this implementation is that although we went
//` through the
//` most general form of persisting userdata, with a `__persist` metamethod and
//` a closure,
//` in the end those functions didn't really do any work, they just wrote down
//` the int,
//` and reinstalled the int within the `token` struct. In other words, we just
//` did "literal"
//` userdata persistence -- we could equally well have just `memcpy`'d the block
//` into our
//` data file essentially.
//`
//` Eris has a special mode to support literal userdata persistence. You simply
//` set the
//` "__persist" entry to `true` instead of to a function.
//`
//` To do this in `primer` we need to drop out of the `luaL_Reg` sequence (since
//` that locks us in
//` to using only functions.) Instead, we can use full manual control for the
//` metatable, and
//` rewrite the above program like so:
