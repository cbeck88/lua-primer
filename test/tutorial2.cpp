#include <primer/primer.hpp>

#include <cassert>
#include <iostream>
#include <string>

using uint = unsigned int;

//[ primer_tutorial_userdata_example_3

//` When using primer, some steps above become unnecessary. Particularly, `init_token_metatable` happens implicitly.
//`  
//` It's necessary for primer to know how to set up the metatables properly -- basically it lets initialization happen
//` lazily, while also allowing serialization and deserialization to happen with less work on your part.

struct token {
  int id;
};

primer::result impl_token_index(lua_State * L, token & tok, const char * str) {
  if (0 == std::strcmp(str, "id")) {
    primer::push(L, tok.id);
    return 1;
  }
  return 0;
}

namespace primer {
namespace traits {

template <>
struct userdata<token> {
  static constexpr const char * name = "token";

  static constexpr std::array<luaL_Reg, 1> metatable {{
    {"__index", PRIMER_ADAPT(&impl_token_index)}
  }};
};

} // end namespace traits
} // end namespace primer

//<-
namespace {
//->

primer::result new_token(lua_State * L) {
  static int count = 0;
  primer::push_udata<token>(L, count++);
  return 1;
}

primer::result inspect_token(lua_State *, token & tok) {
  std::cout << "Token id: " << tok.id << std::endl;
  return 0;
}


//<-
void userdata_test_four() {
  lua_State * L = luaL_newstate();
  luaL_requiref(L, "", &luaopen_base, 1);
//->
  // ...

  lua_pushcfunction(L, PRIMER_ADAPT(&new_token));
  lua_setglobal(L, "new_token");
  lua_pushcfunction(L, PRIMER_ADAPT(&inspect_token));
  lua_setglobal(L, "inspect_token");

  const char * script = ""
  "x = new_token() "
  "y = new_token() "
  "inspect_token(x) "
  "inspect_token(y) "
  "if x.id < y.id then print('x < y') end "
  "y = x "
  "if x.id == y.id then print('x == y') end ";
 
//=  assert(LUA_OK == luaL_loadstring(L, script));
//<-
  if(LUA_OK != luaL_loadstring(L, script)) {
    std::cerr << lua_tostring(L, -1);
    std::abort();
  }
//->
//=  assert(LUA_OK == lua_pcall(L, 0, 0, 0));
//<-
  if(LUA_OK != lua_pcall(L, 0, 0, 0)) {
    std::cerr << lua_tostring(L, -1);
    std::abort();
  }
//<-
  lua_close(L);
}
//->

//` Usually, it is simplest for `metatable` to be a `constexpr std::array` like above.
//` However you have other options, see the reference section for an exhaustive spec.

//]

} // end anonymous namespace

int main() {
  userdata_test_four();
}
