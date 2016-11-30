#include <primer/primer.hpp>
#include <primer/std/vector.hpp>

#include <cassert>
#include <iostream>
#include <string>

using uint = unsigned int;

void push_read() {
//[ primer_tutorial_push_read_example
//` The most basic operations in `primer` are `push` and `read`.
//`  
//` The standard lua C API provides a number of functions which do things like,
//` push a string onto the stack, test if a value on the stack is a string,
//` fetch a string from the stack. Push an integer onto the stack, test, fetch,
//` etc. These functions have names like `lua_pushstring`, `lua_isstring`,
//` `lua_tostring`.

  lua_State * L = luaL_newstate();

  lua_pushstring(L, "foo");
  assert(lua_isstring(L, 1));
  assert(!lua_isnumber(L, 1));
  assert(lua_tostring(L, 1) == std::string{"foo"});
  lua_pop(L, 1);

//` Primer provides a template function, `primer::push`, which will push a C++
//` object to lua using the appropriate API call `lua_pushX`.
//`

  primer::push(L, 2); // Calls lua_pushinteger
  primer::push(L, 4);
  primer::push(L, true); // Calls lua_pushboolean
  primer::push(L, false);
  primer::push(L, "cattle prod"); // Calls lua_pushstring
  assert(lua_gettop(L) == 5);

//` `primer::read` fulfills the purposes of both `lua_isX` and `lua_toX`.
//` When you read a `T`, you get back an `expected<T>`.

  primer::expected<unsigned int> c = primer::read<unsigned int>(L, 2);
  assert(c);

  auto d = primer::read<std::string>(L, 5);
  assert(d);
  assert(*d == "cattle prod");

//` This is sort of like an
//` `std::optional<T>`, in that you can test it (`operator bool`) to see if it
//` actually has a `T`, and get the `T` using `operator *`. (If present, the `T` is
//` stored on site, not on the heap somewhere, like `optional`.) 
//` However, when `expected<T>` does *not* have a `T` value, it instead contains
//` an error message, accessed using `::err()` method. This error message
//` typically corresponds to a human-readable lua error message -- could not
//` convert value on the stack appropriately, etc.

  if(c) {
    std::cout << "Read an integer: " << *c << std::endl;
  } else {
    std::cout << "Could not read an integer: " << c.err().c_str() << std::endl;
  }

//` Besides primitive types, `push` and `read` can also handle more complex datatypes.
//` For instance, a `std::vector` is translated to a table holding a sequence of values.

  std::vector<int> vec{5, 6, 7, 8};
  primer::push(L, vec);

  assert(lua_type(L, -1) == LUA_TTABLE);

  if (auto result = primer::read<std::vector<unsigned int>>(L, -1)) {
    std::cout << "Found a table of " << result->size() << " positive numbers" << std::endl;
  } else {
    std::cout << "Could not recover table of positive numbers: " << result.err().c_str() << std::endl;
  }
  
//` This behavior with `std::vector` requires including the header `#include <primer/std/vector.hpp>`.
//`  
//` There are similar extension headers for other kinds of data structures. (See reference).
//`  
//` It is also relatively easy to provide overloads for `primer::push` and `primer::read`.
//`  
//` These two functions are used by many of the more complex features of `primer`, so
//` customizing them will make primer "aware" of your datatypes more or less everywhere.

//]

  lua_close(L);
}

//[ primer_tutorial_adapt_example

//` The lua C api provides a function `lua_pushcfunction` which allows you to take a function pointer
//` `int (*)(lua_State *)` and pass it to lua, where scripts can use it like a lua function.

//` When a script calls such a function, your function pointer is called, and the arguments appear on
//` the stack. You have to explicitly use `lua_istring` etc. to test and obtain the arguments, when using
//` the pure C api.

int is_pythag(lua_State * L) {
  int a = luaL_checkinteger(L, 1);
  int b = luaL_checkinteger(L, 2);
  int c = luaL_checkinteger(L, 3);
  
  lua_pushboolean(L, (a * a) + (b * b) == (c * c));
  return 1;
}

//<-
void adapt_one() {
  lua_State * L = luaL_newstate();

//->
  // ...

  lua_pushcfunction(L, &is_pythag);
//<-
  lua_close(L);
}
//->

//` When using primer, you can automate that to some extent, and pass C++ functions directly to lua.

primer::result new_is_pythag(lua_State * L, int a, int b, int c) {
  primer::push(L, (a * a) + (b * b) == (c * c));
  return 1;
}

//<-
void adapt_two() {
  lua_State * L = luaL_newstate();

//->
  // ...

  lua_pushcfunction(L, PRIMER_ADAPT(&new_is_pythag));
//<-
  lua_close(L);
}
//->

//` PRIMER_ADAPT is used to "convert" the primer signature
//` `primer::result (*)(lua_State *, ...)` to the lua signature `int (*)(lua_State *)`.

//`  

//` `PRIMER_ADAPT` generates the boiler-plate which reads the arguments off of the stack,
//` using `primer::read` to do the work. That is: If you implement `primer::read` for your custom
//` data types, then they can be arguments to functions which are adapted this way.

//]

