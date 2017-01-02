//  (C) Copyright 2015 - 2017 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * Some macros which are helpful for defining "overloaded" macros which behave
 * differently depending on how many arguments you pass them
 */

#define PRIMER_PP_EXPAND(x) x
#define PRIMER_PP_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, N, ...) N
#define PRIMER_PP_NARG(...)                                                    \
  PRIMER_PP_EXPAND(                                                            \
    PRIMER_PP_ARG_N(__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))

/* need extra level to force extra eval */
#define PRIMER_PP_CONCAT_(a, b) a##b
#define PRIMER_PP_CONCAT(a, b) PRIMER_PP_CONCAT_(a, b)

#define PRIMER_PP_APPLY(M, ...) PRIMER_PP_EXPAND(M(__VA_ARGS__))

/* Invokes a macro with some arguments, after pasting the number of arguments to
 * the macro name.
 *
 * PRIMER_PP_OVERLOAD(F, A, B, C) will expand to `F3(A, B, C)`
 * PRIMER_PP_OVERLOAD(F, A, B, C, D) will expand to `F4(A, B, C, D)`.
 */
#define PRIMER_PP_OVERLOAD(STEM, ...)                                          \
  PRIMER_PP_EXPAND(PRIMER_PP_APPLY(                                            \
    PRIMER_PP_CONCAT(STEM, PRIMER_PP_NARG(__VA_ARGS__)), __VA_ARGS__))
