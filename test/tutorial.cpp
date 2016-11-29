#include <primer/primer.hpp>
#include <primer/std/vector.hpp>

#include <cassert>
#include <iostream>
#include <string>

using uint = unsigned int;

void push_read() {
//[ primer_tutorial_push_read_example
//` The most basic operations in `primer` are `push` and `read`.

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

//` There are similar extension headers for other kinds of data structures. (See reference).

//` It is also relatively easy to provide overloads for `primer::push` and `primer::read`.

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

//` Note that PRIMER_ADAPT must be used to "convert" the primer signature
//` `primer::result (*)(lua_State *, ...)` to the lua signature `int (*)(lua_State *)`.

//` `PRIMER_ADAPT` generates the boiler-plate which reads the arguments off of the stack,
//` using `primer::read` to do the work. That is: If you implement `primer::read` for your custom
//` data types, then they can be arguments to functions which are adapted this way.

//]

int main() {
  push_read();
  adapt_one();
  adapt_two();
}
