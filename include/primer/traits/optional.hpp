//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

namespace primer {
namespace traits {

/***
 * Trait used to register a type as having "optional" semantics.
 *
 * Optional semantics means it is a container that contains one or zero of some
 * other type.
 * Like, boost::optional, std::experimental::optional, or mpl::maybe.
 *
 * Primer treats optional types as follows:
 *   If the optional is empty, then pushing it results it in nil.
 *   Otherwise, it's the same as pushing the base type.
 *
 *   When reading, if the slot is nil, then the optional that results is empty.
 *   Otherwise, try to read the base type and emplace it in the optional.
 *   If an error results, return the error.
 *
 * Primer also supports "relaxed optionals". This means:
 *   When reading, try to read the base type and empalce it in the optional.
 *   If an error results, swallow the error and return an empty optional.
 *
 * To declare an optional type, create a partial specialization of this trait,
 * with the following members:
 *
 * template <typename T>
 * struct optional {
 *   typedef base_type;
 *
 *   static const base_type * as_ptr(const T &);
 *
 *   static T make_empty();
 *   static T from_base(base_type &&);
 *
 * };
 *
 * Additionally, you *may* provide `static constexpr bool relaxed = true;` to
 * give it relaxed semantics. If `relaxed` is false, or not present, then it has
 * strict semantics.
 *
 * See <primer/traits/push.hpp> and <primer/traits/read.hpp> for details.
 */

template <typename T>
struct optional;

} // end namespace traits
} // end namespace primer

/***
 * If your optional type has at least a vaguely similar interface to
 * `boost::optional`, then you can
 * register it using the following macro:
 */

#define PRIMER_DECLARE_OPTIONAL_TEMPLATE_TYPE(NAME)                            \
  PRIMER_ASSERT_FILESCOPE;                                                     \
  namespace primer {                                                           \
  namespace traits {                                                           \
                                                                               \
  template <typename T>                                                        \
  struct optional<NAME<T>> {                                                   \
    typedef T base_type;                                                       \
    static const base_type * as_ptr(const NAME<T> & o) {                       \
      if (o) {                                                                 \
        return &*o;                                                            \
      } else {                                                                 \
        return nullptr;                                                        \
      }                                                                        \
    }                                                                          \
                                                                               \
    static NAME<T> make_empty() { return {}; }                                 \
    static NAME<T> from_base(T && t) { return {std::move(t)}; }                \
  };                                                                           \
  }                                                                            \
  }                                                                            \
  static_assert(true, "")
