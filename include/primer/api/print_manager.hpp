//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/cpp_pcall.hpp>
#include <primer/error_capture.hpp>
#include <primer/lua.hpp>
#include <primer/registry_helper.hpp>
#include <primer/set_funcs.hpp>
#include <primer/support/asserts.hpp>
#include <primer/support/scoped_stash_global_value.hpp>

#include <array>
#include <iostream>
#include <string>
#include <vector>

//[ primer_print_manager_overview
//` The `print_manager` class is an API feature which creates "print" and
//` "pretty_print" functions which can be used by scripts.
//`
//` This feature also makes it so that the output can be easily redirected, to
//` e.g. a diagnostic shell or gui dialog
//` to help create an interactive interpreter for the debugging user scripts.
//`
//`
//` Print redirecting is achieved using the method "set_interpreter_context"
//` which takes a pointer to an arbitrary object satisfying the
//` "interpreter_context" concept. This pointer is placed on a stack, the
//` interpreter context can be detached using "pop_interpreter_context".
//`
//` The method "handle_user_input" takes a string and considers it as user
//` input from an interactive shell or dialog.
//`
//` This is handled using an
//` "experimental compilation" approach. First, we assume that the command is
//` a lua value, and we transform it to `_pretty_print(input)`, and check if
//` that is syntactically well-formed. If so, we execute it.
//`
//` If it is not well-formed,
//` then we try simply loading `input` -- maybe it is just a statement. If so,
//` we execute it, if not we report a syntax error to the user.
//]

namespace primer {

//[ primer_interpreter_context_concept
//` The `interpreter_context` concept consists of three methods:
//`
//= void new_text(const std::string &);
//= void error_text(const std::string &);
//= void clear_input();
//`
//` * `new_text` is called to report a line of print output.
//` * `error_text` is called to report an error message.
//` * `clear_input` is called when the user's input has been accepted. When
//`   creating a gui dialog, you can use this function as a cue to clear the
//`   editbox that the user was using to enter the input. Then if they made
//`   a mistake, they don't have to retype their whole command.
//]

namespace detail {

// Implementation here is based on "impossibly fast delegates" article
class interpreter_context_ptr {
  void * object_;
  void (*new_text_call_)(void *, const std::string &);
  void (*error_text_call_)(void *, const std::string &);
  void (*clear_input_call_)(void *);

public:
  void new_text(const std::string & str) const {
    this->new_text_call_(object_, str);
  }

  void error_text(const std::string & str) const {
    this->error_text_call_(object_, str);
  }

  void clear_input() const { this->clear_input_call_(object_); }

  template <typename T>
  explicit interpreter_context_ptr(T * t)
    : object_(static_cast<void *>(t))
    , new_text_call_(+[](void * o, const std::string & str) {
      static_cast<T *>(o)->new_text(str);
    })
    , error_text_call_(+[](void * o, const std::string & str) {
      static_cast<T *>(o)->error_text(str);
    })
    , clear_input_call_(+[](void * o) { static_cast<T *>(o)->clear_input(); }) {
  }
};

// Default print and pretty print implementations
inline std::string
default_print_format(lua_State * L) {
  std::string buffer;
  int nargs = lua_gettop(L);
  for (int i = 1; i <= nargs; ++i) {
    const char * str = lua_tostring(L, i);
    if (!str) { str = ""; }
    if (i > 1) {
      buffer += "\t"; // separate multiple args with tab character
    }
    buffer += str;
  }
  return buffer;
}

inline std::string
default_pretty_print_format(lua_State * L) {
  if (!lua_gettop(L)) { return ""; }
  lua_settop(L, 1);

  std::string result = luaL_typename(L, 1);

  // Try luaL_tolstring, if that works it gives a nicer result than
  // luaL_typename
  primer::cpp_pcall<1>(L, [&]() {
    result = luaL_tolstring(L, 1, nullptr);
    lua_pop(L, 1);
  });

  return result;
}

// Strip a '[', ']' pair, and at most two subsequent ':' symbols from the middle
// of string
inline std::string
strip_line_info(const std::string & e) {
  typedef std::string::const_iterator str_it;

  for (str_it it = e.begin(); it != e.end(); ++it) {
    if (*it == '[') {
      for (str_it it2 = it; it2 != e.end(); ++it2) {
        if (*it2 == ']') {
          str_it it3 = it2;
          while (it3 != e.end() && *it3 != ':') {
            ++it3;
          }
          if (it3 != e.end()) { it2 = ++it3; }
          while (it3 != e.end() && *it3 != ':') {
            ++it3;
          }
          if (it3 != e.end()) { it2 = ++it3; }

          return std::string(e.begin(), it) + std::string(it2, e.end());
        }
      }
    }
  }
  return e;
}

} // end namespace detail

//[ primer_print_manager_synopsis
namespace api {

class print_manager {
  std::vector<detail::interpreter_context_ptr> stack_;

  using format_func_t = std::string (*)(lua_State *);

  // Pointers for custom print / pretty print formatting functions
  format_func_t print_format_ = nullptr;
  format_func_t pretty_print_format_ = nullptr;

