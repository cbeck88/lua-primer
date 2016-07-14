TESTS
=====

Build the test suite using `b2`.

Our Jamroot, besides the usual "toolset", "release / debug", and so on, responds
to the following options:

Environment variables:

- `LUA_ROOT`  
  This should be a relative path from this
  directory to the root of the lua installation.  
  Default is `eris-master-lua5.3`,
  but you could select any of the `lua-5.3.x` folders or another eris folder.

- `BOOST_ROOT`  
  Specify this to use a boost installation
  to use for tests that require the boost headers. By default it tries to use
  `/usr/include`.  
  This is optional.  
  If specified, `$(BOOST_ROOT)/boost/version.hpp`
  should be a valid file.

Flags:

- `--with-lua-as-cpp`  
  Attempts to compile lua as C++ rather than as C.

- `--with-lua-32bit`  
  Configures lua to use 32 bit integers and floating point numbers.

- `--no-static-asserts`  
  Defines the `PRIMER_NO_STATIC_ASSERTS` define when building.


