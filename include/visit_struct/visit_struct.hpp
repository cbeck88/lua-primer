//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef VISIT_STRUCT_HPP_INCLUDED
#define VISIT_STRUCT_HPP_INCLUDED

/***
 * Provides a facility to declare a structure as "visitable" and apply a visitor
 * to it. The list of members is a compile-time data structure, and there is no
 * run-time overhead.
 */

#include <utility>
#include <type_traits>

// For MSVC 2013 support, we put constexpr behind a define.

# if (defined _MSC_VER) && (_MSC_VER <= 1800)
#   define VISIT_STRUCT_CONSTEXPR
# else
#   define VISIT_STRUCT_CONSTEXPR constexpr
# endif

// After C++14 some more functions, notable apply_visitor, can be constexpr.
// We target C++11, but such functions are tagged VISIT_STRUCT_CXX14_CONSTEXPR.

# if (defined _MSC_VER) || (!defined __cplusplus) || (__cplusplus == 201103L)
#   define VISIT_STRUCT_CXX14_CONSTEXPR
# else
#   define VISIT_STRUCT_CXX14_CONSTEXPR constexpr
# endif

namespace visit_struct {

namespace traits {

// Primary template which is specialized to register a type
template <typename T, typename ENABLE = void>
struct visitable;

// Helper template which checks if a type is registered
template <typename T, typename ENABLE = void>
struct is_visitable : std::false_type {};

template <typename T>
struct is_visitable< T,
                     typename std::enable_if<traits::visitable<T>::value>::type>
 : std::true_type {};

} // end namespace traits

// Interface
template <typename S, typename V>
VISIT_STRUCT_CXX14_CONSTEXPR auto apply_visitor(V && v, S && s) ->
  typename std::enable_if<
             traits::is_visitable<
               typename std::remove_cv<typename std::remove_reference<S>::type>::type
             >::value
           >::type
{
  using clean_S = typename std::remove_cv<typename std::remove_reference<S>::type>::type;
  traits::visitable<clean_S>::apply(std::forward<V>(v), std::forward<S>(s));
}

// Interface
template <typename S, typename V>
VISIT_STRUCT_CXX14_CONSTEXPR auto apply_visitor(V&& v) ->
  typename std::enable_if<
             traits::is_visitable<
               typename std::remove_cv<typename std::remove_reference<S>::type>::type
             >::value
           >::type
{
  using clean_S = typename std::remove_cv<typename std::remove_reference<S>::type>::type;
  traits::visitable<clean_S>::apply(std::forward<V>(v));
}

/***
 * To implement the VISITABLE_STRUCTURE macro, we need a map-macro, which can take
 * the name of a macro and some other arguments, and apply that macro to each other argument.
 *
 * There are some techniques you can use within C preprocessor to accomplish this succinctly,
 * by settng up "recursive" macros.
 *
 * But this can also cause it to give worse error messages when something goes wrong.
 *
 * We are now doing it in a more "dumb" brute force way which has the advantage that it is
 * more portable and gives better error messages.
 * For discussion see IMPLEMENTATION_NOTES.md
 *
 * The code below is based on a patch from Jarod42, and is now generated by a python script.
 * The purpose of the generated code is to define VISIT_STRUCT_PP_MAP as described.
 */

/*** Generated code ***/

static VISIT_STRUCT_CONSTEXPR const int max_visitable_members = 69;

#define VISIT_STRUCT_EXPAND(x) x
#define VISIT_STRUCT_PP_ARG_N( \
        _1, _2, _3, _4, _5, _6, _7, _8, _9, _10,\
        _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,\
        _21, _22, _23, _24, _25, _26, _27, _28, _29, _30,\
        _31, _32, _33, _34, _35, _36, _37, _38, _39, _40,\
        _41, _42, _43, _44, _45, _46, _47, _48, _49, _50,\
        _51, _52, _53, _54, _55, _56, _57, _58, _59, _60,\
        _61, _62, _63, _64, _65, _66, _67, _68, _69, N, ...) N
