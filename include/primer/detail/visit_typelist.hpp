//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * Helper which applies a visitor to each member of a typelist.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/detail/typelist.hpp>
#include <utility>

namespace primer {
namespace detail {

template <typename TL>
struct typelist_iterator;

template <typename... Ts>
struct typelist_iterator<TypeList<Ts...>> {
  template <typename V, typename... Args>
  static /* constexpr */ void apply_visitor(V && v, Args && ... args) {
    int dummy[] = { (std::forward<V>(v).template visit_type<Ts>(std::forward<Args>(args)...), 0)..., 0 };
    static_cast<void>(dummy);
    static_cast<void>(v);
  }
};

} // end namespace detail
} // end namespace primer
