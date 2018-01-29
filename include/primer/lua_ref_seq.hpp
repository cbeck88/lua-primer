//  (C) Copyright 2015 - 2018 Christopher Beck

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

//[ lua_ref_seq_synopsis
struct lua_ref_seq {
  using refs_t = std::vector<lua_ref>;
  refs_t refs_;

  /*<< Push all the refs onto the stack in succession.
Return of `true` means every push succeeded.
You can usually ignore the result, it only fails if some of the refs in the
`lua_ref_seq` are actually in an empty state.
If not all of the refs are in the same VM as the argument `lua_State *`,
then you get UB.
See `lua_ref`. >>*/
  bool push_each(lua_State * L) const noexcept {
    bool result = true;
    for (const auto & r : refs_) {
      bool temp = r.push(L);
      result = result && temp;
    }
    return result;
  }

  //
  // Forward MANY methods from std::vector...
  //

  std::size_t size() const { return refs_.size(); }

  using value_type = refs_t::value_type;
  using reference = refs_t::reference;
  using const_reference = refs_t::const_reference;

  // Index
  reference operator[](std::size_t i) { return refs_[i]; }
  const_reference operator[](std::size_t i) const { return refs_[i]; }

  reference at(std::size_t i) { return refs_.at(i); }
  const_reference at(std::size_t i) const { return refs_.at(i); }

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

  void reserve(std::size_t s) { refs_.reserve(s); }
  void clear() { refs_.clear(); }
  void resize(std::size_t s) { refs_.resize(s); }
  void pop_back() { refs_.pop_back(); }
  template <typename... Args>
  void emplace_back(Args &&... args) {
    refs_.emplace_back(std::forward<Args>(args)...);
  }

  template <typename... Args>
  void emplace(const_iterator pos, Args &&... args) {
    refs_.emplace(pos, std::forward<Args>(args)...);
  }

  //<-
  /* Currently makes travis builds fail...
  iterator insert( const_iterator pos, const lua_ref & value ) {
    return refs_.insert(pos, value);
  }

  iterator insert( const_iterator pos, lua_ref && value ) {
    return refs_.insert(pos, std::move(value));
  }
  */
  //->
  template <typename InputIt>
  iterator insert(const_iterator pos, InputIt first, InputIt last) {
    return refs_.insert(pos, first, last);
  }
  //<-
  // Also makes travis fail
  // iterator erase(const_iterator pos) { return refs_.erase(pos); }
  // iterator erase(const_iterator first, const_iterator last) { return
  // refs_.erase(first, last); }
  //->
};
//]

/*<< Pop `n` elements from the stack `L` and put them in a lua_ref_seq.
     They are ordered such that calling `push_each(L)` will restore them.
     Throws `std::bad_alloc`. Can cause lua memory alloc failure.
     First step is clearing the `lua_ref_seq`, so, no strong exception-safety.
  >>*/
inline void pop_n(lua_State * L, int n, lua_ref_seq & result);

//[ lua_ref_seq_pop_decl
/*<< Pop `n` elements form the stack.
     Throws `std::bad_alloc`. Can cause lua memory alloc failure. >>*/
inline lua_ref_seq pop_n(lua_State * L, int n);

/*<< Pop the entire stack. Throws `std::bad_alloc`. Can cause lua memory alloc
failure >>*/
inline lua_ref_seq pop_stack(lua_State * L);
//]

inline void
pop_n(lua_State * L, int n, lua_ref_seq & result) {
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

inline lua_ref_seq
pop_n(lua_State * L, int n) {
  lua_ref_seq result;
  pop_n(L, n, result);
  return result;
}

inline lua_ref_seq
pop_stack(lua_State * L) {
  return pop_n(L, lua_gettop(L));
}

} // end namespace primer
