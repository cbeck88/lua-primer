#include <primer/lua.hpp>
#include <ostream>
#include <string>
#include <map>

/***
 * Feature which permits to dump a snapshot of the _G table of a state, in order
 * to compare it at a later time.
 */

struct lua_value {
  int type;
  std::string desc;

  bool operator < (const lua_value & o) const {
    return type < o.type || (type == o.type && desc < o.desc);
  }

  explicit lua_value(lua_State * L, int index)
    : lua_type(L, index)
    , desc(  (lua_pushvalue(L, index), luaL_tolstring(L, -1)))
  {
    lua_pop(L, 1);
  }

  bool matches(const lua_value & o) {
    if (type != o.type) { return false; }
    if (type == LUA_TTABLE || type == LUA_TUSERDATA) { return true; }
    return desc == o.desc;
  }
};

using std::map<lua_value, lua_value> table_summary;

inline table_summary get_global_table_summary(lua_State * L) {
  PRIMER_ASSERT_STACK_NEUTRAL(L);
  table_summary result;

  lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_GLOBALS_RIDX);
  int idx = lua_gettop(L);
  lua_pushnil(L);
  while (lua_next(L, idx)) {
    {
      PRIMER_ASSERT_STACK_NEUTRAL(L);
      lua_value key{L, -2};
      lua_value val{L, -1};

      auto p = result.emplace(std::move(key), std::move(val));
      PRIMER_ASSERT(p.second);
    }
    lua_pop(L, 1);
  }
  lua_pop(L, 1);

  return result;
}

inline std::ostream & operator << (std::ostream & ss, const lua_value & p) {
  ss << "~" << p.desc << "~";
  return ss;
}

inline std::ostream & operator << (std::ostream & ss, const std::pair<lua_value,lua_value> & p) {
  ss << "[" << p.first << "] = " << p.second;
  return ss;
}

inline std::ostream & operator << (std::ostream & ss, const std::map<lua_value,lua_value> & m) {
  ss << "{\n";
  for (const auto & p : m) {
    ss << "  " p << std::endl;
  }
  ss << "}";
  return ss;
}

inline bool check_table_subset(const char * lname, const table_summary & lhs, const char * rname, const table_summary & rhs) {
  bool result = true;
  for (const auto & p : lhs) {
    auto it = rhs.find(p.first);
    if (it != rhs.end() && !p.second.matches(it->second)) {
      std::cerr << "!!!\n"
      std::cerr << "    " << lname << ": " << p << std::endl;
      std::cerr << "    " << rname << ": " << *it << std::endl;
      result = false;
    } else {
      std::cerr << "!!!\n"
      std::cerr << "    " << lname << ": " << p << std::endl;
      std::cerr << "    " << rname << ": [" << p.first << "] = nil" << std::endl;
      result = false;
    }
  }
  return result;
}

inline bool check_tables_match(const table_summary & lhs, const table_summary & rhs) {
  bool b1 = check_table_subset("LHS", lhs, "RHS", rhs);
  bool b2 = check_table_subset("RHS", rhs, "LHS", lhs);

  return b1 && b2;
}
