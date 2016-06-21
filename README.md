# lua primer

[![Build Status](https://travis-ci.org/cbeck88/lua-primer.svg?branch=master)](http://travis-ci.org/cbeck88/lua-primer)
[![Coverage Status](https://coveralls.io/repos/cbeck88/lua-primer/badge.svg?branch=master&service=github)](https://coveralls.io/github/cbeck88/lua-primer?branch=master)
[![Boost licensed](https://img.shields.io/badge/license-Boost-blue.svg)](./LICENSE)

**lua primer** is a small, header-only C++11 template library that helps to create lua bindings to C++ code.
It has no external dependencies besides lua and the standard library.

The basic idea behind **primer** is that it provides two template functions, `primer::push` and `primer::read`,
which allow to push C++ types onto the lua stack, using the lua C API, or, read them from the stack.

These functions come with batteries included, and can handle all "basic" types, but they can also be easily
customized to handle any datatypes that you like.

Once you've done this, **primer** can be used to adapt C++ functions with arbitrary function parameters into
functions which can be passed to lua. **primer** takes care of reading the inputs from lua, and signaling errors
in case of a type mismatch. **primer** also supports optional parameters, and can even support a "named parameter" idiom.

**primer** has very good support for registering custom C++ classes as userdata. However, it also is good at transforming
data which is merely "structured" into lua tables, or trying to coerce lua tables to fit a schema. **primer** is very
efficient and can be made not only to work with your types, but also with your containers.

**primer** is different from `luabind` and other similiar libraries in that **primer** is smaller,
more configurable, and more of the logic that it uses is visible at compile-time, rather than existing only in
runtime data structures. This makes it useful as a building block in more complex tools.

`push` and `read`
-----------------

More info to come... check back later.
