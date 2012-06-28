/////////////////////////////////////////////////////////////////////////////
// Name:        src/package.cpp
// Purpose:     source for package management
// Author:      Akiva Miura <akiva.miura@gmail.com>
// Modified by:
// Created:     09/01/2011
// Copyright:   (C) 2010-2012 Akiva Miura
// Licence:     MIT License
/////////////////////////////////////////////////////////////////////////////

// pre-compiled header
#include "prec.h"

// declarations
#include "lev/package.hpp"

// dependencies
//#include "lev/archive.hpp"
#include "lev/debug.hpp"
#include "lev/entry.hpp"
#include "lev/fs.hpp"
#include "lev/util.hpp"
#include "lev/system.hpp"

// libraries
#include <physfs.h>
#include <luabind/raw_policy.hpp>
#include <luabind/luabind.hpp>

int luaopen_lev_package(lua_State *L)
{
  using namespace lev;
  using namespace luabind;

  open(L);
  globals(L)["require"]("table");

  module(L, "lev")
  [
    namespace_("package")
    [
      def("add_font", &package::add_font, raw(_1)),
      def("add_font_dir", &package::add_font_dir, raw(_1)),
      def("add_path", &package::add_path, raw(_1)),
      def("add_path", &package::add_path1, raw(_1)),
      def("clear_path_list", &package::clear_path_list, raw(_1)),
      def("find_font", &package::find_font, raw(_1)),
      def("find_font", &package::find_font0, raw(_1)),
      def("get_archive_list", &package::get_archive_list, raw(_1)),
      def("get_font_dirs", &package::get_font_dirs, raw(_1)),
      def("get_font_list", &package::get_font_list, raw(_1)),
      def("get_path_list", &package::get_path_list, raw(_1)),
      def("get_search_list", &package::get_search_list, raw(_1)),
      def("resolve", &package::resolve, raw(_1))
    ]
  ];
  object lev = globals(L)["lev"];
  object package = lev["package"];

  register_to(package, "add_search", &package::add_search_l);
  register_to(package, "clear_search", &package::clear_search_l);
  register_to(package, "dofile", &package::dofile_l);
  register_to(package, "require", &package::require_l);

  lev["dofile"]  = package["dofile"];
  lev["require"] = package["require"];

  globals(L)["package"]["loaded"]["lev.package"] = package;
  return 0;
}


namespace lev
{

  static int traceback (lua_State *L) {
    if (!lua_isstring(L, 1))  /* 'message' not a string? */
      return 1;  /* keep it intact */
    lua_getfield(L, LUA_GLOBALSINDEX, "debug");
    if (!lua_istable(L, -1)) {
      lua_pop(L, 1);
      return 1;
    }
    lua_getfield(L, -1, "traceback");
    if (!lua_isfunction(L, -1)) {
      lua_pop(L, 2);
      return 1;
    }
    lua_pushvalue(L, 1);  /* pass error message */
    lua_pushinteger(L, 2);  /* skip this function and traceback */
    lua_call(L, 2, 1);  /* call debug.traceback */
    return 1;
  }

  static int dobuffer(lua_State *L, const std::string &name, const std::string &data)
  {
    lua_pushcfunction(L, traceback);
    int trace_pos = lua_gettop(L);
    int result = luaL_loadbuffer(L, data.c_str(), data.length(), name.c_str())
               || lua_pcall(L, 0, LUA_MULTRET, trace_pos);
    lua_remove(L, trace_pos);
    return result;
  }

  static bool purge_path(std::string &path)
  {
    long double_slash = path.find("//", 1);
    while (double_slash != std::string::npos)
    {
      path.replace(double_slash, 2, "/");
      double_slash = path.find("//", 1);
    }
    if (path[0] == '/') { path.replace(0, 1, ""); }
    return true;
  }


  bool package::add_font(lua_State *L, const std::string &filename)
  {
    using namespace luabind;
    open(L);
    globals(L)["require"]("lev.package");
    object t = globals(L)["lev"]["package"]["get_font_list"]();
    globals(L)["table"]["insert"](t, 1, filename);
    return true;
  }

