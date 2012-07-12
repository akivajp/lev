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
#include "lev/archive.hpp"
#include "lev/debug.hpp"
#include "lev/entry.hpp"
#include "lev/fs.hpp"
#include "lev/util.hpp"
#include "lev/system.hpp"

// libraries
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
      def("find_font", &package::find_font, raw(_1)),
      def("find_font", &package::find_font0, raw(_1)),
      def("get_font_dirs", &package::get_font_dirs, raw(_1)),
      def("get_font_list", &package::get_font_list, raw(_1)),
      def("resolve", &package::resolve, raw(_1))
    ]
  ];
  object lev = globals(L)["lev"];
  object package = lev["package"];

  register_to(package, "add_search", &package::add_search_l);
  register_to(package, "clear_search", &package::clear_search_l);
  register_to(package, "dofile", &package::dofile_l);
  register_to(package, "require", &package::require_l);

  lev["require"] = package["require"];

  globals(L)["package"]["loaded"]["lev.package"] = package;
  return 0;
}


namespace lev
{

  static bool purge_path(std::string &path)
  {
    long double_slash = path.find("//", 1);
    while (double_slash != std::string::npos)
    {
      path.replace(double_slash, 2, "/");
      double_slash = path.find("//", 1);
    }
    return true;
  }


  bool package::add_font(lua_State *L, const std::string &filename)
  {
    luabind::open(L);
    luabind::globals(L)["require"]("lev.package");
    luabind::object t = luabind::globals(L)["lev"]["package"]["get_font_list"]();
    luabind::globals(L)["table"]["insert"](t, 1, filename);
    return true;
  }

  bool package::add_font_dir(lua_State *L, const std::string &dir)
  {
    luabind::open(L);
    luabind::globals(L)["require"]("lev.package");
    luabind::object t = luabind::globals(L)["lev"]["package"]["get_font_dirs"]();
    luabind::globals(L)["table"]["insert"](t, 1, dir);
  }

  bool package::add_path(lua_State *L, const std::string &path)
  {
    using namespace luabind;
    open(L);
    globals(L)["require"]("lev.package");
    if (! globals(L)["lev"]["package"]["path_list"])
    {
      globals(L)["lev"]["package"]["path_list"] = newtable(L);
    }
    globals(L)["table"]["insert"](globals(L)["lev"]["package"]["path_list"], 1, path);
    return true;
  }

