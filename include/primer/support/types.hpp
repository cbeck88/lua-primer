//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * Various special types recognized by push and read
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <string>

//[ primer_support_types
namespace primer {

// Use this type if you want to push nil onto the stack using primer::push
// interface.
struct nil_t {};

// Use this type if you want primer to convert given type to a boolean, even if
// it was not originally a boolean
struct truthy {
  bool value;
};

// Use this type if you want primer to convert given type to a string, even if
// it was not originally a string
struct stringy {
  std::string value;
};

} // end namespace primer
//]