  bool package::add_font_dir(lua_State *L, const std::string &dir)
  {
    using namespace luabind;
    open(L);
    globals(L)["require"]("lev.package");
    object t = globals(L)["lev"]["package"]["get_font_dirs"]();
    globals(L)["table"]["insert"](t, 1, dir);
  }

  bool package::add_path(lua_State *L, const std::string &path, bool append)
  {
    using namespace luabind;

    if (! PHYSFS_isInit()) { return false; }
    if (! PHYSFS_addToSearchPath(path.c_str(), append)) { return false; }
//    if (file_system::file_exists(path, false))
//    {
//      // archive was added
//      std::string arc_name = file_system::to_stem(path);
//      if (file_system::dir_exists(arc_name, true))
//      {
//        // directory with archive name exists
//        // adding this archive name directory to search prefix
//        add_search(L, arc_name + "/", append);
//      }
//    }
    return true;
  }

  bool package::add_search(lua_State *L, const std::string &search, bool append)
  {
    using namespace luabind;
    open(L);
    globals(L)["require"]("table");
    module(L, "lev")
    [
      namespace_("package")
    ];
    if (! globals(L)["lev"]["package"]["search_list"])
    {
      globals(L)["lev"]["package"]["search_list"] = newtable(L);
    }

    if (append)
    {
      globals(L)["table"]["insert"](globals(L)["lev"]["package"]["search_list"], search);
    }
    else
    {
      globals(L)["table"]["insert"](globals(L)["lev"]["package"]["search_list"], 1, search);
    }
    return true;
  }

  int package::add_search_l(lua_State *L)
  {
    using namespace luabind;
    const char *search = NULL;
    bool append = true;

    object t = util::get_merged(L, 1, -1);
    if (t["search_path"]) { search = object_cast<const char *>(t["search_path"]); }
    else if (t["search"]) { search = object_cast<const char *>(t["search"]); }
    else if (t["path"]) { search = object_cast<const char *>(t["path"]); }
    else if (t["lua.string1"]) { search = object_cast<const char *>(t["lua.string1"]); }
    if (search == NULL) { luaL_error(L, "path (string) is not given"); }

    if (util::is_boolean(t["appending"])) { append = object_cast<bool>(t["appending"]); }
    else if (util::is_boolean(t["append"])) { append = object_cast<bool>(t["append"]); }
    else if (util::is_boolean(t["a"])) { append = object_cast<bool>(t["a"]); }
    else if (util::is_boolean(t["lua.boolean1"])) { append = object_cast<bool>(t["lua.boolean1"]); }

    lua_pushboolean(L, package::add_search(L, search, append));
    return 1;
  }

  bool package::clear_path_list(lua_State *L)
  {
    if (! PHYSFS_isInit()) { return false; }
    char **list = PHYSFS_getSearchPath();
    for (char **i = list; *i != NULL; i++)
    {
      PHYSFS_removeFromSearchPath(*i);
    }
    PHYSFS_freeList(list);
    return true;
  }

  int package::clear_search_l(lua_State *L)
  {
    using namespace luabind;
    luabind::module(L, "lev") [ namespace_("package") ];
    luabind::globals(L)["lev"]["package"]["search_path"] = luabind::nil;
    lua_pushboolean(L, true);
    return 1;
  }

  bool package::dofile(lua_State *L, const std::string &filename)
  {
    using namespace luabind;

    boost::shared_ptr<path> fpath = package::resolve(L, filename);
    if (fpath)
    {
      std::string data;
      if (! fpath->read_all(data))
      {
        return false;
      }
      if (dobuffer(L, fpath->to_str(), data) != 0)
      {
        lev::debug_print(lua_tostring(L, -1));
        return false;
      }
      return true;
    }

    lev::debug_print("cannnot open " + filename + ": No such file or directory");
    return true;
  }

  int package::dofile_l(lua_State *L)
  {
    using namespace luabind;

    luaL_checkstring(L, 1);
    std::string filename = object_cast<const char *>(object(from_stack(L, 1)));

    boost::shared_ptr<path> fpath = package::resolve(L, filename);
    if (fpath)
    {
      std::string data;
      if (! fpath->read_all(data))
      {
        lua_pushnil(L);
        return 1;
      }
      if (dobuffer(L, fpath->to_str(), data) != 0)
      {
        lev::debug_print(lua_tostring(L, -1));
        lua_pushnil(L);
        return 1;
      }
      lua_pushboolean(L, true);
      return 1;
    }

    luaL_error(L, ("cannnot open " + filename + ": No such file or directory").c_str());
    return 0;
  }

