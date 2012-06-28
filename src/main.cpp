/////////////////////////////////////////////////////////////////////////////
// Name:        src/main.cpp
// Purpose:     entry point for standalone executable running lua script
// Author:      Akiva Miura <akiva.miura@gmail.com>
// Modified by:
// Created:     12/02/2010
// Copyright:   (C) 2010 Akiva Miura
// Licence:     MIT License
/////////////////////////////////////////////////////////////////////////////

// pre-compiled header
#include "prec.h"

// dependencies
#include "lev/debug.hpp"
#include "lev/entry.hpp"
#include "lev/fs.hpp"
#include "lev/package.hpp"
#include "lev/system.hpp"

// libraries
#include <allegro5/allegro_native_dialog.h>
#include <boost/format.hpp>
#include <iostream>
#include <string>

using namespace lev;
using namespace luabind;

const char *entry_dir[] = {"entry"};
const char *entry_files[] = {"entry.lc", "entry.lua", "entry.txt"};
const int entry_files_len = 3;

//static int traceback (lua_State *L) {
//  if (!lua_isstring(L, 1))  /* 'message' not a string? */
//    return 1;  /* keep it intact */
//  lua_getfield(L, LUA_GLOBALSINDEX, "debug");
//  if (!lua_istable(L, -1)) {
//    lua_pop(L, 1);
//    return 1;
//  }
//  lua_getfield(L, -1, "traceback");
//  if (!lua_isfunction(L, -1)) {
//    lua_pop(L, 2);
//    return 1;
//  }
//  lua_pushvalue(L, 1);  /* pass error message */
//  lua_pushinteger(L, 2);  /* skip this function and traceback */
//  lua_call(L, 2, 1);  /* call debug.traceback */
//  return 1;
//}

//static int print_file_and_line(lua_State *L)
//{
//   lua_Debug d;
//   lua_getstack(L, 2, &d);
//   lua_getinfo(L, "Sln", &d);
//   std::string error_message =
//     (boost::format("error on lua script execution at %1%:%2%") % d.short_src % d.currentline).str();
//   lev::debug_print(error_message);
//   return 1;
//}

//static bool do_file(lua_State *L, const std::string &filename)
//{
////    if (luaL_dofile(L, filename.c_str()))
////    lua_pushcfunction(L, print_file_and_line);
//  lua_pushcfunction(L, traceback);
//  if (luaL_loadfile(L, filename.c_str()) || lua_pcall(L, 0, LUA_MULTRET, 1))
//  {
//    lev::debug_print(lua_tostring(L, -1));
//    return false;
//  }
//  lua_pop(L, 1);
//  return true;
//}


static bool do_string(lua_State *L, const std::string &str)
{
  if (luaL_dostring(L, str.c_str()))
  {
    lev::debug_print(lua_tostring(L, -1));
    return false;
  }
  return true;
}


static bool set_args(lua_State *L, int argc, char **argv, int axis)
{
  globals(L)["arg"] = newtable(L);
  object arg = globals(L)["arg"];
  for (int i = 0; i < argc; i++)
  {
    arg[i - axis] = std::string(argv[i]);
  }
  return true;
}


static bool set_libs(lua_State *L)
{
  // open basic (and critical) libs
  lua_pushcfunction(L, &luaopen_base);
  lua_pushstring(L, LUA_COLIBNAME);
  lua_call(L, 1, 0);
  lua_pushcfunction(L, &luaopen_package);
  lua_pushstring(L, LUA_LOADLIBNAME);
  lua_call(L, 1, 0);

  // lev library registration
  register_to(globals(L)["package"]["preload"], LUA_DBLIBNAME, &luaopen_debug);
  register_to(globals(L)["package"]["preload"], LUA_IOLIBNAME, &luaopen_io);
  register_to(globals(L)["package"]["preload"], LUA_MATHLIBNAME, &luaopen_math);
  register_to(globals(L)["package"]["preload"], LUA_OSLIBNAME, &luaopen_os);
  register_to(globals(L)["package"]["preload"], LUA_STRLIBNAME, &luaopen_string);
  register_to(globals(L)["package"]["preload"], LUA_TABLIBNAME, &luaopen_table);
  lev::set_preloaders(L);

  return true;
}


