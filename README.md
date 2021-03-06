# lua primer

[![Build Status](https://travis-ci.org/cbeck88/lua-primer.svg?branch=master)](http://travis-ci.org/cbeck88/lua-primer)
[![Appveyor status](https://ci.appveyor.com/api/projects/status/github/cbeck88/lua-primer?branch=master&svg=true)](https://ci.appveyor.com/project/cbeck88/lua-primer)
[![Boost licensed](https://img.shields.io/badge/license-Boost-blue.svg)](./LICENSE)

**lua primer** is a modern C++ library that creates bindings between [**lua**](http://lua.org/) and C++.
It is header-only, and has no external dependencies besides lua and the standard library.
**primer** was specifically designed to support [**lua-eris**](https://github.com/fnuecke/eris).
Indeed, you can think of it basically as a C++ wrapper over eris.

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

- Primer is tested against **lua 5.3**, but should also work with lua 5.2.  
  We should in general be able to support lua versions that eris supports.

- The library consists only of headers.  
  No makefiles, no project files, nothing to link with besides lua.

- Zero cost abstraction.  
  Primer is just a thin collection of templates over the lua C api, and a few additional helper classes.  
  Primer doesn't make use of C++ virtual functions, exceptions, or RTTI, and we test that it works when compiled with `-fno-exceptions` and `-fno-rtti`.  
  This ensures that Primer can run as fast as possible in as many environments as possible, and can be used within the constraints of almost any C++11 project.

- Provides extensive, traits-based customization points.

- A safe abstraction of lua functions and coroutines.  
  This allows them to be used like typical C++ function objects elsewhere in your application, without you
  having to think about the lua stack or lua errors when using them.

- Support objects which help with sandboxing.  
  Primer includes a number of support objects and methods to make it easy to run lua within a sandbox.
  You can select a subset of the lua base libraries to expose, sans any functions that access external resources.
  There are other support objects which can replace those functions with customized versions.
  For instance, `primer::api::vfs` can be used to create new versions of `require`, `loadfile`, and `dofile` which
  make requests through a custom vfs object of your choosing.
  Another support object makes it easy to interact with your lua VM via an interpreter dialog,
  to help debug scripts.

- An extensible framework for automatic, type-safe binding of lua arguments to C++ function parameters.  
  This includes built-in support for translating fundamental C++ types and lua types.

  Lua:
  ```lua
    f('execute', 1, 2)
  ```

  C++:
  ```c++
    primer::result f(lua_State * L, std::string command, int x, int y) {
      ...
    }
  ```

- Primer can also translate fundamental C++ containers like `map`, `set`, and `vector`.  

  Lua:
  ```lua
    local list = reverse({1,2,3,4})
  ```

  C++:
  ```c++
    primer::result reverse(lua_State * L, std::vector<int> numbers) {
      std::reverse(numbers.begin(), numbers.end());
      primer::push(L, numbers);
      return 1;
    }
  ```

- A "named parameter" idiom for C++ callback functions that you expose to lua.

  Lua:
  ```lua
    h{ name = 'Charlie', id = 44 }
  ```

  C++:
  ```c++
    struct h_arguments {
      std::string name;
      int id;
      boost::optional<int> group;
      boost::optional<std::string> task;
    };

    VISITABLE_STRUCT(h_arguments, name, id, group, task);

    primer::result h(lua_State * L, h_arguments args) {
      auto it = database.find(args.id);
      ...
    }
  ```

- Makes it easy for C++ to refer to and use lua objects which exist in a VM, such as functions.

  Lua:
  ```lua
    local function my_func()
      print("Hello world!")
    end

    bind_click(my_func)
  ```

  C++:
  ```c++
    struct lua_callback_runner : my_gui::event_handler {
      primer::bound_function func_;

      void handle_event() {
        func_.call();
      }
    };

    primer::result bind_click(lua_State * L, primer::bound_function func) {
      my_gui::bind_click(lua_callback_runner{std::move(func)});
      return 0;
    }
  ```
      

Licensing and Distribution
==========================

**lua primer** is available under the boost software license. To my knowledge, this is in all cases compatible with the MIT/expat license (which is used by lua).

Not generous enough? Contact me, I am flexible.

Tests
=====

The tests and documentation are built using [Boost.Build](http://www.boost.org/build/).

(You can install `boost.build` by downloading the Boost sources from sourceforge and running
`bootstrap` which will build it from source on your machine. Or, download one of their
packaged releases. On Debian-based linux, you can install `libboost-tools-dev`, which will install
the executable to `/usr/bin/bjam`.)

To run the tests, go to the `/test` folder, build with `b2`, and
go to `/test/stage` to run primer's test executables there.

You can also run lua's and eris' internal unit tests, relevant executables for that
go to `/test/stage_lua`.

Compiler Support
================

`primer` is currently tested against `gcc 4.9, gcc 5.3, clang 3.5, clang 3.7`,
and against MSVC 2015 and 2017.

(It should work with all later versions of `gcc` and `clang`, for instance I also
use this code at time of writing in another project with clang 3.8 and clang 4.0, but I don't specifically
run the primer unit tests there.)
