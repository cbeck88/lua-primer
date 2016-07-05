//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * A weak version of std::experimental::make_array
 */

#include <array>
#include <utility>

namespace primer {
namespace detail {

template <typename T, typename... Args>
constexpr auto make_array(Args && ... args) -> std::array<T, sizeof...(Args)> {
  return {{std::forward<Args>(args)...}};
}

} // end namespace detail
} // end namespace primer