static bool execute_path(lua_State *L, const std::string &path_str)
{
//printf("EXECUTE PATH: %s\n", path_str.c_str());
  // initialization of pathes
  package::clear_path_list(L);
  package::add_path(L, "./");
  if (lev::package::add_path(L, path_str))
  {
    // trying execution within the path (directory or archive)
    for (int i = 0; i < entry_files_len; i++)
    {
//printf("SEARCHING: %s %s\n", path_str.c_str(), entry_files[i]);
      boost::shared_ptr<path> p = lev::package::resolve(L, entry_files[i]);
      if (! p) { continue; }
//printf("EXISTS: %s %s\n", path_str.c_str(), entry_files[i]);
      if (! lev::package::dofile(L, p->to_str())) { continue; }
      return true;
    }
  }
  // trying direct execution
//printf("DIRECT: %s\n", path_str.c_str());
  return lev::package::dofile(L, path_str);

//  if (file_system::dir_exists(path))
//  {
//    // given path is directory
//    // run entry program in path directory
//    package::clear_path_list(L);
//    package::add_path(L, "./");
//    package::add_path(L, path);
//    for (int i = 0; i < entry_files_len; i++)
//    {
//      std::string filename = path + "/" + entry_files[i];
//      if (! file_system::file_exists(filename)) { continue; }
//      do_file(L, filename);
//      return true;
//    }
//  }
//  else if (lev::archive::is_archive(path))
//  {
//    // given path is a archive file
//    // run entry program in path archive
//    std::string entry_name;
//    for (int i = 0; i < entry_files_len; i++)
//    {
//      std::string arc_name = boost::filesystem::path(path).stem().gen    eric_string();
//      if (lev::archive::find_direct(path, entry_files[i], entry_name)
//          || lev::archive::find_direct(path, arc_name + "/" + entry_f    iles[i], entry_name))
//      {
//        std::string code;
//        package::add_path(L, path);
//        package::add_path(L, "./");
//        if (! lev::archive::read_direct(path, entry_name, code)) { co    ntinue; }
//        do_string(L, code);
//        return true;
//      }
//    }
//  }
//  else
//  {
//    // given path is a regular file
//    do_file(L, path);
//    return true;
//  }
  lev::debug_print((boost::format("Target %1% is not found.") % path_str).str());
  return false;
}


int main(int argc, char **argv)
{
  bool done_something = false;

  // initializing lua statement
  lua_State *L = lua_open();
  boost::shared_ptr<lev::system> sys = system::init(L);

  // set libraries
  set_libs(L);

  // lev entry
//  application::entry(L, argc, argv);

  for (int i = 1; i < argc; i++)
  {
    switch(argv[i][0])
    {
      case '-':
        if (argv[i][1] == 'e' && argc > i + 1)
        {
          // -e stat
          i++;
          if (! do_string(L, argv[i])) { return -1; }
          done_something = true;
        }
        else if (argv[i][1] == '\0')
        {
          // - (stdin)
          set_args(L, argc, argv, i);

          typedef std::istreambuf_iterator<char> iterator;
          std::string input(iterator(std::cin), iterator());
          if (! do_string(L, input.c_str())) { return -1; }
          done_something = true;
          i = argc; // escaping
        }
        break;
      default:
        set_args(L, argc, argv, i);
        execute_path(L, argv[i]);
        done_something = true;
    }
  }

  // nothing was done
  // run entry program
  if (! done_something)
  {
//printf("ENTRY?\n");
    if (file_system::dir_exists(entry_dir[0]))
    {
      execute_path(L, entry_dir[0]);
      done_something = true;
    }
  }

  if (! done_something)
  {
    for (int i = 0; i < entry_files_len; i++)
    {
      if (done_something) { break; }
      if (! file_system::file_exists(entry_files[i])) { continue; }
//      if (! do_file(L, entry_files[i])) { return -1; }
      if (! lev::package::dofile(L, entry_files[i])) { return -1; }
      done_something = true;
    }
  }

  if (! done_something)
  {
    luaL_dostring(L, "");
    sys->start_debug();
    al_show_native_message_box(NULL, "Usage", "Usage:", "create \"entry.txt\" file and put in the same directory with this program", "OK", 0);
//    lev::debug_print("Usage: create \"entry.txt\" file and put in the same directory with the program");
  }

//  if (sys->is_running())
//  {
//    while (sys && sys->is_debugging())
//    {
//      try {
//        sys->do_event();
//      }
//      catch (std::exception &e) {
//        lev::debug_print(e.what());
//      }
//    }
//  }
  sys->done();
  lua_gc(L, LUA_GCCOLLECT, 0);
  lua_close(L);

  return 0;
}