  bool package::add_search(lua_State *L, const std::string &search)
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
    globals(L)["table"]["insert"](globals(L)["lev"]["package"]["search_list"], 1, search);
    return true;
  }

  int package::add_search_l(lua_State *L)
  {
    using namespace luabind;
    const char *search = NULL;

    object t = util::get_merged(L, 1, -1);
    if (t["search_path"]) { search = object_cast<const char *>(t["search_path"]); }
    else if (t["search"]) { search = object_cast<const char *>(t["search"]); }
    else if (t["path"]) { search = object_cast<const char *>(t["path"]); }
    else if (t["lua.string1"]) { search = object_cast<const char *>(t["lua.string1"]); }
    if (search == NULL) { luaL_error(L, "path (string) is not given"); }

    lua_pushboolean(L, package::add_search(L, search));
    return 1;
  }

  int package::clear_search_l(lua_State *L)
  {
    using namespace luabind;
    module(L, "lev") [ namespace_("package") ];
    globals(L)["lev"]["package"]["search_path"] = luabind::nil;
    lua_pushboolean(L, true);
    return 1;
  }

  int package::dofile_l(lua_State *L)
  {
    using namespace luabind;

    luaL_checkstring(L, 1);
    std::string filename = object_cast<const char *>(object(from_stack(L, 1)));

    object path_list = package::get_path_list(L);
    file::ptr f = package::resolve(L, filename);
    if (f)
    {
      std::string data;
      if (! f->read_all(data)) { return 0; }
//      if (luaL_dofile(L, fpath->get_string().c_str()) != 0)
      if (luaL_dostring(L, data.c_str()) != 0)
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

      for (iterator i(dirs), end; i != end; i++)
      {
        std::string path = object_cast<const char *>(*i);
        if (fs::is_file(path + "/" + filename))
        {
          f = font::load(path + "/" + filename);
          if (f) { break; }
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
      globals(L)["table"]["insert"](t, "./");
      globals(L)["table"]["insert"](t, "./fonts");
      globals(L)["table"]["insert"](t, "/usr/share/fonts/corefonts");
      globals(L)["table"]["insert"](t, "C:/system/fonts");
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
      globals(L)["table"]["insert"](t, "default.ttf");
      globals(L)["table"]["insert"](t, "arial.ttf");
    }
    return globals(L)["lev"]["package"]["font_list"];
  }

  luabind::object package::get_path_list(lua_State *L)
  {
    using namespace luabind;
    open(L);
    module(L, "lev")
    [
      namespace_("package")
    ];
    if (! globals(L)["lev"]["package"]["path_list"])
    {
      object path_list = newtable(L);
      path_list[1] = "./";
      globals(L)["lev"]["package"]["path_list"] = path_list;
    }
    return globals(L)["lev"]["package"]["path_list"];
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
    file::ptr f = package::resolve(L, module);
    if (! f) { f = package::resolve(L, module + ".lua"); }
    if (f)
    {
      std::string data;
      if (! f->read_all(data)) { return 0; }
//      if (luaL_dofile(L, fpath->get_string().c_str()))
      if (luaL_dostring(L, data.c_str()))
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

    try {
      object result = globals(L)["require"](module);
      result.push(L);
      return 1;
    }
    catch (...) {
      luaL_error(L, ("module '" + module + "' not found").c_str());
      return 0;
    }
  }

//  filepath::ptr package::resolve(lua_State *L, const std::string &file)
  file::ptr package::resolve(lua_State *L, const std::string &file)
  {
    using namespace luabind;

    try {
      object path_list   = package::get_path_list(L);
      object search_list = package::get_search_list(L);

      for (iterator p(path_list), end; p != end; p++)
      {
        std::string path = object_cast<const char *>(*p);

        for (iterator s(search_list); s != end; s++)
        {
          std::string search = object_cast<const char *>(*s);

          std::string real_path = path + "/" + search + "/" + file;
          purge_path(real_path);

          if (fs::is_file(real_path))
          {
            return file::open(real_path, "rb");
//            return filepath::create(real_path);
          }
        }

        if (lev::archive::is_archive(path))
        {
          for (iterator s(search_list); s != end; s++)
          {
            std::string entry = object_cast<const char *>(*s);
            if (entry.empty()) { entry = file; }
            else { entry = entry + "/" + file; }
            if (archive::entry_exists_direct(path, entry))
            {
              std::string sys_name = ".";
              if (system::get()) { sys_name = system::get()->get_name(); }
              std::string ext = fs::to_extension(file);

//              filepath::ptr fpath(filepath::create_temp(sys_name + "/", ext));
//              if (! fpath) { return fpath; }
//              lev::archive::extract_direct_to(path, entry, fpath->get_string());
//              return fpath;
              return archive::extract_direct(path, entry, NULL);
            }
          }

          std::string arc_name = fs::to_stem(path);
          for (iterator s(search_list); s != end; s++)
          {
            std::string entry = object_cast<const char *>(*s);
            if (entry.empty()) { entry = arc_name + "/" + file; }
            else { entry = arc_name + "/" + entry + "/" + file; }
            if (archive::entry_exists_direct(path, entry))
            {
              std::string sys_name = ".";
              if (system::get()) { sys_name = system::get()->get_name(); }
              std::string ext = fs::to_extension(file);

//              filepath::ptr fpath(filepath::create_temp(sys_name + "/", ext));
//              if (! fpath) { return fpath; }
//              lev::archive::extract_direct_to(path, entry, fpath->get_string());
//              return fpath;
              return archive::extract_direct(path, entry, NULL);
            }
          }
        }
      }
    }
    catch (...) {
      lev::debug_print("error on file path resolving");
    }
    return file::ptr();
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


