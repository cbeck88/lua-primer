//  (C) Copyright 2015 - 2018 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * Forward-facing header which includes all of the core primer functionality.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/adapt.hpp>
#include <primer/bound_function.hpp>
#include <primer/coroutine.hpp>
#include <primer/error.hpp>
#include <primer/error_capture.hpp>
#include <primer/expected.hpp>
#include <primer/function.hpp>
#include <primer/lua.hpp>
#include <primer/lua_ref.hpp>
#include <primer/lua_ref_as.hpp>
#include <primer/lua_ref_seq.hpp>
#include <primer/metatable.hpp>
#include <primer/push.hpp>
#include <primer/push_singleton.hpp>
#include <primer/read.hpp>
#include <primer/registry_helper.hpp>
#include <primer/result.hpp>
#include <primer/set_funcs.hpp>
#include <primer/userdata.hpp>
#include <primer/userdata_dispatch.hpp>

#include <primer/container/map_base.hpp>
#include <primer/container/optional_base.hpp>
#include <primer/container/seq_base.hpp>
#include <primer/container/set_base.hpp>
