//  (C) Copyright 2015 - 2018 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef NONSTD_WEAK_REF_HPP_INCLUDED
#define NONSTD_WEAK_REF_HPP_INCLUDED

/***
 * Synopsis:
 * master_ref and weak_ref are a pair of "smart pointer-like" objects.
 *
 * - Both are *value types* about the size of a pointer. They both can be
     dereferenced, to access the object they are pointing to.
 * - master_ref is constructed from a raw pointer to an object (or, default
     constructed.)
 * - weak_ref can be constructed from master_ref.
 * - They do not manage the lifetime of the thing they are referring to,
     instead, the master_ref merely manages the validity of the weak_ref's,
     making it easier to manage the possibility of dangling pointers. (But not
     100% eliminating it.)
 * - On the plus side, it has significantly less overhead, and can be used
     with any object, even a stack-allocated one.
 * - "weak_ref::lock" returns a raw pointer rather than a smart pointer, since
     it is not locking in the sense of taking ownership. It's merely returning
     a validity-checked pointer -- if it's not nullptr, then it is safe to
     dereference, unless the "master_ref" itself is dangling (easier to manage.)
 * - They are not thread safe, they should not be passed across threads. If you
     need that then you should use `std::shared_ptr` instead.
 * - The interface mimics `std::weak_ptr`.
 * - There is no possibility of leaks due to "cyclic references". The only
     object whose lifetime is managed by reference counting here, is a shared
     control structure, whose destructor is trivial.
 */

#include <utility>

namespace nonstd {

namespace detail {

template <typename T>
struct weak_ref_control_structure {
  T * payload_;
  mutable long ref_count_;

  explicit weak_ref_control_structure(T * t) : payload_(t), ref_count_(0) {}
};

} // end namespace detail

// Forward declare weak_ref
template <typename T>
class weak_ref;

// master_ref: Owner of control structure
template <typename T>
class master_ref {
  using ctrl_t = detail::weak_ref_control_structure<T>;

  ctrl_t * ptr_;

  void init(T * t) {
    if (t) {
      ptr_ = new ctrl_t(t);
    } else {
      ptr_ = nullptr;
    }
  }

  void move(master_ref & o) noexcept {
    ptr_ = o.ptr_;
    o.ptr_ = nullptr;
  }

  // Invariant: If ptr_ is not null, it points to a ctrl_t that no other
  // master_ref points to, and ptr_->payload_ is also not null.

  friend class weak_ref<T>;

public:
  typedef T element_type;

  // Initialization
  explicit master_ref(T * t) { this->init(t); }

  // Special member functions
  constexpr master_ref() noexcept : ptr_(nullptr) {}

  ~master_ref() noexcept { this->reset(); }

  master_ref(master_ref && other) noexcept { this->move(other); }

  master_ref & operator = (master_ref && other) noexcept {
    this->reset();
    this->move(other);
    return *this;
  }

  // Copy ctor: Make a new ctrl structure pointing to the same payload
  master_ref(const master_ref & other) {
    this->init(other.ptr_ ? other.ptr_->payload_ : nullptr);
  }

  // Copy assignment: Copy and swap
  master_ref & operator = (const master_ref & other) {
    master_ref temp{other};
    this->swap(temp);
    return *this;
  }

  // Reset (release managed control object)
  // Set the payload pointer to null, to signal to weak refs that they are
  // now closed. If ref_count_ is zero, then we must delete.
  void reset() noexcept {
    if (ptr_) {
      ptr_->payload_ = nullptr;
      if (!ptr_->ref_count_) {
        delete ptr_;
      }
      ptr_ = nullptr;
    }
  }

  // Swap
  void swap(master_ref & other) noexcept {
    ctrl_t * temp = ptr_;
    ptr_ = other.ptr_;
    other.ptr_ = temp;
  }

  // Observers
  // Get the managed pointer
  T * get() const noexcept {
    if (ptr_) { return ptr_->payload_; }
    return nullptr;
  }

  // Operator *: Blindly dereference the getted pointer, without a null check.
  T & operator *() const noexcept {
    return *this->get();
  }