namespace test_one {

//[ primer_tutorial_userdata_example_1

//` `userdata` is a special kind of lua value. It cannot be created directly by lua code -- it can only
//` be created by using C API functions.  

//` A `userdata` is essentially a block of memory, owned by lua,
//` which external code has initialized in some way. Scripts are able to pass this "value" around, storing
//` it in variables or passing it to functions.  

//` Here's a minimal example of userdata -- an opaque "token" type for use with some API.

struct token {
  int id;
};

int new_token(lua_State * L) {
  static int count = 0;
  void * udata = lua_newuserdata(L, sizeof(token));
  new (udata) token{count++};
  return 1;
}

int inspect_token(lua_State * L) {
  if (void * udata = lua_touserdata(L, 1)) {
    std::cout << "Token id: " << static_cast<token*>(udata)->id << std::endl;
  } else {
    std::cout << "Not a userdata" << std::endl;
  }
  return 0;
}

//<-
void userdata_test_one() {
  lua_State * L = luaL_newstate();

//->
  // ...

  lua_pushcfunction(L, new_token);
  lua_setglobal(L, "new_token");
  lua_pushcfunction(L, inspect_token);
  lua_setglobal(L, "inspect_token");

  const char * script = ""
  "x = new_token() "
  "y = new_token() "
  "inspect_token(x) "
  "inspect_token(y) "
  "y = x "
  "inspect_token(y) "
  "inspect_token(5) ";

  
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
//->
//<-
  lua_close(L);
}

} // end anonymous namespace
//->

//` This example prints
//`  
//= Token id: 0  
//= Token id: 1  
//= Token id: 0  
//= Not a userdata  
//`  
//`  
//` While the tokens can be stored in variables, lua can't inspect or change them directly.
//` `x` cannot be used with lua functions like `+` that take a number, and `x.id` is an error.
//`  
//` An issue with the above code is that if your application has multiple userdata types,
//` then `inspect_token` can lead to undefined behavior -- it uses `static_cast` to convert
//` any userdata type to a `token`.  
//`
//` For this and other reasons, you should always set a metatable associated to any userdata value,
//` and use `luaL_testudata` to test that it's the userdata type you think you had.
//`  
//` Rather than illustrate that, we'll now show a more primer way of doing it.

struct token {
  int id;
};

namespace primer {
namespace traits {

template <>
struct userdata<token> {
  static constexpr const char * name = "token";
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
void userdata_test_two() {
  lua_State * L = luaL_newstate();

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
  "y = x "
  "inspect_token(y)";
  // "inspect_token(5)";

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

//` In this example, the userdata type `token` is passed by reference to the function `inspect_token`.
//` This implicitly carries out the `luaL_testudata`, and signals an argument error if the it isn't passed
//` a `token`. (i.e. `inspect_token(5)`)
//`
//` Generally userdata must be passed by reference and not by value -- the values exist in lua's memory, not
//` on the stack. It's usually incorrect and undesirable to take a copy.
//`
//` By constrast, when passing `std::string` or `std::vector`, usually it makes the most sense to recieve it
//` by value. It doesn't exist in that format in lua's memory, so primer will make a temporary when it calls 
//` your function. You might as well take ownership of that temporary, rather than getting it by `const &`
//` or something.

//]

} // end anonymous namespace

namespace test_three {

//[ primer_tutorial_userdata_example_2

//` [h3 Metatables]
//` Extending our contrived example, let's suppose we want lua scripts to be able to access the token id, but not change it.
//`  
//` We'd like to give `token` a metatable that looks like
//= {
//=   __index = impl_token_index
//= }
//` where `impl_token_index` is some appropriate C function.
//`  
//` In pure lua it looks something like this:

struct token {
  int id;
};

int impl_token_index(lua_State * L) {
  token & tok = *static_cast<token*>(luaL_checkudata(L, 1, "token"));
  if (0 == std::strcmp(lua_tostring(L, 2), "id")) {
    lua_pushnumber(L, tok.id);
    return 1;
  }
  return 0;
}

void init_token_metatable(lua_State * L) {
  luaL_newmetatable(L, "token");

  lua_pushcfunction(L, impl_token_index);
  lua_setfield(L, -2, "__index");

  lua_pop(L, 1);
}

int new_token(lua_State * L) {
  static int count = 0;
  void * udata = lua_newuserdata(L, sizeof(token));
  new (udata) token{count++};
  luaL_setmetatable(L, "token");
  return 1;
}

int inspect_token(lua_State * L) {
  token & tok = *static_cast<token*>(luaL_checkudata(L, 1, "token"));
  std::cout << "Token id: " << tok.id << std::endl;
  return 0;
}


//<-
void userdata_test_three() {
  lua_State * L = luaL_newstate();
  luaL_requiref(L, "", &luaopen_base, 1);
//->
  // ...

  init_token_metatable(L);

  lua_pushcfunction(L, new_token);
  lua_setglobal(L, "new_token");
  lua_pushcfunction(L, inspect_token);
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
//->
//<-
  lua_close(L);
}
} // end namespace test_three

//->

//` This example prints
//`  
//= Token id: 0  
//= Token id: 1  
//= x < y
//= x == y
//`  

//]

int main() {
  push_read();
  adapt_one();
  adapt_two();
  test_one::userdata_test_one();
  userdata_test_two();
  test_three::userdata_test_three();
}
