#ifndef _FS_HPP
#define _FS_HPP

/////////////////////////////////////////////////////////////////////////////
// Name:        src/lev/fs.hpp
// Purpose:     header for filesystem management classes
// Author:      Akiva Miura <akiva.miura@gmail.com>
// Modified by:
// Created:     14/07/2011
// Copyright:   (C) 2010-2012 Akiva Miura
// Licence:     MIT License
/////////////////////////////////////////////////////////////////////////////

#include "base.hpp"
#include <luabind/luabind.hpp>

extern "C" {
  int luaopen_lev_fs(lua_State *L);
}

namespace lev
{

//  class temp_name : public base
//  {
//    protected:
//      temp_name();
//    public:
//      virtual ~temp_name();
//      static temp_name* create(const std::string &prefix = "temp", const std::string &suffix = "");
//      static int create_l(lua_State *L);
//      const std::string& get_name() { return path; }
//      virtual type_id get_type_id() const { return LEV_TTEMP_NAME; }
//      virtual const char *get_type_name() const { return "lev.fs.temp_name"; }
//    protected:
//      std::string path;
//  };


  class file_path : public base
  {
//    protected:
//      file_path();
//    public:
//      virtual ~file_path();
//      bool clear();
//      static file_path* create(const std::string &path);
//      static int create_l(lua_State *L);
//      static file_path* create_temp(const std::string &prefix = "temp",
//                                    const std::string &suffix = "");
//      static int create_temp_l(lua_State *L);
//      bool dir_exists();
//      bool file_exists();
//      std::string get_dir_path();
//      std::string get_full_path();
//      std::string get_ext();
//      std::string get_name();
//      long get_size();
//      std::string get_url();
//      virtual type_id get_type_id() const { return LEV_TFILE_PATH; }
//      virtual const char *get_type_name() const { return "lev.fs.file_path"; }
//      bool is_dir();
//      bool is_dir_readable();
//      bool is_dir_writable();
//      bool is_file_executable();
//      bool is_file_readable();
//      bool is_file_writable();
//      bool mkdir(bool force);
//      bool mkdir0() { return mkdir(false); }
//      bool touch();
//
//    protected:
//      void *_obj;
  };

  class file_system : public base
  {
//    protected:
//      file_system();
    public:
//      virtual ~file_system();
      static bool dir_exists(const std::string &dir_path);
      static bool exists(const std::string &path);
      static bool file_exists(const std::string &file_path);
//      bool find(const std::string &pattern, std::string &file_name);
//      static int find_l(lua_State *L);
//      bool find_next(std::string &file_name);
//      static int find_next_l(lua_State *L);
//      static std::string get_cwd();
//      static std::string get_executable_path();
//      static std::string get_ext(const std::string &path);
//      std::string get_path();
//      static std::string get_resource_dir();
      static long get_size(const std::string &file_path);
//      static std::string get_temp_dir();
//      virtual type_id get_type_id() const { return LEV_TFILE_SYSTEM; }
//      virtual const char *get_type_name() const { return "lev.fs.file_system"; }
      static bool mkdir(const std::string &path, bool force = false);
      static bool mkdir1(const std::string &path) { return file_system::mkdir(path); }
//      static file_system* open(const std::string &path);
//      static int open_l(lua_State *L);
//      static bool remove(const char *path, bool force);
//      static bool remove1(const char *path) { return file_system::remove(path, false); }
//      bool set_path(const std::string &path);
//      static std::string to_file_path(const std::string &url);
//      static std::string to_full_path(const std::string &path);
//      static std::string to_name(const std::string &path_to_file);
//      static std::string to_url(const std::string &filename);
//      static bool touch(const char *path);
//
//    protected:
//      void *_obj;
  };

}

#endif // _FS_HPP

