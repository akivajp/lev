#ifndef _UTIL_HPP
#define _UTIL_HPP

/////////////////////////////////////////////////////////////////////////////
// Name:        src/lev/util.hpp
// Purpose:     header for useful Lua functions
// Author:      Akiva Miura <akiva.miura@gmail.com>
// Created:     01/18/2011
// Copyright:   (C) 2010-2011 Akiva Miura
// Licence:     MIT License
/////////////////////////////////////////////////////////////////////////////

#include "base.hpp"
#include <luabind/luabind.hpp>

extern "C" {
  int luaopen_lev_util(lua_State *L);
}

namespace lev
{
  class util
  {
    public:
      static luabind::object copy_function(luabind::object func);
      static luabind::object copy_table(luabind::object table);
      static bool execute(const std::string &target);
      static int execute_code_l(lua_State *L);
      static luabind::object get_merged(lua_State *L, int start, int end);
      static int merge(lua_State *L);
      static bool open(const std::string &path, const std::string &type = "");
      static bool open1(const std::string &path) { return open(path); }
      static bool print_table(luabind::object t);
      static int remove_first(lua_State *L);
      static int reverse(lua_State *L);
      static std::string serialize(luabind::object var, int indent = 0);
      static std::string serialize1(luabind::object var) { return serialize(var); }
      static int using_l(lua_State *L);
  };

}

#endif // _UTIL_HPP

