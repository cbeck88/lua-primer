# lua primer

[![Build Status](https://travis-ci.org/cbeck88/lua-primer.svg?branch=master)](http://travis-ci.org/cbeck88/lua-primer)
[![Coverage Status](https://coveralls.io/repos/cbeck88/lua-primer/badge.svg?branch=master&service=github)](https://coveralls.io/github/cbeck88/lua-primer?branch=master)
[![Boost licensed](https://img.shields.io/badge/license-Boost-blue.svg)](./LICENSE)

**lua primer** is a modern C++ library that creates bindings between [**lua**](http://lua.org/) and to C++ code.
It is header-only, and has no external dependencies besides lua and the standard library.
**primer** was specifically designed to support [**lua-eris**](https://github.com/fnuecke/eris). 

In short, if you use **lua-eris** and expose your C++ api via **primer**, then you get to write your
application in a nice idiomatic C++ style, expose designated features to lua VMs, and you get
for free the ability to **save and restore** essentially all of the user-visible aspects of the VM,
even if user scripts have created a very complicated state referring to many custom
C++ objects and functions, with closures over hidden objects, suspended coroutines, etc.

The save format is largely portable as well -- see eris' documentation.

By contrast, this can be very difficult to do correctly with other lua binding libraries, which
may be cumbersome to use in concert with **eris**.

Documentation
=============

On [github pages](https://cbeck88.github.io/lua-primer/index.html).

Features
========

- Primer is written to the C++11 standard, with substantial documentation and unit tests.

- Primer is tested against **lua 5.3**, but should also work with lua 5.2. We should
  in general be able to support lua versions that eris supports.

- Automatic, type-safe binding of lua arguments to C++ function parameters.
  This includes built-in support for translating fundamental C++ types and lua types.

  Lua:
  ```
    f('execute', 1, 2)
  ```

  C++:
  ```
    primer::result f(lua_State * L, std::string command, int x, int y) {
      ...
    }
  ```

- Primer can also translate fundamental C++ containers like `map`, `set`, and `vector`.

  Lua:
  ```
    local list = reverse({1,2,3,4})
  ```

  C++:
  ```
    primer::result reverse(lua_State * L, std::vector<int> numbers) {
      std::reverse(numbers.begin(), numbers.end());
      primer::push(L, numbers);
      return 1;
    }
  ```

- A "named parameter" idiom for C++ callback functions that you expose to lua.

  Lua:
  ```
    h{ name = 'Charlie', id = 44 }
  ```

  C++:
  ```
    struct h_arguments {
      std::string name;
      int id;
      boost::optional<int> group;
      boost::optional<std::string> task;
    };

    VISITABLE_STRUCTURE(h_arguments, name, id, group, task);

    primer::result h(lua_State * L, h_arguments args) {
      auto it = database.find(args.id);
      ...
    }
  ```

- Makes it easy for C++ to refer to and use lua objects which exist in a VM, such as functions.

  Lua:
  ```
    local function my_func()
      print("Hello world!")
    end

    bind_click(my_func)
  ```

  C++:
  ```
    struct lua_callback_runner : my_gui::event_handler {
      primer::bound_function func_;

      void handle_event() {
        func_.call();
      }
    }

    primer::result bind_click(lua_State * L) {
      lua_pushvalue(L, 1);
      primer::bound_function func{L};

      if (!func) { return primer::error("Expected a function!"); }

      my_gui::bind_click(lua_callback_runner{std::move(func)});
      return 0;
    }
  ```
      
- Provides extensive customization points.

Licensing and Distribution
==========================

**lua primer** is available under the boost software license. To my knowledge, this is in all cases compatible with the MIT license (which is used by lua).

Just in case, it is also available under the MIT license.

Not generous enough? Contact me, I am flexible.
