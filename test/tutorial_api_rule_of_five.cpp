#include <cstdlib>
#include <iostream>
#include <string>

using uint = unsigned int;

#define ASSERT(X)                                                              \
  if (!(X)) {                                                                  \
    std::cerr << "Assertion failed [" << __FILE__ << __LINE__ << "]: " << #X   \
              << std::endl;                                                    \
    std::abort();                                                              \
  }

//[ primer_tutorial_example_api_rule_of_five ]

//` [*vm.hpp]

#include <memory>
#include <string>

class vm {
  struct impl;

  std::unique_ptr<impl> impl_;

public:
  vm();
  ~vm();

  vm(const vm &);
  vm(vm &&);

  vm & operator=(const vm &);
  vm & operator=(vm &&);

  void run_script(const std::string &);

  std::string serialize() const;
  void deserialize(const std::string &);
};

//` [*vm.cpp]

//=#include <vm.hpp>

#include <primer/api.hpp>
#include <primer/primer.hpp>

#include <iostream>
#include <memory>
#include <string>
#include <utility>

struct lua_raii {
  lua_State * L_;

  lua_raii()
    : L_(luaL_newstate()) {}

  ~lua_raii() { lua_close(L_); }

  operator lua_State *() const { return L_; }
};

namespace api = primer::api;

struct vm::impl : api::base<impl> {
  lua_raii lua_;

  API_FEATURE(api::callbacks, callbacks_);

  NEW_LUA_CALLBACK(a)(lua_State *)->int {
    std::cout << "a" << std::endl;
    return 0;
  }

  NEW_LUA_CALLBACK(b)(lua_State *)->int {
    std::cout << "b" << std::endl;
    return 0;
  }

  NEW_LUA_CALLBACK(c)(lua_State *)->int {
    std::cout << "c" << std::endl;
    return 0;
  }

  impl()
    : callbacks_(this) {
    ASSERT(this->initialize_api(lua_));
  }

  std::string serialize() {
    std::string result;
    ASSERT(this->persist(lua_, result));
    return result;
  }

  void deserialize(const std::string & buff) {
    ASSERT(this->unpersist(lua_, buff));
  }

  void run_script(const std::string & script) {
    lua_settop(lua_, 0);
    if (LUA_OK != luaL_loadstring(lua_, script.c_str())) {
      std::cerr << lua_tostring(lua_, -1);
      std::abort();
    }

    if (LUA_OK != lua_pcall(lua_, 0, 0, 0)) {
      std::cerr << lua_tostring(lua_, -1);
      std::abort();
    }
  }
};

/***
 * pimpl definitions
 */

vm::vm()
  : impl_(new vm::impl()) {}

vm::~vm() = default;

vm::vm(vm &&) = default;

vm::vm(const vm & m)
  : vm() // Call default ctor first
{
  std::string buffer{m.serialize()};
  impl_->deserialize(buffer);
}

vm & vm::operator=(vm &&) = default;

vm &
vm::operator=(const vm & other) {
  vm temp{other};
  *this = std::move(temp);
  return *this;
}

void
vm::run_script(const std::string & script) {
  impl_->run_script(script);
}

std::string
vm::serialize() const {
  return impl_->serialize();
}

void
vm::deserialize(const std::string & buff) {
  impl_->deserialize(buff);
}

//` [*main.cpp]

//=#include <vm.hpp>
#include <iostream>
#include <string>

void
call_funcs(vm & m) {
  m.run_script(
    "a() "
    "b() "
    "c() ");
  std::cout << std::endl;
}

void
cycle_funcs(vm & m) {
  m.run_script(
    "d = c "
    "c = b "
    "b = a "
    "a = d "
    "d = nil ");
}

int
main() {
  vm m1;
  call_funcs(m1);
  cycle_funcs(m1);
  call_funcs(m1);

  {
    vm m2;
    call_funcs(m2);
    m2 = m1;
    call_funcs(m1);
    call_funcs(m2);
  }

  call_funcs(m1);
  cycle_funcs(m1);
  call_funcs(m1);

  m1 = vm{};

  call_funcs(m1);
}

//` This example shows how we can give our VM pimpl objects proper C++
//` value-type semantics. The API consists of three trivial functions, and the
//` scripts `call_funcs` and `cycle_funcs` variously shuffle these functions
//` around and dump the current arrangement. The example shows that things like
//` copy assignment and move assignment work as expected here. The expected
//` output is:

//=a
//=b
//=c
//=
//=c
//=a
//=b
//=
//=a
//=b
//=c
//=
//=c
//=a
//=b
//=
//=c
//=a
//=b
//=
//=c
//=a
//=b
//=
//=b
//=c
//=a
//=
//=a
//=b
//=c

//` It's worth it to mention a non-obvious optimization in the above program:
//` Ccopying the VM this way can ['potentially] be made
//` slightly more efficient, by eliminating the use of the buffer.  
//`
//` Under the hood, eris' dump and undump functions are implemented in terms
//` of incremental read and write functions, and don't require the use of a
//` giant presized buffer.  
//` 
//` `primer`'s functions wrap over this by trivially
//` buffering to and from a `std::string`, but in principle if your goal is
//` just to copy a VM, you could do without the buffer, and push bytes from
//` one VM directly to the other, in producer / consumer fashion.  
//`  
//` The reason it isn't that way right now is that  
//`
//` * It's more complex to implement, and potentially to debug.
//` * It's more complex to figure out how to do error handling in that case --
//`   it's not clear to me that the reader / writer functions are permitted to
//`   raise lua errors, so there might be some hackery involved.
//` * It's not clear that it would really be that much faster, and I didn't
//`   really need it.
//`  
//` If you decide to implement this, you could contribute it
//` as a patch, it would be quite welcome!

//]
