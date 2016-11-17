#include <cassert>
#include <new>
#include <string>
#include <type_traits>
#include <utility>

/***
 * This is a small test, meant to help
 * isolate a bug when compiling with msvc.
 */

namespace detail {

// Trait
template <typename T, typename ENABLE = void>
struct str_cat_helper {};

template <>
struct str_cat_helper<std::string> {
  static std::string to_string(std::string s) { return s; }
};

template <>
struct str_cat_helper<const char *> {
  static std::string to_string(const char * s) { return s; }
};

template <typename T>
struct str_cat_helper<T, typename std::enable_if<std::is_integral<T>::value>::
                           type> {
  static std::string to_string(T t) { return std::to_string(t); }
};

// Template function
inline std::string
str_cat() {
  return {};
}

template <typename T, typename... Args>
std::string
str_cat(T && t, Args &&... args) {
  using helper_t = str_cat_helper<typename std::decay<T>::type>;
  return helper_t::to_string(std::forward<T>(t))
         + str_cat(std::forward<Args>(args)...);
}

} // end namespace detail

class error {
  class impl {
    enum class state {
      uninitialized,
      bad_alloc,
      cant_lock_vm,
      invalid_coroutine,
      dynamic_text
    };

    using string_t = std::string;

    string_t str_;
    state state_;

    // Helpers
    template <typename T>
    void initialize_string(T && t) {
      state_ = state::dynamic_text;
      str_ = std::forward<T>(t);
    }

  public:
    impl()
      : str_()
      , state_(state::uninitialized) {}
    impl(impl && other) = default;
    impl(const impl & other) = default;
    impl & operator=(impl && other) = default;
    impl & operator=(const impl & other) = default;
    ~impl() = default;

    // Construct with fixed error messages
    struct bad_alloc_tag {
      static constexpr state value = state::bad_alloc;
    };
    struct cant_lock_vm_tag {
      static constexpr state value = state::cant_lock_vm;
    };
    struct invalid_coroutine_tag {
      static constexpr state value = state::invalid_coroutine;
    };

    template <typename T, typename = decltype(T::value)>
    explicit impl(T) noexcept
      : str_()
      , state_(T::value) {}

    // Construct from string
    explicit impl(std::string s) noexcept
      : impl() {
      this->initialize_string(std::move(s));
    }

    // Access error message
    const char * c_str() const {
      switch (state_) {
        case state::uninitialized:
          return "uninitialized error message";
        case state::bad_alloc:
          return "bad_alloc";
        case state::cant_lock_vm:
          return "couldn't access the lua VM";
        case state::invalid_coroutine:
          return "invalid coroutine";
        case state::dynamic_text:
          return str_.c_str();
        default:
          return "invalid error message state";
      }
    }

    // Get reference to dynamic string, or convert it to such.
    std::string & str() {
      if (state_ != state::dynamic_text) {
        this->initialize_string(this->c_str());
      }
      return str_;
    }
  };

  impl msg_;

  explicit error(impl m)
    : msg_(std::move(m)) {}

  //->
public:
  // Defaulted special member functions
  error() = default;
  error(const error &) = default;
  error(error &&) = default;
  error & operator=(const error &) = default;
  error & operator=(error &&) = default;
  ~error() = default;

  // General constructor
  // Takes a sequence of strings, string literals, or numbers
  // and concatenates them to form the message.
  template <typename... Args>
  explicit error(Args &&... args) noexcept;

  // Accessor
  const char * what() const noexcept;
  const char * c_str() const noexcept { return this->what(); }
  std::string str() const { return this->what(); }
};

template <typename... Args>
inline error::error(Args &&... args) noexcept {
  msg_ = impl{detail::str_cat(std::forward<Args>(args)...)};
}

inline const char *
error::what() const noexcept {
  return msg_.c_str();
}


int main() {
  error e1{"foo"};
  assert(e1.str() == "foo");

  error e2{"bar", 5};
  assert(e2.str() == "bar5");
}
