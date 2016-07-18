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
#include <primer/support/asserts.hpp>
#include <primer/support/function.hpp>
#include <primer/support/registry_helper.hpp>
#include <primer/support/set_funcs.hpp>

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

class vfs {

  // Delegate to user object
  void * object_;
  expected<void> (*load_method_)(void *, lua_State *, const std::string &);

  expected<void> load(lua_State * L, const std::string & path) {
    if (object_) { return load_method_(object_, L, path); }
    return primer::error::module_not_found(path);
  }

  static vfs * recover_this(lua_State * L) {
    return detail::registry_helper<vfs>::obtain_self(L);
  }

public:
  template <typename T>
  explicit vfs(T * t)
    : object_(static_cast<void *>(t))
    , load_method_(
        +[](void * o, lua_State * L, const std::string & str) -> expected<void> {
          return static_cast<T *>(o)->load(L, str);
        })
  {}

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
  static std::array<const luaL_Reg, 3> get_permanent_entries() {
    std::array<const luaL_Reg, 3> funcs = {{
      luaL_Reg{"primer_loadfile", PRIMER_ADAPT(&intf_loadfile)},
      luaL_Reg{"primer_dofile", PRIMER_ADAPT(&intf_dofile)},
      luaL_Reg{"primer_require", PRIMER_ADAPT(&intf_require)},
    }};
    return funcs;
  }

public:
  // API Feature

  void on_init(lua_State * L) {
    detail::registry_helper<vfs>::store_self(L, this);

    for (const auto & r : vfs::get_permanent_entries()) {
      lua_pushcfunction(L, r.func);
      lua_setglobal(L, r.name + 7); // lop off the "primer_" prefix
    }
  }

  void on_persist_table(lua_State * L) {
    primer::set_funcs_reverse(L, vfs::get_permanent_entries());
  }

  void on_unpersist_table(lua_State * L) {
    primer::set_funcs(L, vfs::get_permanent_entries());
  }
};

} // end namespace api
} // end namespace primer