#define VISIT_STRUCT_PP_NARG(...) VISIT_STRUCT_EXPAND(VISIT_STRUCT_PP_ARG_N(__VA_ARGS__,  \
        69, 68, 67, 66, 65, 64, 63, 62, 61, 60,  \
        59, 58, 57, 56, 55, 54, 53, 52, 51, 50,  \
        49, 48, 47, 46, 45, 44, 43, 42, 41, 40,  \
        39, 38, 37, 36, 35, 34, 33, 32, 31, 30,  \
        29, 28, 27, 26, 25, 24, 23, 22, 21, 20,  \
        19, 18, 17, 16, 15, 14, 13, 12, 11, 10,  \
        9, 8, 7, 6, 5, 4, 3, 2, 1, 0))

/* need extra level to force extra eval */
#define VISIT_STRUCT_CONCAT_(a,b) a ## b
#define VISIT_STRUCT_CONCAT(a,b) VISIT_STRUCT_CONCAT_(a,b)

#define VISIT_STRUCT_APPLYF0(f)
#define VISIT_STRUCT_APPLYF1(f,_1) f(_1)
#define VISIT_STRUCT_APPLYF2(f,_1,_2) f(_1) f(_2)
#define VISIT_STRUCT_APPLYF3(f,_1,_2,_3) f(_1) f(_2) f(_3)
#define VISIT_STRUCT_APPLYF4(f,_1,_2,_3,_4) f(_1) f(_2) f(_3) f(_4)
#define VISIT_STRUCT_APPLYF5(f,_1,_2,_3,_4,_5) f(_1) f(_2) f(_3) f(_4) f(_5)
#define VISIT_STRUCT_APPLYF6(f,_1,_2,_3,_4,_5,_6) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6)
#define VISIT_STRUCT_APPLYF7(f,_1,_2,_3,_4,_5,_6,_7) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7)
#define VISIT_STRUCT_APPLYF8(f,_1,_2,_3,_4,_5,_6,_7,_8) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8)
#define VISIT_STRUCT_APPLYF9(f,_1,_2,_3,_4,_5,_6,_7,_8,_9) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9)
#define VISIT_STRUCT_APPLYF10(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10)
#define VISIT_STRUCT_APPLYF11(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11)
#define VISIT_STRUCT_APPLYF12(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12)
#define VISIT_STRUCT_APPLYF13(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13)
#define VISIT_STRUCT_APPLYF14(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14)
#define VISIT_STRUCT_APPLYF15(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15)
#define VISIT_STRUCT_APPLYF16(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16)
#define VISIT_STRUCT_APPLYF17(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17)
#define VISIT_STRUCT_APPLYF18(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18)
#define VISIT_STRUCT_APPLYF19(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19)
#define VISIT_STRUCT_APPLYF20(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20)
#define VISIT_STRUCT_APPLYF21(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21)
#define VISIT_STRUCT_APPLYF22(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22)
#define VISIT_STRUCT_APPLYF23(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23)
#define VISIT_STRUCT_APPLYF24(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24)
#define VISIT_STRUCT_APPLYF25(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25)
#define VISIT_STRUCT_APPLYF26(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26)
#define VISIT_STRUCT_APPLYF27(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27)
#define VISIT_STRUCT_APPLYF28(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28)
#define VISIT_STRUCT_APPLYF29(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29)
#define VISIT_STRUCT_APPLYF30(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30)
#define VISIT_STRUCT_APPLYF31(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31)
#define VISIT_STRUCT_APPLYF32(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32)
#define VISIT_STRUCT_APPLYF33(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33)
#define VISIT_STRUCT_APPLYF34(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34)
#define VISIT_STRUCT_APPLYF35(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35)
#define VISIT_STRUCT_APPLYF36(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36)
#define VISIT_STRUCT_APPLYF37(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37)
#define VISIT_STRUCT_APPLYF38(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38)
#define VISIT_STRUCT_APPLYF39(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39)
#define VISIT_STRUCT_APPLYF40(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40)
#define VISIT_STRUCT_APPLYF41(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41)
#define VISIT_STRUCT_APPLYF42(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42)
#define VISIT_STRUCT_APPLYF43(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43)
#define VISIT_STRUCT_APPLYF44(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44)
#define VISIT_STRUCT_APPLYF45(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45)
#define VISIT_STRUCT_APPLYF46(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46)
#define VISIT_STRUCT_APPLYF47(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47)
#define VISIT_STRUCT_APPLYF48(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48)
#define VISIT_STRUCT_APPLYF49(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49)
#define VISIT_STRUCT_APPLYF50(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50)
#define VISIT_STRUCT_APPLYF51(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51)
#define VISIT_STRUCT_APPLYF52(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52)
#define VISIT_STRUCT_APPLYF53(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52,_53) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53)
#define VISIT_STRUCT_APPLYF54(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52,_53,_54) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54)
#define VISIT_STRUCT_APPLYF55(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52,_53,_54,_55) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55)
#define VISIT_STRUCT_APPLYF56(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52,_53,_54,_55,_56) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55) f(_56)
#define VISIT_STRUCT_APPLYF57(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52,_53,_54,_55,_56,_57) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55) f(_56) f(_57)
#define VISIT_STRUCT_APPLYF58(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52,_53,_54,_55,_56,_57,_58) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55) f(_56) f(_57) f(_58)
#define VISIT_STRUCT_APPLYF59(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52,_53,_54,_55,_56,_57,_58,_59) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55) f(_56) f(_57) f(_58) f(_59)
#define VISIT_STRUCT_APPLYF60(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52,_53,_54,_55,_56,_57,_58,_59,_60) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55) f(_56) f(_57) f(_58) f(_59) f(_60)
#define VISIT_STRUCT_APPLYF61(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52,_53,_54,_55,_56,_57,_58,_59,_60,_61) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55) f(_56) f(_57) f(_58) f(_59) f(_60) f(_61)
#define VISIT_STRUCT_APPLYF62(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52,_53,_54,_55,_56,_57,_58,_59,_60,_61,_62) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55) f(_56) f(_57) f(_58) f(_59) f(_60) f(_61) f(_62)
#define VISIT_STRUCT_APPLYF63(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52,_53,_54,_55,_56,_57,_58,_59,_60,_61,_62,_63) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55) f(_56) f(_57) f(_58) f(_59) f(_60) f(_61) f(_62) f(_63)
#define VISIT_STRUCT_APPLYF64(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52,_53,_54,_55,_56,_57,_58,_59,_60,_61,_62,_63,_64) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55) f(_56) f(_57) f(_58) f(_59) f(_60) f(_61) f(_62) f(_63) f(_64)
#define VISIT_STRUCT_APPLYF65(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52,_53,_54,_55,_56,_57,_58,_59,_60,_61,_62,_63,_64,_65) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55) f(_56) f(_57) f(_58) f(_59) f(_60) f(_61) f(_62) f(_63) f(_64) f(_65)
#define VISIT_STRUCT_APPLYF66(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52,_53,_54,_55,_56,_57,_58,_59,_60,_61,_62,_63,_64,_65,_66) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55) f(_56) f(_57) f(_58) f(_59) f(_60) f(_61) f(_62) f(_63) f(_64) f(_65) f(_66)
#define VISIT_STRUCT_APPLYF67(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52,_53,_54,_55,_56,_57,_58,_59,_60,_61,_62,_63,_64,_65,_66,_67) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55) f(_56) f(_57) f(_58) f(_59) f(_60) f(_61) f(_62) f(_63) f(_64) f(_65) f(_66) f(_67)
#define VISIT_STRUCT_APPLYF68(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52,_53,_54,_55,_56,_57,_58,_59,_60,_61,_62,_63,_64,_65,_66,_67,_68) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55) f(_56) f(_57) f(_58) f(_59) f(_60) f(_61) f(_62) f(_63) f(_64) f(_65) f(_66) f(_67) f(_68)
#define VISIT_STRUCT_APPLYF69(f,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52,_53,_54,_55,_56,_57,_58,_59,_60,_61,_62,_63,_64,_65,_66,_67,_68,_69) f(_1) f(_2) f(_3) f(_4) f(_5) f(_6) f(_7) f(_8) f(_9) f(_10) f(_11) f(_12) f(_13) f(_14) f(_15) f(_16) f(_17) f(_18) f(_19) f(_20) f(_21) f(_22) f(_23) f(_24) f(_25) f(_26) f(_27) f(_28) f(_29) f(_30) f(_31) f(_32) f(_33) f(_34) f(_35) f(_36) f(_37) f(_38) f(_39) f(_40) f(_41) f(_42) f(_43) f(_44) f(_45) f(_46) f(_47) f(_48) f(_49) f(_50) f(_51) f(_52) f(_53) f(_54) f(_55) f(_56) f(_57) f(_58) f(_59) f(_60) f(_61) f(_62) f(_63) f(_64) f(_65) f(_66) f(_67) f(_68) f(_69)

