#include <primer/primer.hpp>

#include <cassert>
#include <iostream>
#include <string>

using uint = unsigned int;


//[ primer_tutorial_userdata_example_4

//` [h3 Userdata Member Functions]

//` Besides registering free functions, sometimes we'd like to register methods
//` (member functions) of userdata types -- it may simply be more elegant that way.
//` You can do that using a special version of `PRIMER_ADAPT`.

struct bar {
  std::string str;
  
  primer::result foo(lua_State *, const char * s) {
    str += s;
    return 0;
  }
};

namespace primer {
namespace traits {

template <>
struct userdata<bar> {
  static constexpr const char * name = "bar";

  static constexpr std::array<luaL_Reg, 1> metatable {{
    {"foo", PRIMER_ADAPT_USERDATA(bar, &bar::foo)}
  }};
};

} // end namespace traits
} // end namespace primer

//<-
namespace {
//->

primer::result new_bar(lua_State * L, const char * s) {
  primer::push_udata<bar>(L, s);
  return 1;
}

//<-
void userdata_test_five() {
  lua_State * L = luaL_newstate();
  luaL_requiref(L, "", &luaopen_base, 1);
//->
  // ...

  lua_pushcfunction(L, PRIMER_ADAPT(&new_bar));
  lua_setglobal(L, "new_bar");

  const char * script = ""
  "x = new_bar('a') "
  "x:foo('b') "
  "x:foo('c') ";
 
//=  assert(LUA_OK == luaL_loadstring(L, script));
  if (LUA_OK != luaL_loadstring(L, script)) {
    std::cerr << lua_tostring(L, -1) << std::endl;
    assert(false);
  }
//=  assert(LUA_OK == lua_pcall(L, 0, 0, 0));
//<-
  if (LUA_OK != lua_pcall(L, 0, 0, 0)) {
    std::cerr << lua_tostring(L, -1) << std::endl;
    assert(false);
  }
//<-
  lua_close(L);
}

} // end anonymous namespace
//->

//` The above example works because, by default, unless you provide an `__index` function,
//` primer sets __index to refer to the metatable itself -- so `.foo` obtains the function `foo` which we registered.
//` (This is an extremely common technique in lua C api programming. If you really want to have no `__index` function,
//` you should specifiy `nullptr`. See reference for more detail.)
//`
//` Another example, using the token class again, to do something like "operator overloading".
//` Suppose we don't want the `id` value to be directly visible from lua, but we want lua to be able to sort tokens.



//` When using primer, some steps above become unnecessary. Particularly, `init_token_metatable` happens implicitly.
//`  
//` It's necessary for primer to know how to set up the metatables properly -- basically it lets initialization happen
//` lazily, while also allowing serialization and deserialization to happen with less work on your part.

struct token {
  int id;
  
  bool operator < (const token & other) const {
    return this->id < other.id;
  }

  primer::result lua_less(lua_State * L, const token & other) const {
    primer::push(L, *this < other);
    return 1;
  }
};

namespace primer {
namespace traits {

template <>
struct userdata<token> {
  static constexpr const char * name = "token";

  static constexpr std::array<luaL_Reg, 1> metatable {{
    {"__lt", PRIMER_ADAPT_USERDATA(token, &token::lua_less)}
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
void userdata_test_six() {
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
  "if x < y then print('x < y') end "
  "y = x "
  "if not (x < y) and not (y < x) then print('x == y') end ";
 
//=  assert(LUA_OK == luaL_loadstring(L, script));
  if (LUA_OK != luaL_loadstring(L, script)) {
    std::cerr << lua_tostring(L, -1) << std::endl;
    assert(false);
  }
//=  assert(LUA_OK == lua_pcall(L, 0, 0, 0));
//<-
  if (LUA_OK != lua_pcall(L, 0, 0, 0)) {
    std::cerr << lua_tostring(L, -1) << std::endl;
    assert(false);
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
  userdata_test_five();
  userdata_test_six();
}
