//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

namespace primer {

/***
 * Utility for manipulating lists of integers
 */

template <std::size_t... Ss>
struct SizeList {
  static constexpr std::size_t size = sizeof...(Ss);
};

template <typename L, typename R>
struct Concat;

template <std::size_t... TL, std::size_t... TR>
struct Concat<SizeList<TL...>, SizeList<TR...>> {
  typedef SizeList<TL..., TR...> type;
};

/***
 * Count_t<n> produces a sizelist containing numbers 0 to n-1.
 */
template <std::size_t n>
struct Count {
  typedef typename Concat<typename Count<n - 1>::type, SizeList<n - 1>>::type type;
};

template <>
struct Count<0> {
  typedef SizeList<> type;
};

template <std::size_t n>
using Count_t = typename Count<n>::type;

} // end namespace primer
