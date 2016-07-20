//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * Return Expected is useful for doing monadic computations using expected.
 *
 * Sometimes, when a taking a generic function F : A -> B, and invoking it in
 * a context where failure may occur, you end up making a function that
 * logically looks like,
 *
    template <typename F, typename... Args>
    auto G (F && f, Args && ... args) -> expected<result_of<F(Args...)>> {
      ...
    }
 *
 * This works fine but in cases where `F` already returns a type `expected<T>`,
 * it does something stupid, making `expected<expected<T>>`.
 *
 * In such cases it is better to return `expected<T>`. Since, `expected<T>` is
 * always constructible from the same types that `expected<expected<T>>` is, so
 * this problem can be solved by simply making `G` return
 *
   return_expected_t<result_of<F(Args...)>>
 *
 * This template is so-named because it plays the same role as the `return`
 * function in a haskell monad, collapsing the doubly-monadic types.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/expected.hpp>

namespace primer {

template <typename T>
struct return_expected {
  typedef expected<T> type;
};

template <typename T>
struct return_expected<expected<T>> {
  typedef expected<T> type;
};

template <typename T>
using return_expected_t = typename return_expected<T>::type;

} // end namespace primer