  // Operator bool: check if there is a managed object
  explicit constexpr operator bool() const noexcept { return ptr_; }

  // use_count: mimic std::shared_ptr interface
  constexpr long use_count() const noexcept {
    return ptr_ ? 1 : 0;
  }

  // unique: mimic std::shared_ptr interface
  constexpr bool unique() const noexcept {
    return this->use_count() == 1;
  }

  // weak_ref_count: do something more useful :)
  long weak_ref_count() const noexcept {
    if (ptr_) { return ptr_->ref_count_; }
    return 0;
  }
};

template <typename T>
class weak_ref {
  using ctrl_t = detail::weak_ref_control_structure<T>;

  // Rationale: When we lock the weak_ref, if the ref has expired, we want to
  // release this immediately, and set to nullptr, so that future lookups are
  // faster. Since the caller is going to test the pointer we return anyways,
  // this should be a cheap operation in an optimized build.
  mutable const ctrl_t * ptr_;

  void init(const ctrl_t * c) noexcept {
    if (c) {
      ++(c->ref_count_);
    }
    ptr_ = c;
  }

  void move(weak_ref & o) noexcept {
    ptr_ = o.ptr_;
    o.ptr_ = nullptr;
  }

  // This is const qualified so that dereference can be const qualified, and
  // *still* result in release if the master_ref is gone.
  void release() const noexcept {
    if (ptr_) {
      if (!--(ptr_->ref_count_)) {
        if (!ptr_->payload_) { // Check if the master is still pointing to it.
          delete ptr_;
        }
      }
      ptr_ = nullptr;
    }
  }

public:
  typedef T element_type;

  // Special member functions
  constexpr weak_ref() noexcept : ptr_(nullptr) {}
  weak_ref(const weak_ref & o) noexcept {
    this->init(o.ptr_);
  }
  weak_ref(weak_ref && o) noexcept { this->move(o); }
  ~weak_ref() noexcept { this->release(); }

  weak_ref & operator = (const weak_ref & o) noexcept {
    this->release();
    this->init(o.ptr_);
    return *this;
  }

  weak_ref & operator = (weak_ref && o) noexcept {
    this->release();
    this->move(o);
    return *this;
  }

  // Construct from master_ref
  explicit weak_ref(const master_ref<T> & u) noexcept {
    this->init(u.ptr_);
  }

  weak_ref & operator = (const master_ref<T> & u) noexcept {
    this->release();
    this->init(u.ptr_);
    return *this;
  }

  // Swap
  void swap(weak_ref & o) noexcept {
    const ctrl_t * temp = ptr_;
    ptr_ = o.ptr_;
    o.ptr_ = temp;
  }

  // Reset is not const qualified, from user perspective this makes the most sense.
  void reset() noexcept {
    this->release();
  }

  // Lock: Obtain the payload if possible, otherwise return nullptr
  T * lock() const noexcept {
    if (ptr_) {
      T * result = ptr_->payload_;
      if (!result) { this->release(); }
      return result;
    }
    return nullptr;
  }

  // Expired: cast this->lock() to bool
  bool expired() const noexcept {
    return static_cast<bool>(this->lock());
  }

  // use_count: mimic std::shared_ptr interface
  long use_count() const noexcept {
    return this->expired() ? 1 : 0;
  }

  // weak_ref_count: do something more useful :)
  long weak_ref_count() const noexcept {
    if (ptr_) {
      if (ptr_->payload_) {
        return ptr_->ref_count_;
      }
      this->release();
    }
    return 0;
  }
};

/***
 * This is a helper for using the class properly. Since you don't want the
 * master_ref to outlive the pointed object, it makes sense to hold them in the
 * same structure.
 */

template <typename T>
struct weakly_referenced {
  T object;

private:
  master_ref<T> ref;

public:
  template <typename... Args>
  explicit weakly_referenced(Args && ... args)
    : object(std::forward<Args>(args)...)
    , ref(&object)
  {}

  weak_ref<T> get_weak_ref() const {
    return weak_ref<T>{this->ref};
  }
};

} // end namespace nonstd

#endif // NONSTD_WEAK_REF_HPP_INCLUDED
