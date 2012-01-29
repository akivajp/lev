#ifndef _PACKAGE_HPP
#define _PACKAGE_HPP

/////////////////////////////////////////////////////////////////////////////
// Name:        src/lev/package.hpp
// Purpose:     header for package management
// Author:      Akiva Miura <akiva.miura@gmail.com>
// Modified by:
// Created:     09/01/2011
// Copyright:   (C) 2010-2012 Akiva Miura
// Licence:     MIT License
/////////////////////////////////////////////////////////////////////////////

#include "base.hpp"
#include <luabind/luabind.hpp>
#include <string>

extern "C" {
  int luaopen_lev_package(lua_State *L);
}

namespace lev
{

  class file_path;

  class package
  {
    public:
      static bool add_font(lua_State *L, const std::string &filename);
      static int add_font_l(lua_State *L);
      static bool add_path(lua_State *L, const std::string &path);
      static int add_path_l(lua_State *L);
      static bool add_search(lua_State *L, const std::string &search);
      static int add_search_l(lua_State *L);
      static int clear_search_l(lua_State *L);
      static int dofile_l(lua_State *L);
      static const char *get_archive_dir(lua_State *L);
      static luabind::object get_font_list(lua_State *L);
      static luabind::object get_path_list(lua_State *L);
      static luabind::object get_search_list(lua_State *L);
      static int require_l(lua_State *L);
      static boost::shared_ptr<file_path> resolve(lua_State *L, const std::string &file);
      static luabind::object search_font(lua_State *L, const std::string &filename);
      static int search_font_l(lua_State *L);
      static bool set_archive_dir(lua_State *L, const std::string &archive_dir);
      static bool set_default_font_dirs(lua_State *L);
  };

}

#endif // _PACKAGE_HPP