#define VISIT_STRUCT_APPLY_F_(M, ...) VISIT_STRUCT_EXPAND(M(__VA_ARGS__))
#define VISIT_STRUCT_PP_MAP(f, ...) VISIT_STRUCT_EXPAND(VISIT_STRUCT_APPLY_F_(VISIT_STRUCT_CONCAT(VISIT_STRUCT_APPLYF, VISIT_STRUCT_PP_NARG(__VA_ARGS__)), f, __VA_ARGS__))

/*** End generated code ***/

/***
 * These macros are used with VISIT_STRUCT_PP_MAP
 */

#define VISIT_STRUCT_MEMBER_HELPER(MEMBER_NAME)                                        \
  std::forward<V>(visitor)(#MEMBER_NAME, struct_instance.MEMBER_NAME);

#define VISIT_STRUCT_MEMBER_HELPER_MOVE(MEMBER_NAME)                                   \
  std::forward<V>(visitor)(#MEMBER_NAME, std::move(struct_instance.MEMBER_NAME));

#define VISIT_STRUCT_MEMBER_HELPER_TYPE(MEMBER_NAME)                                   \
  std::forward<V>(visitor)(#MEMBER_NAME, &self_type::MEMBER_NAME);

// This macro specializes the trait, provides "apply" method which does the work.

#define VISITABLE_STRUCT(STRUCT_NAME, ...)                                                         \
namespace visit_struct {                                                                           \
namespace traits {                                                                                 \
                                                                                                   \
template <>                                                                                        \
struct visitable<STRUCT_NAME, void> {                                                              \
  template <typename V>                                                                            \
  VISIT_STRUCT_CXX14_CONSTEXPR static void apply(V && visitor, STRUCT_NAME & struct_instance)      \
  {                                                                                                \
    VISIT_STRUCT_PP_MAP(VISIT_STRUCT_MEMBER_HELPER, __VA_ARGS__)                                   \
  }                                                                                                \
                                                                                                   \
  template <typename V>                                                                            \
  VISIT_STRUCT_CXX14_CONSTEXPR static void apply(V && visitor, const STRUCT_NAME & struct_instance)\
  {                                                                                                \
    VISIT_STRUCT_PP_MAP(VISIT_STRUCT_MEMBER_HELPER, __VA_ARGS__)                                   \
  }                                                                                                \
                                                                                                   \
  template <typename V>                                                                            \
  VISIT_STRUCT_CXX14_CONSTEXPR static void apply(V && visitor, STRUCT_NAME && struct_instance)     \
  {                                                                                                \
    VISIT_STRUCT_PP_MAP(VISIT_STRUCT_MEMBER_HELPER_MOVE, __VA_ARGS__)                              \
  }                                                                                                \
  template <typename V>                                                                            \
  VISIT_STRUCT_CXX14_CONSTEXPR static void apply(V&& visitor)                                      \
  {                                                                                                \
    using self_type = STRUCT_NAME;                                                                 \
    VISIT_STRUCT_PP_MAP(VISIT_STRUCT_MEMBER_HELPER_TYPE, __VA_ARGS__)                              \
  }                                                                                                \
                                                                                                   \
  static VISIT_STRUCT_CONSTEXPR const bool value = true;                                           \
};                                                                                                 \
                                                                                                   \
}                                                                                                  \
}                                                                                                  \
static_assert(true, "")

} // end namespace visit_struct

#endif // VISIT_STRUCT_HPP_INCLUDED