  //<-
  static constexpr const char * pretty_print_name = "_pretty_print";

  // Push a unique object to be our registry key for the "this" void pointer

  static int intf_print_impl(lua_State * L) {
    print_manager * man = registry_helper<print_manager>::obtain_self(L);
    if (man->print_format_) {
      man->new_text(man->print_format_(L));
    } else {
      man->new_text(detail::default_print_format(L));
    }
    return 0;
  }

  static int intf_pretty_print_impl(lua_State * L) {
    print_manager * man = registry_helper<print_manager>::obtain_self(L);
    if (man->pretty_print_format_) {
      man->new_text(man->pretty_print_format_(L));
    } else {
      man->new_text(detail::default_pretty_print_format(L));
    }
    return 0;
  }

  void handle_interpreter_error(lua_State * L, int code) {
    primer::error e{primer::pop_error(L, code)};
    this->error_text(detail::strip_line_info(e.what()));
  }

  // Helper
  static std::array<const luaL_Reg, 2> get_funcs() {
    std::array<const luaL_Reg, 2> funcs = {{
      luaL_Reg{"print", &intf_print_impl},
      luaL_Reg{pretty_print_name, &intf_pretty_print_impl},
    }};
    return funcs;
  }

  //->
public:
  // Add or remove interpreter context pointers from the stack
  template <typename T>
  void set_interpreter_context(T * t) {
    stack_.emplace_back(t);
  }

  void pop_interpreter_context() {
    if (stack_.size()) { stack_.pop_back(); }
  }

  // Set a custom print or pretty-print formatting
  // Function should take a `lua_State *` and return `std::string`.
  // Do whatever you like with the stack. Don't raise errors.
  void set_custom_print_format_func(format_func_t f) { print_format_ = f; }

  void set_custom_pretty_print_format_func(format_func_t f) {
    pretty_print_format_ = f;
  }

  // Interact directly with context on top of stack, or,
  // with stdout / stderr if stack is empty
  void new_text(const std::string & str) const {
    if (stack_.size()) {
      stack_.back().new_text(str);
    } else {
      std::cout << str << std::endl;
    }
  }

  void error_text(const std::string & str) const {
    if (stack_.size()) {
      stack_.back().error_text(str);
    } else {
      std::cerr << str << std::endl;
    }
  }

  void clear_input() const {
    if (stack_.size()) { stack_.back().clear_input(); }
  }

  // Handle interpreter input
  // Note: Clears the entire stack when it runs.
  inline void handle_interpreter_input(lua_State * L,
                                       const std::string & user_input);

  // Convenience function over the above.
  // Recover the print_manager from a lua_State * pointer, then setup an
  // arbitrary interpreter context, run a command, and pop the context.
  //
  // Note: There must be a print manager attached to the lua_State * or an
  // assertion will fail.
  template <typename T>
  static void interpreter_input(lua_State * L, T & t, const std::string & s) { 
    print_manager * man = registry_helper<print_manager>::obtain_self(L);
    man->set_interpreter_context(&t);
    man->handle_interpreter_input(L, s);
    man->pop_interpreter_context();
  }

  //
  // API Feature
  //

  void on_init(lua_State * L) {
    PRIMER_ASSERT_STACK_NEUTRAL(L);

    registry_helper<print_manager>::store_self(L, this);

    lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);
    primer::set_funcs(L, get_funcs());
    lua_pop(L, 1);
  }

  void on_persist_table(lua_State * L) {
    primer::set_funcs_prefix_reverse(L, "print_manager__", get_funcs());
  }

  void on_unpersist_table(lua_State * L) {
    primer::set_funcs_prefix(L, "print_manager__", get_funcs());
  }
};

} // end namespace api
//]

inline void
api::print_manager::handle_interpreter_input(lua_State * L,
                                             const std::string & text) {
  lua_settop(L, 0);

  if (!lua_checkstack(L, 2)) {
    this->error_text("Insufficient stack space, needed 2");
    return;
  }

  // If the user messed with _pretty_print function, temporarily ours back.
  primer::detail::scoped_stash_global_value(L, pretty_print_name);
  lua_pushcfunction(L, &intf_pretty_print_impl);
  lua_setglobal(L, pretty_print_name);

  std::string experiment = pretty_print_name + ("(" + text + ")");

  if (LUA_OK != luaL_loadstring(L, experiment.c_str())) {
    // Got an error (presumably a syntax error), try just the original text
    lua_pop(L, 1);
    int err_code = luaL_loadstring(L, text.c_str());
    if (LUA_OK != err_code) {
      this->handle_interpreter_error(L, err_code);
      lua_settop(L, 0);
      return;
    }
    this->new_text("$ " + text);
    this->clear_input();

    err_code = lua_pcall(L, 0, 0, 0);
    if (LUA_OK != err_code) { this->handle_interpreter_error(L, err_code); }
  } else {
    this->new_text("$ " + text);
    this->clear_input();

    int err_code = lua_pcall(L, 0, 0, 0);
    if (LUA_OK != err_code) { this->handle_interpreter_error(L, err_code); }
  }
  lua_settop(L, 0);
}
} // end namespace primer