  boost::shared_ptr<font> package::find_font(lua_State *L, const std::string &filename)
  {
    using namespace luabind;
    boost::shared_ptr<font> f;
    try {
      globals(L)["require"]("lev.font");
      object dirs = package::get_font_dirs(L);

//printf("FIND FONT: %s\n", filename.c_str());
      for (iterator i(dirs), end; i != end; i++)
      {
        std::string path = object_cast<const char *>(*i);
//printf("FIND FONT DIR: %s\n", path.c_str());
        if (file_system::file_exists(path + "/" + filename))
        {
          f = font::load(path + "/" + filename);
          if (f) {
//printf("FIND EXISTS: %s\n", (path + "/" + filename).c_str());
            return f;
          }
        }
      }
    }
    catch (...) {
      f.reset();
      lev::debug_print(lua_tostring(L, -1));
      lev::debug_print("error on font finding");
    }
    return f;
  }

  boost::shared_ptr<font> package::find_font0(lua_State *L)
  {
    using namespace luabind;
    boost::shared_ptr<font> f;
    try {
      globals(L)["require"]("lev.font");
      object fonts = package::get_font_list(L);

      for (iterator i(fonts), end; i != end; i++)
      {
        std::string filename = object_cast<const char *>(*i);
        f = package::find_font(L, filename);
        if (f) { break; }
      }
    }
    catch (...) {
      f.reset();
      lev::debug_print(lua_tostring(L, -1));
      lev::debug_print("error on font finding\n");
    }
    return f;
  }

  luabind::object package::get_archive_list(lua_State *L)
  {
    using namespace luabind;

    object archives = newtable(L);
    if (! PHYSFS_isInit()) { return archives; }
    char **list = PHYSFS_getSearchPath();
    for (char **i = list; *i != NULL; i++)
    {
      if (file_system::file_exists(*i, false))
      {
        globals(L)["table"]["insert"](archives, *i);
      }
    }
    return archives;
  }

  luabind::object package::get_font_dirs(lua_State *L)
  {
    using namespace luabind;

    open(L);
    module(L, "lev")
    [
      namespace_("package")
    ];
    if (! globals(L)["lev"]["package"]["font_dirs"])
    {
      object t = newtable(L);
      globals(L)["lev"]["package"]["font_dirs"] = t;
//      globals(L)["table"]["insert"](t, "./");
      globals(L)["table"]["insert"](t, "fonts");
//      globals(L)["table"]["insert"](t, "/usr/share/fonts/corefonts");
//      globals(L)["table"]["insert"](t, "C:/system/fonts");
      return t;
    }
    return globals(L)["lev"]["package"]["font_dirs"];
  }

  luabind::object package::get_font_list(lua_State *L)
  {
    using namespace luabind;

    open(L);
    globals(L)["require"]("lev.package");
    if (! globals(L)["lev"]["package"]["font_list"])
    {
      object t = newtable(L);
      globals(L)["lev"]["package"]["font_list"] = t;
//      globals(L)["table"]["insert"](t, "arial.ttf");
      globals(L)["table"]["insert"](t, "default.ttf");
    }
    return globals(L)["lev"]["package"]["font_list"];
  }

  luabind::object package::get_path_list(lua_State *L)
  {
    using namespace luabind;

    object path_list = newtable(L);
    if (! PHYSFS_isInit())
    {
      globals(L)["table"]["insert"](path_list, ".");
    }
    else
    {
      char **list = PHYSFS_getSearchPath();
      for (char **i = list; *i != NULL; i++)
      {
        globals(L)["table"]["insert"](path_list, *i);
      }
      PHYSFS_freeList(list);
    }
    return path_list;
  }

