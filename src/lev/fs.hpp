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
#include <boost/shared_ptr.hpp>
#include <luabind/luabind.hpp>

extern "C" {
  int luaopen_lev_fs(lua_State *L);
}

namespace lev
{

  class temp_name : public base
  {
    protected:
      temp_name();
    public:
      virtual ~temp_name();
      static boost::shared_ptr<temp_name> create(const std::string &prefix = "temp", const std::string &suffix = "");
      static int create_l(lua_State *L);
      const std::string& get_name() { return path_str; }
      virtual type_id get_type_id() const { return LEV_TTEMP_NAME; }
    protected:
      std::string path_str;
  };

  class file : public base
  {
    protected:
      file();
    public:
      ~file();

      bool close();
      int64_t get_size() const;
      virtual type_id get_type_id() const { return LEV_TFILE; }
      static int lines_l(lua_State *L);

      static boost::shared_ptr<file> open(const std::string &file_path,
                                     const std::string &mode = "r");
      static boost::shared_ptr<file> open1(const std::string &file_path)
      { return open(file_path); }

      bool read_all(std::string &content);
      bool read_line(std::string &content);
      static int read_l(lua_State *L);
    protected:
      void *_obj;
  };

  class path : public base
  {
    protected:
      path();
    public:
      virtual ~path();
      bool clear();
      static boost::shared_ptr<path> create(const std::string &path);
      static int create_l(lua_State *L);
      static boost::shared_ptr<path> create_temp(const std::string &prefix = "temp",
                                    const std::string &suffix = "");
      static int create_temp_l(lua_State *L);

      bool dir_exists(bool transparent = true);
      bool dir_exists0() { return dir_exists(); }

      bool exists(bool transparent = true);
      bool exists0() { return exists(); }

      bool file_exists(bool transparent = true);
      bool file_exists0() { return file_exists(); }

//      std::string get_dir_path();
      std::string get_full_path() const;
//      std::string get_ext();
//      std::string get_name();
      long get_size();
//      std::string get_url();
      virtual type_id get_type_id() const { return LEV_TPATH; }
//      bool is_dir();
//      bool is_dir_readable();
//      bool is_dir_writable();
//      bool is_file_executable();
//      bool is_file_readable();
//      bool is_file_writable();
//      bool mkdir(bool force);
//      bool mkdir0() { return mkdir(false); }
      bool read_all(std::string &data);
      std::string to_str() const;
//      bool touch();
//
    protected:
      void *_obj;

      friend class file_system;
  };

  class file_system : public base
  {
//    protected:
//      file_system();
    public:
//      virtual ~file_system();
      static bool dir_exists(const std::string &dir_path, bool transparent = true);
      static bool dir_exists1(const std::string &dir_path)
      { return dir_exists(dir_path); }

      static bool exists(const std::string &path, bool transparent = true);
      static bool exists1(const std::string &path)
      { return exists(path); }

      static bool file_exists(const std::string &path, bool transparent = true);
      static bool file_exists1(const std::string &path)
      { return file_exists(path); }

//      bool find(const std::string &pattern, std::string &file_name);
//      static int find_l(lua_State *L);
//      bool find_next(std::string &file_name);
//      static int find_next_l(lua_State *L);
      static std::string get_cwd();
      static std::string get_exe_dir();
//      static std::string get_executable_path();
      static std::string get_ext(const std::string &path);
      std::string get_path();
//      static std::string get_resource_dir();
      static long get_size(const std::string &path);
      static std::string get_temp_dir();
      static std::string get_user_dir();
//      virtual type_id get_type_id() const { return LEV_TFILE_SYSTEM; }
      static bool mkdir(const std::string &p, bool force = false);
      static bool mkdir1(const std::string &p) { return mkdir(p); }
//      static file_system* open(const std::string &path);
//      static int open_l(lua_State *L);
      static bool read_all(const std::string &file, std::string &data);
      static bool remove(const std::string &path, bool force);
      static bool remove1(const std::string &path) { return file_system::remove(path, false); }
//      bool set_path(const std::string &path);
//      static std::string to_path(const std::string &url);
//      static std::string to_full_path(const std::string &path);
      static std::string to_name(const std::string &path_to_file);
      static std::string to_stem(const std::string &path);
//      static std::string to_url(const std::string &filename);
      static bool touch(const std::string &path);
//
//    protected:
//      void *_obj;
  };

}

#endif // _FS_HPP

