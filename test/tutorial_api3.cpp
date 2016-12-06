#include <primer/api.hpp>
#include <primer/primer.hpp>

#include <cassert>
#include <iostream>
#include <string>

using uint = unsigned int;

/***
 * TODO: This example doesn't actually work!
 *       The problem seems to be that when we use literal persistence, and don't
 *       explicitly take care of userdata metatables created using
 * luaL_newmetatable,
 *       they don't get restored by primer -- this makes sense, as the list of
 * those
 *       things is in the registry, how could literal userdata restore it.
 */

//[ primer_tutorial_api_example_2

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

// Making `metatable` a function taking `lua_State *` means "full manual
// control"
// for the metatable. We get a lua_State, with the table which will be the
// metatable
// on the top of the stack, and we modify it as we please.

namespace primer {
namespace traits {

template <>
struct userdata<token> {
  static constexpr const char * name = "token";

  static void metatable(lua_State * L) {
    lua_pushcfunction(L, PRIMER_ADAPT(&impl_token_index));
    lua_setfield(L, -2, "__index");
    lua_pushboolean(L, true);
    lua_setfield(L, -2, "__persist");
  }

  static constexpr std::array<luaL_Reg, 1> permanents() {
    return {{{"__index", PRIMER_ADAPT(&impl_token_index)}}};
  }
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
    assert(this->initialize_api(lua_));
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

//` This is considerably more terse in this case, but in cases of more complex
//objects,
//` the array syntax may save you from a lot of boiler plate that you are forced
//` to write in the metatable version. Also, when you are in manual control,
//it's easier
//` to make mistakes. For instance in this case we didn't set a `__gc`
//metamethod,
//` so in manual control that's a leak, or would be if `token` were not trivial
//and
//` actually required destruction.

//` Note also that when using literal persistence, we need to put
//`impl_token_index`
//` in the permanent objects table, or we get an error on persistence. The
//reason is,
//` when we use literal persistence, eris will also persist the metatable of the
//` userdata type. When we use the closure version, it doesn't actually need to
//do
//` that, and so `impl_token_index` isn't actually ever encountered when eris
//traverses
//` the state -- in the closure version, as soon as we encounter the userdata,
//the
//` whole value is replaced with the result of the closure, and that closure is
//` persisted instead, so the metatable of the original is never encountered.
//` In the literal persistence, it is, and so we still have to put something in
//` the permanent objects table -- either the ctor, or all of the metamethods.

//]