  luabind::object package::get_search_list(lua_State *L)
  {
    using namespace luabind;

    open(L);
    module(L, "lev")
    [
      namespace_("package")
    ];
    if (! globals(L)["lev"]["package"]["search_list"])
    {
      object search_list = newtable(L);
      search_list[1] = "";
      globals(L)["lev"]["package"]["search_list"] = search_list;
    }
    return globals(L)["lev"]["package"]["search_list"];
  }

  int package::require_l(lua_State *L)
  {
    using namespace luabind;

    luaL_checkstring(L, 1);
    std::string module = object_cast<const char *>(object(from_stack(L, 1)));

    if (globals(L)["package"]["loaded"][module])
    {
      object t = globals(L)["package"]["loaded"][module];
      t.push(L);
      return 1;
    }

    object path_list = package::get_path_list(L);
    boost::shared_ptr<path> fpath = package::resolve(L, module);
    if (! fpath) { fpath = package::resolve(L, module + ".lc"); }
    if (! fpath) { fpath = package::resolve(L, module + ".lua"); }
    if (fpath)
    {
      std::string data;
      if (! fpath->read_all(data))
      {
        lua_pushnil(L);
        return 1;
      }
      if (dobuffer(L, fpath->to_str(), data) != 0)
      {
        lev::debug_print(lua_tostring(L, -1));
        lua_pushnil(L);
        return 1;
      }
      if (! globals(L)["package"]["loaded"][module])
      {
        globals(L)["package"]["loaded"][module] = true;
      }
      globals(L)["package"]["loaded"][module].push(L);
      return 1;
    }
    else
    {
      luaL_error(L, ("module '" + module + "' not found").c_str());
      return 0;
    }

    object result = globals(L)["require"](module);
    result.push(L);
    return 1;
  }

  boost::shared_ptr<path> package::resolve(lua_State *L,
                                           const std::string &file)
  {
    using namespace luabind;

    try {
      object archive_list = package::get_archive_list(L);
      object search_list = package::get_search_list(L);

      for (iterator s(search_list), end; s != end; s++)
      {
        std::string search = object_cast<const char *>(*s);
        std::string real_path = search + "/" + file;
        purge_path(real_path);
//printf("SEARCH: %s\n", search.c_str());

        // search for "search/file"
        if (file_system::file_exists(real_path))
        {
          return path::create(real_path);
        }
        // search for "arc_name/search/file"
        for (iterator a(archive_list); a != end; a++)
        {
          std::string archive = object_cast<const char *>(*a);
          std::string arc_name = file_system::to_stem(archive);
          real_path = arc_name + "/" + search + "/" + file;
//printf("ARCHIVE: %s\n", arc_name.c_str());
          purge_path(real_path);
          if (file_system::file_exists(real_path))
          {
            return path::create(real_path);
          }
        }
      }

      // search for "file"
      if (file_system::file_exists(file)) { return path::create(file); }
      // search for "arc_name/file"
      for (iterator a(archive_list), end; a != end; a++)
      {
        std::string archive = object_cast<const char *>(*a);
        std::string arc_name = file_system::to_stem(archive);
        std::string real_path = arc_name + "/" + file;
        purge_path(real_path);
        if (file_system::file_exists(real_path))
        {
          return path::create(real_path);
        }
      }
      return boost::shared_ptr<path>();
    }
    catch (...) {
      lev::debug_print("error on file path resolving");
    }
    return boost::shared_ptr<path>();
  }

  bool package::set_default_font_dirs(lua_State *L)
  {
    using namespace luabind;
    open(L);
    globals(L)["require"]("table");
    module(L, "lev")
    [
      namespace_("package")
    ];
    try {
      object list = newtable(L);
      globals(L)["lev"]["package"]["font_dirs"] = list;
#if (defined(_WIN32))
      globals(L)["table"]["insert"](list, 1, "C:/Windows/Fonts");
#elif (defined(__linux))
      globals(L)["table"]["insert"](list, 1, "/usr/share/fonts/");
#else
#endif // _WIN32
      globals(L)["table"]["insert"](list, 1, "./");
      globals(L)["table"]["insert"](list, 1, "./font");
      globals(L)["table"]["insert"](list, 1, "./fonts");
      return true;
    }
    catch (...) {
      return false;
    }
  }

}


