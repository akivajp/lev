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
#include "lev/archive.hpp"
#include "lev/entry.hpp"
#include "lev/fs.hpp"
#include "lev/package.hpp"
#include "lev/system.hpp"
#include "register.hpp"

// libraries
#include <iostream>
#include <string>

using namespace lev;
using namespace luabind;

const char *entry_dir[] = {"entry"};
const char *entry_files[] = {"entry.lc", "entry.lua", "entry.txt"};
const int entry_files_len = 3;

static bool do_file(lua_State *L, const std::string &filename)
{
  if (luaL_dofile(L, filename.c_str()))
  {
    fprintf(stderr, "%s\n", lua_tostring(L, -1));
    return false;
  }
  return true;
}


static bool do_string(lua_State *L, const std::string &str)
{
  if (luaL_dostring(L, str.c_str()))
  {
    fprintf(stderr, "%s\n", lua_tostring(L, -1));
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


static bool execute_path(lua_State *L, const std::string &path)
{
  if (file_system::dir_exists(path))
  {
    // given path is directory
    // run entry program in path directory
    package::add_path(L, "./");
    package::add_path(L, path);
    for (int i = 0; i < entry_files_len; i++)
    {
      std::string filename = path + "/" + entry_files[i];
      if (! file_system::file_exists(filename)) { continue; }
      do_file(L, filename);
      return true;
    }
  }
  else if (lev::archive::is_archive(path))
  {
    // given path is a archive file
    // run entry program in path archive
    std::string entry_name;
    for (int i = 0; i < entry_files_len; i++)
    {
      std::string arc_name = boost::filesystem::path(path).stem().generic_string();
      if (lev::archive::find_direct(path, entry_files[i], entry_name)
          || lev::archive::find_direct(path, arc_name + "/" + entry_files[i], entry_name))
      {
        std::string code;
        package::add_path(L, path);
        package::add_path(L, "./");
        if (! lev::archive::read_direct(path, entry_name, code)) { continue; }
        do_string(L, code);
        return true;
      }
    }
  }
  else
  {
    // given path is a regular file
    if (! file_system::file_exists(path)) { return false; }
    do_file(L, path);
    return true;
  }
  return false;
}


int main(int argc, char **argv)
{
  bool done_something = false;

  // initializing lua statement
  lua_State *L = lua_open();

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
        done_something = execute_path(L, argv[i]);
    }
  }

  // nothing was done
  // run entry program
  if (file_system::dir_exists(entry_dir[0]))
  {
    done_something = execute_path(L, entry_dir[0]);
  }
  for (int i = 0; i < entry_files_len; i++)
  {
    if (done_something) { break; }
    if (! file_system::file_exists(entry_files[i])) { continue; }
    if (! do_file(L, entry_files[i])) { return -1; }
    done_something = true;
  }
  if (! done_something)
  {
    fprintf(stderr, "Usage: create \"entry.txt\" file and put in the same directory with the program\n");
//    wxMessageBox(_("Usage: create \"entry.txt\" file and put in the same directory with the program"), _("About usage"));
    return -1;
  }

//printf("GC START\n");
  lua_gc(L, LUA_GCCOLLECT, 0);
//printf("BEGIN ALL CLOSING!\n");
  lua_close(L);
//printf("END ALL CLOSING!\n");

  return 0;
}

