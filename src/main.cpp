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
//#include "lev/archive.hpp"
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

static bool do_file(lua_State *L, const char *filename)
{
  if (luaL_dofile(L, filename))
  {
    fprintf(stderr, "%s\n", lua_tostring(L, -1));
    return -1;
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


int main(int argc, char **argv)
{
  const int len = 3;
  const char *entry[] = {"entry.lc", "entry.lua", "entry.txt"};
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
        if (file_system::dir_exists(argv[i]))
        {
          // argv[i] is directory
          // run entry program in argv[i] directory
//          package::add_path(L, file_system::to_full_path(argv[i]));
          package::add_path(L, argv[i]);
          for (int j = 0; j < len; j++)
          {
            std::string filename = argv[i];
            filename = filename + "/" + entry[j];
            if (! file_system::file_exists(filename)) { continue; }
            if (! do_file(L, filename.c_str())) { return -1; }
            done_something = true;
            break;
          }
          if (! done_something)
          {
            fprintf(stderr, "%s\n", lua_tostring(L, -1));
            return -1;
          }
        }
        else
        {
//          // argv[i] is archive or script file
//          if (lev::archive::is_archive(argv[i]))
//          {
//            // argv[i] is archive
//            boost::shared_ptr<file_path> path(file_path::create(argv[i]));
//            std::string entry_name;
//            for (int j = 0; j < len; j++)
//            {
//              if (lev::archive::find_direct(argv[i], entry[j], entry_name)
//                  || lev::archive::find_direct(argv[i], path->get_name() + "/" + entry[j], entry_name))
//              {
//                std::string code;
//                package::add_path(L, file_system::to_full_path(argv[i]));
//                if (! lev::archive::read_direct(argv[i], entry_name, code)) { continue; }
//                if (! do_string(L, code)) { return -1; }
//                done_something = true;
//                break;
//              }
//              else { continue; }
//            }
//          }
//          else
          {
            if (! do_file(L, argv[i])) { return -1; }
          }
        }
        done_something = true;
        i = argc; // escaping
        break;
    }
  }

  // nothing was done
  // run entry program
  for (int i = 0; i < len; i++)
  {
    if (done_something) { break; }
    if (! file_system::file_exists(argv[i])) { continue; }
    if (! do_file(L, entry[i])) { return -1; }
    done_something = true;
  }
  if (! done_something)
  {
    fprintf(stderr, "Usage: create \"entry.txt\" file and put in the same directory with the program\n");
//    wxMessageBox(_("Usage: create \"entry.txt\" file and put in the same directory with the program"), _("About usage"));
    return -1;
  }

  lua_close(L);

  return 0;
}

