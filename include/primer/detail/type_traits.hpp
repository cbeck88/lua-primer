//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * Some standard type traits that aren't available everywhere in C++11 standard.
 */

#include <type_traits>

namespace primer {

template <typename T>
using remove_cv_t = typename std::remove_cv<T>::type;

template <typename T>
using remove_reference_t = typename std::remove_reference<T>::type;

template <typename T>
using remove_pointer_t = typename std::remove_pointer<T>::type;

template <typename T>
using decay_t = typename std::decay<T>::type;

template <bool b, typename V = void>
using enable_if_t = typename std::enable_if<b, V>::type;

} // end namespace primer
