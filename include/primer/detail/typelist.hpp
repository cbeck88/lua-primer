//  (C) Copyright 2015 - 2017 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

namespace primer {

namespace detail {

/***
 * Utility for manipulating lists of types
 */

template <typename... Ts>
struct TypeList {
  static constexpr std::size_t size = sizeof...(Ts);
};

/***
 * Concat metafunction
 */

template <typename L, typename R>
struct Concat;

template <typename... TL, typename... TR>
struct Concat<TypeList<TL...>, TypeList<TR...>> {
  typedef TypeList<TL..., TR...> type;
};

/***
 * Append metafunction
 */
template <class List, class T>
struct Append;

template <class... Ts, class T>
struct Append<TypeList<Ts...>, T> {
  typedef TypeList<Ts..., T> type;
};

template <class L, class T>
using Append_t = typename Append<L, T>::type;

} // end namespace detail

} // end namespace primer
