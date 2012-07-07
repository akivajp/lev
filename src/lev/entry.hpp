#ifndef _ENTRY_HPP
#define _ENTRY_HPP

/////////////////////////////////////////////////////////////////////////////
// Name:        lev/entry.hpp
// Purpose:     header for binding the C++ library to lua script
// Author:      Akiva Miura <akiva.miura@gmail.com>
// Created:     12/01/2010
// Copyright:   (C) 2010-2011 Akiva Miura
// Licence:     MIT License
/////////////////////////////////////////////////////////////////////////////

#include <luabind/luabind.hpp>

extern "C"
{
  extern int luaopen_lev(lua_State *L);
  extern int luaopen_lev_std(lua_State *L);
}

namespace lev
{
  bool load_to(luabind::object to, const char *funcname, const char *code);
  bool register_to(luabind::object to, const char *funcname, lua_CFunction func);
  void set_preloaders(lua_State *L);
}

#endif // _ENTRY_HPP

