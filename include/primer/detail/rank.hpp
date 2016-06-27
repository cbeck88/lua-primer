//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * The "rank" template is a handy trick which can be used for
 * certain compile-time metaprogramming techniques. It creates
 * an inheritance hierarchy of trivial classes.
 */

namespace primer {

namespace detail {

template <int N>
struct Rank : Rank<N - 1> {};

template <>
struct Rank<0> {};

} // end namespace detail

} // end namespace mpl
