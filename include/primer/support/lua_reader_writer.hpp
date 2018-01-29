//  (C) Copyright 2015 - 2018 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * Helper functions to assist with serialization.
 * These functions are valid "luaReader" and "luaWriter" type functions.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <string>

namespace primer {
namespace detail {

// Helper structure for the reader:
struct reader_helper {
  const std::string * str;
  bool sent;

  explicit reader_helper(const std::string & s)
    : str(&s)
    , sent(false) {}
};

// Expects 2nd argument to be reader_helper *
inline const char *
trivial_string_reader(lua_State *, void * data, size_t * size) {
  auto & h = *reinterpret_cast<reader_helper *>(data);
  if (h.sent) {
    *size = 0;
    return nullptr;
  }
  h.sent = true;
  *size = h.str->size();
  return h.str->c_str();
}

// Expects 4th argument to be std::string *
inline int
trivial_string_writer(lua_State *, const void * b, size_t size, void * B) {
  std::string & output = *reinterpret_cast<std::string *>(B);
  const char * incoming = reinterpret_cast<const char *>(b);
  for (size_t i = 0; i < size; ++i) {
    output.push_back(incoming[i]);
  }
  return 0;
}

} // end namespace detail
} // end namespace primer
