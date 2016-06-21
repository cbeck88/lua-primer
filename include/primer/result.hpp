//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * Primer result is a structure that signals how to terminate a lua
 * function call.
 *
 * There are three possible signals:
 *   return
 *   yield
 *   error
 *
 * Return and yield both take an int, indicating how many arguments went on the
 * stack.
 * error is indicated by a primer::error object.
 *
 * Code to send corresponding signal to lua is in
 * `primer/support/implement_result.hpp`
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/expected.hpp>

namespace primer {

// Tag used by the user to signal yield.
struct yield {
  int n_;
};

// Helper object: Represents a return or yield signal.
struct return_or_yield {
  int n_;
  bool is_return_;

  bool is_valid() const { return n_ >= 0; }
};


// Actual object
class result {

  expected<return_or_yield> payload_;

public:
  // Ctors (implicit for ease of use)
  result(int i)
    : payload_(return_or_yield{i, true})
  {}
  result(yield y)
    : payload_(return_or_yield{y.n_, false})
  {}
  result(error e)
    : payload_(e)
  {}

  // Accessor
  const expected<return_or_yield> & get_payload() const & { return payload_; }
  expected<return_or_yield> & get_payload() & { return payload_; }
};

} // end namespace primer
