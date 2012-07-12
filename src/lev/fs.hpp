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

  class file : public base
  {
    public:
      typedef boost::shared_ptr<file> ptr;
    protected:
      file() : base() { }
    public:
      virtual ~file() { }

      virtual bool close() = 0;
      virtual bool eof() const = 0;
      virtual bool find(const void *chunk, int length) = 0;
      virtual bool find_data(const std::string &data, int numtry = 1) = 0;
      virtual void *get_ops() = 0;
      virtual long get_size() const = 0;
      virtual type_id get_type_id() const { return LEV_TFILE; }
      static int lines_l(lua_State *L);
      static file::ptr open(const std::string &path, const std::string &mode = "r");
      static file::ptr open1(const std::string &path) { return open(path); }
      virtual size_t read(void *buf, size_t size, size_t maxnum) = 0;
      virtual bool read_all(std::string &content) = 0;
      virtual bool read_count(std::string &content, int count) = 0;
      virtual unsigned short read_le16() = 0;
      virtual unsigned long read_le32() = 0;
      virtual bool read_line(std::string &content) = 0;
      static int read_l(lua_State *L);
      virtual long seek(long pos) = 0;
      virtual long tell() const = 0;
      virtual bool write(const std::string &data) = 0;
  };

  class memfile : public file
  {
    public:
      typedef boost::shared_ptr<memfile> ptr;
    protected:
      memfile() : file() { }
    public:
      virtual ~memfile() { }

      virtual unsigned char *get_buffer() = 0;
      static memfile::ptr create(long size);
      virtual type_id get_type_id() const { return LEV_TMEMFILE; }
  };

  class temp_name : public base
  {
    public:
      typedef boost::shared_ptr<temp_name> ptr;
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


  class filepath : public base
  {
    public:
      typedef boost::shared_ptr<filepath> ptr;
    protected:
      filepath() : base() { }
    public:
      virtual ~filepath() { }
      virtual bool clear() = 0;
      static filepath::ptr create(const std::string &path = "");
      static filepath::ptr create0() { return create(); }
      static filepath::ptr create_temp(const std::string &prefix = "temp",
                                       const std::string &suffix = "");
      virtual std::string get_filename() const = 0;
      virtual std::string get_fullpath() const = 0;
      virtual std::string get_extension() const = 0;
      virtual long get_size() const = 0;
      virtual std::string get_string() const = 0;
      virtual type_id get_type_id() const { return LEV_TFILEPATH; }
      virtual bool is_directory() const = 0;
//      bool is_dir_readable();
//      bool is_dir_writable();
      virtual bool is_file() const = 0;
//      bool is_file_executable();
//      bool is_file_readable();
//      bool is_file_writable();
//      bool mkdir(bool force);
//      bool mkdir0() { return mkdir(false); }
//      bool touch();
  };

  class fs : public base
  {
    public:
      static bool exists(const std::string &path);
//      bool find(const std::string &pattern, std::string &file_name);
//      static int find_l(lua_State *L);
//      bool find_next(std::string &file_name);
//      static int find_next_l(lua_State *L);
      static std::string get_current_directory();
//      static std::string get_executable_path();
//      static std::string get_resource_dir();
      static long get_size(const std::string &filepath);
      static std::string get_temp_dir();
//      virtual type_id get_type_id() const { return LEV_TFS; }
      static bool is_directory(const std::string &dir_path);
      static bool is_file(const std::string &filepath);
      static bool mkdir(const std::string &path, bool force = false);
      static bool mkdir1(const std::string &path) { return fs::mkdir(path); }
//      static fs* open(const std::string &path);
//      static int open_l(lua_State *L);
      static bool remove(const std::string &path, bool force);
      static bool remove1(const std::string &path) { return fs::remove(path, false); }
//      bool set_path(const std::string &path);
      static std::string to_extension(const std::string &path);
      static std::string to_filename(const std::string &path);
      static std::string to_fullpath(const std::string &path);
      static std::string to_stem(const std::string &path);
      static bool touch(const std::string &path);
  };

}

#endif // _FS_HPP

