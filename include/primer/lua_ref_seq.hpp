//  (C) Copyright 2015 - 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * A lua_ref_seq is a sequence lua_refs. It's purpose is to make it easy to
 * say "take all the elements on this stack and put them on that stack". Or, to
 * transfer the results of a lua function call that returns multiple elements to
 * C++ comfortably.
 */

#include <primer/base.hpp>

PRIMER_ASSERT_FILESCOPE;

#include <primer/lua.hpp>
#include <primer/lua_ref.hpp>

#include <type_traits>
#include <utility>
#include <vector>

namespace primer {

//` `lua_ref_seq` represents a sequence of elements. The purpose is to support
//` lua functions that have multiple / variable numbers of return values from
//` C++. The idea is that you call
//= primer::pop_stack(L)
// or
//= primer::pop_n(L, num)
//` and you pop a large number of elements off the stack, or the entire stack,
//` and get them in a nice vector of lua_ref's.
//
/*` An important thing to note is that `lua_ref_seq` cannot participate in
   `push` or `read` because it has the wrong semantics. `primer::push` always
   pushes exactly one object on the stack, and `read` only reads a single
   position. `lua_ref_seq` needs to be pushed as multiple objects on the stack,
   so it is pushed using its `push_each` method.
*/
//` For this reason it cannot be simply a typedef of `std::vector`.
struct lua_ref_seq {
  using refs_t = std::vector<lua_ref>;
  refs_t refs_;

  // Forward some choice methods from std::vector
  std::size_t size() const { return refs_.size(); }

  using value_type = refs_t::value_type;
  using reference = refs_t::reference;
  using const_reference = refs_t::const_reference;

  // Index
  reference operator[](std::size_t i) { return refs_[i]; }
  const_reference operator[](std::size_t i) const { return refs_[i]; }

  reference front() { return refs_.front(); }
  const_reference front() const { return refs_.front(); }

  reference back() { return refs_.back(); }
  const_reference back() const { return refs_.back(); }

  // Support iteration
  using iterator = refs_t::iterator;
  using const_iterator = refs_t::const_iterator;

  iterator begin() { return refs_.begin(); }
  const_iterator begin() const { return refs_.begin(); }

  iterator end() { return refs_.end(); }
  const_iterator end() const { return refs_.end(); }

  // Manipulation

  void clear() { refs_.clear(); }
  void resize(std::size_t s) { refs_.resize(s); }
  void pop_back() { refs_.pop_back(); }
  template <typename... Args>
  void emplace_back(Args &&... args) {
    refs_.emplace_back(std::forward<Args>(args)...);
  }

  /*<< Push all the refs onto the stack in succession.
Return of `true` means every push succeeded.
You can usually ignore the result, it only fails if some of the refs in the
ref_seq are actually in an empty state.
If not all of the refs are in the same VM as the argument, then you get UB.
See `lua_ref`. >>*/
  bool push_each(lua_State * L) const noexcept {
    bool result = true;
    for (const auto & r : refs_) {
      bool temp = r.push(L);
      result = result && temp;
    }
    return result;
  }
};

/*<< Pop `n` elements from the stack `L` and put them in a lua_ref_seq.
     They are ordered such that calling `push_each(L)` will restore them.

     Throws `std::bad_alloc`. Can cause lua memory alloc failure.
     First step is clearing the lua_ref_seq, so, no strong exception-safety.
  >>*/
void pop_n(lua_State * L, int n, lua_ref_seq & result) {
  result.clear();

  {
    int top = lua_gettop(L);
    if (n > top) { n = top; }
    if (n < 0) { n = 0; }
  }

  result.resize(n); // Default constructs (empty) lua_refs
  // Put the items in the vector in the same order that they sat on stack
  for (--n; n >= 0; --n) {
    result[n] = lua_ref{L};
  }
}

/*<< Same thing but with different return convention >>*/
lua_ref_seq pop_n(lua_State * L, int n) {
  lua_ref_seq result;
  pop_n(L, n, result);
  return result;
}


/*<< Pop the entire stack. Throws `std::bad_alloc`. Can cause lua memory alloc
 * failure >>*/
lua_ref_seq pop_stack(lua_State * L) { return pop_n(L, lua_gettop(L)); }

} // end namespace primer
