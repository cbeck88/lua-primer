//  (C) Copyright 2015 - 2018 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

// RAII object for a lua state

#include <primer/base.hpp>
#include <primer/lua.hpp>

struct lua_raii {
  lua_State * const L_;

  lua_raii()
    : L_(luaL_newstate()) {
    // std::cerr << "opened state: L = " << L_ << std::endl;
    luaL_checkversion(L_);
  }

  ~lua_raii() {
    // std::cerr << "closing state: L = " << L_ << std::endl;
    lua_close(L_);
  }

  lua_raii(const lua_raii &) = delete;
  lua_raii(lua_raii &&) = delete;

  operator lua_State *() const { return L_; }
};
