//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/lua.hpp>

#include <primer/adapt.hpp>
#include <primer/cpp_pcall.hpp>
#include <primer/error_capture.hpp>
#include <primer/registry_helper.hpp>
#include <primer/set_funcs.hpp>
#include <primer/support/asserts.hpp>
#include <primer/support/function.hpp>

#include <array>
#include <string>
#include <tuple>
#include <utility>

//[ primer_vfs_overview
/*`
`primer::api::vfs` helps to create a custom filesystem exposed to lua.

If you have a class which can resolve lua-style paths to loadable chunks in
some fashion, you can use the `primer::api::vfs` object to
get implementations of `loadfile`, `dofile`, and `require`
which are tied to your vfs and not the actual OS.

Your class should essentially provide an implementation of `loadfile`, with
the following signature:
*/
//= // Concept "VFS Provider"
//= expected<void> load(lua_State *, const std::string & path)
/*`
Given a pointer to the stack and the path argument from the lua script,
either call `luaL_loadbuffer` to load a chunk onto the stack and return
"ok" (`expected<void>{}`), or return some sort of error
message regarding the path.

The vfs can then be constructed from a pointer to an instance of your class.
*/
//]

namespace primer {
namespace api {

template <typename T>
class vfs {

  static T * recover_this(lua_State * L) {
    vfs * ptr = registry_helper<vfs>::obtain(L);
    PRIMER_ASSERT(ptr, "Could not recover self pointer!");
    return static_cast<T *>(ptr);
  }

protected:
  // Implementations

  static primer::result intf_loadfile(lua_State * L, std::string path) {
    if (auto ok = recover_this(L)->load(L, path)) {
      return 1;
    } else {
      return std::move(ok.err());
    }
  }

  static primer::result intf_dofile(lua_State * L, std::string path) {
    if (auto ok = recover_this(L)->load(L, path)) {
      int code, idx;
      std::tie(code, idx) = detail::pcall_helper(L, 0, LUA_MULTRET);

      if (code == LUA_OK) {
        return lua_gettop(L) - idx + 1;
      } else {
        return primer::pop_error(L, code);
      }
    } else {
      return std::move(ok.err());
    }
  }

  static primer::result intf_require(lua_State * L, std::string path) {
    lua_settop(L, 1); /* _LOADED table will be at index 2 */
    lua_getfield(L, LUA_REGISTRYINDEX, "_LOADED");
    lua_getfield(L, 2, path.c_str()); /* _LOADED[path] */
    if (lua_toboolean(L, -1)) { return 1; }
    lua_pop(L, 1);

    if (auto ok = recover_this(L)->load(L, path)) {
      PRIMER_ASSERT(lua_isfunction(L, -1), "load did not produce a function");

      int code;
      std::tie(code, std::ignore) = detail::pcall_helper(L, 0, 1);
      if (code != LUA_OK) { return primer::pop_error(L, code); }

      lua_pushvalue(L, -1);             // push an extra copy
      lua_setfield(L, 2, path.c_str()); // set to _LOADED table
      return 1;
    } else {
      return std::move(ok.err());
    }
  }

  // Helper
  static std::array<const luaL_Reg, 3> get_funcs() {
    std::array<const luaL_Reg, 3> funcs = {{
      luaL_Reg{"loadfile", PRIMER_ADAPT(&intf_loadfile)},
      luaL_Reg{"dofile", PRIMER_ADAPT(&intf_dofile)},
      luaL_Reg{"require", PRIMER_ADAPT(&intf_require)},
    }};
    return funcs;
  }

public:
  // API Feature

  void on_init(lua_State * L) {
    registry_helper<vfs>::store(L, this);

    PRIMER_ASSERT(recover_this(L) == static_cast<T *>(this), "bad self store");

    lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);
    primer::set_funcs(L, vfs::get_funcs());
    lua_pop(L, 1);
  }

  void on_persist_table(lua_State * L) {
    primer::set_funcs_prefix_reverse(L, "vfs_funcs_", vfs::get_funcs());
  }

  void on_unpersist_table(lua_State * L) {
    primer::set_funcs_prefix(L, "vfs_funcs_", vfs::get_funcs());
  }
};

} // end namespace api
} // end namespace primer
