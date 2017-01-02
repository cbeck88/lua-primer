//  (C) Copyright 2015 - 2017 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * Convenient base class for making api objects
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/api/callback_registrar.hpp>
#include <primer/api/persistable.hpp>

namespace primer {

namespace api {

template <typename T>
struct base : public callback_registrar<T>, public persistable<T> {};

} // end namespace api

} // end namespace primer
