//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * How to push `std::set` to the stack, as a table, using the lua "set" idiom.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/traits/optional.hpp>
#include <boost/optional/optional.hpp>

PRIMER_DECLARE_OPTIONAL_TEMPLATE_TYPE(boost::optional);
