/////////////////////////////////////////////////////////////////////////////
// Name:        src/fs.cpp
// Purpose:     source for filesystem management classes
// Author:      Akiva Miura <akiva.miura@gmail.com>
// Modified by:
// Created:     14/07/2011
// Copyright:   (C) 2010-2011 Akiva Miura
// Licence:     MIT License
/////////////////////////////////////////////////////////////////////////////

// pre-compiled header
#include "prec.h"

// declarations
#include "lev/fs.hpp"

// dependencies
#include "lev/debug.hpp"
#include "lev/entry.hpp"
#include "lev/util.hpp"

// libraries
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/scoped_array.hpp>
#include <string>


namespace lev
{

  // file class implement
  template <typename T>
    class impl_file : public T
  {
    public:
      typedef boost::shared_ptr<impl_file> ptr;
    protected:
      impl_file() :
        T(),
        ops(NULL)
      { }
    public:
      virtual ~impl_file()
      {
        close();
      }

      virtual bool close()
      {
        if (! ops) { return false; }
        ops->close(ops);
        ops = NULL;
        return true;
      }

      virtual bool eof() const
      {
        return tell() == get_size();
      }

      virtual bool find(const void *chunk, int length)
      {
        if (length < 0) { return false; }
        std::string data;
        data.assign((const char *)chunk, length);
        return find_data(data, 1);
      }

      virtual void *get_ops()
      {
        return ops;
      }

      virtual bool find_data(const std::string &data, int numtry = 1)
      {
        if (numtry == 0) { numtry = get_size(); }
        if (numtry < 0) { return false; }
        std::string buf;
        if (! read_count(buf, data.length())) { return false; }
        for (int i = 0; i < numtry; i++)
        {
//printf("POS: %d\n", (int)tell());
          if (buf == data) { return true; }
          unsigned char byte;
          if (read(&byte, 1, 1) == 0) { return false; }
          buf.replace(0, 1, "");
          buf.append(1, byte);
        }
        return false;
      }

      virtual long get_size() const
      {
        if (! ops) { return -1; }
        long pos = ops->seek(ops, 0, SEEK_CUR);
        long size = ops->seek(ops, 0, SEEK_END);
        ops->seek(ops, pos, SEEK_SET);
        return size;
      }

      static impl_file::ptr open(const std::string &path,
                                 const std::string &mode = "r")
      {
        impl_file::ptr f;
        try {
          f.reset(new impl_file);
          if (! f) { throw -1; }
          f->wptr = f;
          f->ops = SDL_RWFromFile(path.c_str(), mode.c_str());
          if (! f->ops) { throw -2; }
        }
        catch (...) {
          f.reset();
          lev::debug_print("error on file opening");
        }
        return f;
      }

      virtual size_t read(void *buf, size_t size, size_t maxnum)
      {
        if (! ops) { return 0; }
        return ops->read(ops, buf, size, maxnum);
      }

      virtual bool read_all(std::string &content)
      {
        return read_count(content, get_size());
      }

      virtual bool read_count(std::string &content, int count)
      {
        if (count <= 0) { return false; }
        boost::scoped_array<unsigned char> buf(new unsigned char[count]);
        if (! buf) { return false; }
        size_t readed = read(buf.get(), 1, count);
        if (readed <= 0) { return false; }
        content.assign((const char *)buf.get(), readed);
        return true;
      }

      virtual unsigned short read_le16()
      {
        unsigned char buf[2];
        read(buf, 1, 2);
        return buf[0] << 8 | buf[1];
      }

      virtual unsigned long read_le32()
      {
        unsigned char buf[4];
        read(buf, 1, 4);
        return buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3];
      }

      virtual bool read_line(std::string &content)
      {
        if (! ops) { return false; }
        const int buf_size = 4096;
        unsigned char buf[buf_size + 1];
        int i = 0;
        while ( true )
        {
          if (ops->read(ops, &buf[i], 1, 1) == 0)
          {
            if (i == 0) { return false; }
            else { return true; }
          }
          if (buf[i] == '\n')
          {
            buf[i] = '\0';
            content.assign((const char *)buf, 0, i + 1);
            return true;
          }
          if (buf[i] != '\r')
          {
            i++;
            if (i == buf_size) { return true; }
          }
        }
      }

      static int read_l(lua_State *L)
      {
        using namespace luabind;
        const char *format_str = NULL;
        int num = -1;

        try {
          luaL_checktype(L, 1, LUA_TUSERDATA);
          T *f = object_cast<T *>(object(from_stack(L, 1)));
          if (! f) { throw -1; }
          object t = util::get_merged(L, 2, -1);

          if (t["lua.string1"]) { format_str = object_cast<const char *>( t["lua.string1"]); }
          if (t["lua.number1"]) { num = object_cast<int>(t["lua.number1"]); }

          if (! format_str && num < 0)
          {
            std::string line;
            if (! f->read_line(line)) { lua_pushnil(L); }
            else { lua_pushstring(L, line.c_str()); }
            return 1;
          }
          else if (strcmp(format_str, "*l") == 0)
          {
            std::string line;
            if (! f->read_line(line)) { lua_pushnil(L); }
            else { lua_pushstring(L, line.c_str()); }
            return 1;
          }
          else if (strcmp(format_str, "*a") == 0)
          {
            std::string data;
            if (! f->read_all(data)) { lua_pushnil(L); }
            else { lua_pushlstring(L, data.c_str(), data.length()); }
            return 1;
          }
          else
          {
            lua_error(L);
            return 0;
          }
        }
        catch (...) {
          lev::debug_print(lua_tostring(L, -1));
          lev::debug_print("error on file reading");
          lua_pushnil(L);
          return 1;
        }
      }

      virtual bool save(const std::string &path)
      {
        file::ptr output = file::open(path, "wb");
        if (! output) { return false; }
        std::string data;
        if (! read_all(data)) { return false; }
        return output->write(data);
      }

      virtual long seek(long pos)
      {
        if (! ops) { return -1; }
        return ops->seek(ops, pos, SEEK_SET);
      }

      virtual long tell() const
      {
        if (! ops) { return -1; }
        return ops->seek(ops, 0, SEEK_CUR);
      }

      virtual file::ptr to_file()
      {
        return file::ptr(wptr);
      }

      virtual bool write(const std::string &data)
      {
        if (! ops) { return false; }
        return ops->write(ops, data.c_str(), 1, data.length());
      }

      SDL_RWops *ops;
      boost::weak_ptr<impl_file> wptr;
  };

  file::ptr file::open(const std::string &path, const std::string &mode)
  {
    return impl_file<file>::open(path, mode);
  }

  // memery class implementation
  class impl_memfile : public impl_file<memfile>
  {
    public:
      typedef boost::shared_ptr<impl_memfile> ptr;
    protected:
      impl_memfile() :
        impl_file<memfile>(),
        buffer(NULL)
      { }
    public:
      virtual ~impl_memfile() { }

      virtual unsigned char *get_buffer()
      {
        return buffer;
      }

      virtual bool close()
      {
        if (! ops)
        {
          ops->close(ops);
          ops = NULL;
        }
        if (buffer)
        {
          delete [] buffer;
          buffer = NULL;
        }
        return true;
      }

      static memfile::ptr create(long size)
      {
        impl_memfile::ptr f;
        if (size < 0) { return f; }
        try {
          f.reset(new impl_memfile);
          if (! f) { throw -1; }
          f->wptr = f;
          f->buffer = new unsigned char [size];
          if (! f->buffer) { throw -2; }
          f->ops = SDL_RWFromMem(f->buffer, size);
          if (! f->ops) { throw -3; }
        }
        catch (...) {
          f.reset();
          lev::debug_print("error on memory file allocation");
        }
        return f;
      }

      unsigned char *buffer;
  };

  memfile::ptr memfile::create(long size)
  {
    return impl_memfile::create(size);
  }


  temp_name::temp_name() : path_str() { }

  temp_name::~temp_name()
  {
    if (! path_str.empty())
    {
      fs::remove(path_str.c_str(), false);
    }
  }

  boost::shared_ptr<temp_name> temp_name::create(const std::string &prefix,
                                                 const std::string &suffix)
  {
    boost::shared_ptr<temp_name> tmp;
    try {
      tmp.reset(new temp_name);
      if (! tmp) { throw -1; }

      for (int i = 0 ; ; i++)
      {
        std::string format = (boost::format("%s%d%s") % prefix % i % suffix).str();
        boost::filesystem::path new_path = fs::get_temp_dir() + "/" + format;
        new_path = boost::filesystem::absolute(new_path);

        if (! fs::is_directory(new_path.parent_path().generic_string()))
        {
          fs::mkdir(new_path.parent_path().generic_string(), true);
        }

        if (fs::exists(new_path.generic_string())) { continue; }
        if (! fs::touch(new_path.generic_string())) { continue; }
        tmp->path_str = new_path.generic_string();
        break;
      }
    }
    catch (...) {
      tmp.reset();
      lev::debug_print("error on temp name instance creation");
    }
    return tmp;
  }

  int temp_name::create_l(lua_State *L)
  {
    using namespace luabind;
    const char *prefix = "temp", *suffix = "";

    object t = util::get_merged(L, 1, -1);

    if (t["prefix"]) { prefix = object_cast<const char *>(t["prefix"]); }
    else if (t["pre"]) { prefix = object_cast<const char *>(t["pre"]); }
    else if (t["p"]) { prefix = object_cast<const char *>(t["p"]); }
    else if (t["lua.string1"]) { prefix = object_cast<const char *>(t["lua.string1"]); }

    if (t["suffix"]) { suffix = object_cast<const char *>(t["suffix"]); }
    else if (t["suf"]) { suffix = object_cast<const char *>(t["suf"]); }
    else if (t["s"]) { suffix = object_cast<const char *>(t["s"]); }
    else if (t["lua.string2"]) { suffix = object_cast<const char *>(t["lua.string2"]); }

    object o = globals(L)["lev"]["classes"]["temp_name"]["create_c"](prefix, suffix);
    o.push(L);
    return 1;
  }

  // file path class implementation
  class impl_filepath : public filepath
  {
    public:
      typedef boost::shared_ptr<impl_filepath> ptr;
    protected:
      impl_filepath(const std::string &path = "",
                    temp_name::ptr tmp = temp_name::ptr()) :
        filepath(),
        p(path),
        tmp(tmp)
      { }
    public:
      virtual ~impl_filepath() { }

      virtual bool clear()
      {
        p.clear();
        return true;
      }

      static impl_filepath::ptr create(const std::string &path = "")
      {
        impl_filepath::ptr fpath;
        try {
          fpath.reset(new impl_filepath(path));
          if (! fpath) { throw -1; }
        }
        catch (...) {
          fpath.reset();
          lev::debug_print("error on file path instance creation");
        }
        return fpath;
      }

      static impl_filepath::ptr create_temp(const std::string &prefix,
                                            const std::string &suffix)
      {
        impl_filepath::ptr fpath;
        try {
          temp_name::ptr tmp = temp_name::create(prefix, suffix);
          if (! tmp) { throw -1; }
          fpath.reset(new impl_filepath(tmp->get_name(), tmp));
          if (! fpath) { throw -2; }
        }
        catch (...) {
          fpath.reset();
          lev::debug_print("error on temp file path instance creation");
        }
        return fpath;
      }

      static int create_temp_l(lua_State *L)
      {
        using namespace luabind;
        const char *prefix = "temp", *suffix = "";

        int n = lua_gettop(L);
        lua_pushcfunction(L, &util::merge);
        newtable(L).push(L);
        for (int i = 1; i <= n; i++)
        {
          object(from_stack(L, i)).push(L);
        }
        lua_call(L, n + 1, 1);
        object t(from_stack(L, -1));
        lua_remove(L, -1);

        if (t["prefix"]) { prefix = object_cast<const char *>(t["prefix"]); }
        else if (t["pre"]) { prefix = object_cast<const char *>(t["pre"]); }
        else if (t["p"]) { prefix = object_cast<const char *>(t["p"]); }
        else if (t["lua.string1"]) { prefix = object_cast<const char *>(t["lua.string1"]); }

        if (t["suffix"]) { suffix = object_cast<const char *>(t["suffix"]); }
        else if (t["suf"]) { suffix = object_cast<const char *>(t["suf"]); }
        else if (t["s"]) { suffix = object_cast<const char *>(t["s"]); }
        else if (t["lua.string2"]) { suffix = object_cast<const char *>(t["lua.string2"]); }

        object fpath = globals(L)["lev"]["classes"]["filepath"]["create_temp_c"](prefix, suffix);
        fpath.push(L);
        return 1;
      }

      virtual std::string get_extension() const
      {
        return p.extension().generic_string();
      }

      virtual std::string get_filename() const
      {
        return p.filename().generic_string();
      }

      virtual filepath::ptr get_fullpath() const
      {
        return filepath::create(boost::filesystem::absolute(p).generic_string());
      }

      virtual filepath::ptr get_parent() const
      {
        return filepath::create(p.parent_path().generic_string());
      }

      virtual long get_size() const
      {
        try {
          return boost::filesystem::file_size(p);
        }
        catch (...) { return -1; }
      }

      virtual std::string get_string() const
      {
        return p.generic_string();
      }

      virtual bool is_directory() const
      {
        return boost::filesystem::is_directory(p);
      }

      virtual bool is_file() const
      {
        return boost::filesystem::is_regular_file(p);
      }

      virtual bool mkdir(bool force = false)
      {
        try {
          if (force) { return boost::filesystem::create_directories(p); }
          else { return boost::filesystem::create_directory(p); }
        }
        catch (...) {
          return false;
        }
      }

      boost::filesystem::path p;
      temp_name::ptr tmp;
  };

  filepath::ptr filepath::create(const std::string &path)
  {
    return impl_filepath::create(path);
  }

  filepath::ptr filepath::create_temp(const std::string &prefix,
                                      const std::string &suffix)
  {
    return impl_filepath::create_temp(prefix, suffix);
  }


  /*
  std::string filepath::get_url()
  {
    return fs::to_url(get_full_path().c_str());
  }

  bool filepath::is_dir()
  {
    return cast_path(_obj)->IsDir();
  }

  bool filepath::is_dir_readable()
  {
    return cast_path(_obj)->IsDirReadable();
  }

  bool filepath::is_dir_writable()
  {
    return cast_path(_obj)->IsDirWritable();
  }

  bool filepath::is_file_executable()
  {
    return cast_path(_obj)->IsFileExecutable();
  }

  bool filepath::is_file_readable()
  {
    return cast_path(_obj)->IsFileReadable();
  }

  bool filepath::is_file_writable()
  {
    return cast_path(_obj)->IsFileWritable();
  }

  bool filepath::touch()
  {
    return cast_path(_obj)->Touch();
  }

  class myFileSystem : public wxFileSystem
  {
    public:
      myFileSystem() : wxFileSystem(), last_find() { }
      virtual ~myFileSystem() { }

      std::string last_find;
  };
  static myFileSystem* cast_fs(void *obj) { return (myFileSystem *)obj; }

  fs::fs() : _obj(NULL) { }

  fs::~fs()
  {
    if (_obj)
    {
      delete cast_fs(_obj);
      _obj = NULL;
    }
  }

  */

  bool fs::exists(const std::string &path)
  {
    try {
      return boost::filesystem::exists(path);
    }
    catch (...) {
      return false;
    }
  }

  /*
  bool fs::find(const std::string &pattern, std::string &file_name)
  {
    wxString res = cast_fs(_obj)->FindFirst(wxString(pattern.c_str(), wxConvUTF8));
    file_name = (const char *)res.mb_str();
    if (res.empty())
    {
      cast_fs(_obj)->last_find = "";
      return false;
    }
    cast_fs(_obj)->last_find = pattern;
    return true;
  }

  int fs::find_l(lua_State *L)
  {
    using namespace luabind;
    const char *pattern = "*";

    luaL_checktype(L, 1, LUA_TUSERDATA);
    fs *fs = object_cast<fs *>(object(from_stack(L, 1)));
    if (fs == NULL) { return 0; }
    object t = util::get_merged(L, 2, -1);

    if (t["pattern"]) { pattern = object_cast<const char *>(t["pattern"]); }
    else if (t["lua.string1"]) { pattern = object_cast<const char *>(t["lua.string1"]); }

    std::string file_name;
    if (fs->find(pattern, file_name)) { lua_pushstring(L, file_name.c_str()); }
    else { lua_pushnil(L); }
    return 1;
  }

  bool fs::find_next(std::string &file_name)
  {
    if (cast_fs(_obj)->last_find.empty()) { return false; }
    wxString res = cast_fs(_obj)->FindNext();
    file_name = (const char *)res.mb_str();
    if (res.empty())
    {
      cast_fs(_obj)->last_find = "";
      return false;
    }
    return true;
  }

  int fs::find_next_l(lua_State *L)
  {
    using namespace luabind;

    luaL_checktype(L, 1, LUA_TUSERDATA);
    fs *fs = object_cast<fs *>(object(from_stack(L, 1)));
    if (fs == NULL) { return 0; }
//    object t = util::get_merged(L, 2, -1);

    std::string file_name;
    if (fs->find_next(file_name)) { lua_pushstring(L, file_name.c_str()); }
    else { lua_pushnil(L); }
    return 1;
  }

  */

  std::string fs::get_current_directory()
  {
    return boost::filesystem::current_path().generic_string();
  }

  /*
  std::string fs::get_executable_path()
  {
    return (const char *)wxStandardPaths::Get().GetExecutablePath().mb_str();
  }

  */
  /*
  std::string fs::get_resource_dir()
  {
    return (const char *)wxStandardPaths::Get().GetResourcesDir().mb_str();
  }

  */

  long fs::get_size(const std::string &file_path)
  {
    try {
      return boost::filesystem::file_size(file_path);
    }
    catch (...) {
      return -1;
    }
  }


  std::string fs::get_temp_dir()
  {
    try {
      return boost::filesystem::temp_directory_path().generic_string();
    }
    catch (...) {
      return "";
    }
  }

  /*
  std::string fs::get_path()
  {
    return (const char *)cast_fs(_obj)->GetPath().mb_str();
  }

*/

  bool fs::is_directory(const std::string &dir_path)
  {
    try {
      return boost::filesystem::is_directory(dir_path);
    }
    catch (...) {
      return false;
    }
  }

  bool fs::is_file(const std::string &file_path)
  {
    try {
      return boost::filesystem::is_regular_file(file_path);
    }
    catch (...) {
      return false;
    }
  }

  bool fs::mkdir(const std::string &path, bool force)
  {
    try {
      if (force)
      {
        return boost::filesystem::create_directories(path);
      }
      else
      {
        return boost::filesystem::create_directory(path);
      }
    }
    catch (...) { return false; }
  }

/*
  fs* fs::open(const std::string &path)
  {
    fs *fs = NULL;
    myFileSystem *obj = NULL;
    try {
      fs = new fs;
      fs->_obj = obj = new myFileSystem;
      fs->set_path(path);
      return fs;
    }
    catch (...) {
      delete fs;
      return NULL;
    }
  }

  int fs::open_l(lua_State *L)
  {
    using namespace luabind;
    const char *path = ".";

    object t = util::get_merged(L, 1, -1);
    if (t["path"]) { path = object_cast<const char *>(t["path"]); }
    else if (t["lua.string1"]) { path = object_cast<const char *>(t["lua.string1"]); }

    object fs = globals(L)["lev"]["classes"]["fs"]["open_c"](path);
    fs.push(L);
    return 1;
  }

  */
  bool fs::remove(const std::string &path, bool force)
  {
    try {
      if (force) { return boost::filesystem::remove_all(path); }
      else { return boost::filesystem::remove(path); }
    }
    catch (...) {
      return false;
    }
  }

  /*
  bool fs::set_path(const std::string &path)
  {
    std::string full = fs::to_full_path(path);
    cast_fs(_obj)->ChangePathTo(wxString(full.c_str(), wxConvUTF8));
    return true;
  }

  */

  std::string fs::to_extension(const std::string &path)
  {
    return boost::filesystem::path(path).extension().generic_string();
  }

  std::string fs::to_filename(const std::string &path)
  {
    return boost::filesystem::path(path).filename().generic_string();
  }

  std::string fs::to_fullpath(const std::string &path)
  {
    return boost::filesystem::absolute(path).generic_string();
  }

  std::string fs::to_stem(const std::string &path)
  {
    return boost::filesystem::path(path).stem().generic_string();
  }

  /*
  std::string fs::to_url(const std::string &filename)
  {
    wxString file(filename.c_str(), wxConvUTF8);
    return (const char *)wxFileSystem::FileNameToURL(file).mb_str();
  }

  */
  bool fs::touch(const std::string &path)
  {
    FILE *w = fopen(path.c_str(), "a+");
    if (! w) { return false; }
    fclose(w);
    return true;
  }

}

int luaopen_lev_fs(lua_State *L)
{
  using namespace luabind;
  using namespace lev;

  open(L);
  globals(L)["require"]("lev.base");

  module(L, "lev")
  [
    namespace_("classes")
    [
      class_<file, base, base::ptr>("file")
        .def("close", &file::close)
        .property("file", &file::to_file)
        .def("eof", &file::eof)
        .def("find", &file::find_data)
        .property("pos", &file::tell, &file::seek)
        .property("postion", &file::tell, &file::seek)
        .property("size", &file::get_size)
        .def("save", &file::save)
        .def("seek", &file::seek)
        .def("tell", &file::tell)
        .def("write", &file::write)
        .scope
        [
          def("open", &file::open),
          def("open", &file::open1)
        ],
      class_<memfile, file, file::ptr>("memfile")
        .scope
        [
          def("create", &memfile::create)
        ],
      class_<temp_name, base, boost::shared_ptr<base> >("temp_name")
        .def("__tostring", &temp_name::get_name)
        .property("name", &temp_name::get_name)
        .property("path", &temp_name::get_name)
        .scope
        [
          def("create_c", &temp_name::create)
        ],
      class_<filepath, base, boost::shared_ptr<base> >("filepath")
        .def("clear", &filepath::clear)
        .property("abs", &filepath::get_fullpath)
        .property("absolute", &filepath::get_fullpath)
//        .property("dir", &filepath::get_dir_path)
//        .property("dir_path", &filepath::get_dir_path)
        .property("filename", &filepath::get_filename)
        .property("filesize", &filepath::get_size)
        .property("full", &filepath::get_fullpath)
        .property("fullpath", &filepath::get_fullpath)
        .property("ext", &filepath::get_extension)
        .property("extension", &filepath::get_extension)
        .property("is_directory", &filepath::is_directory)
//        .property("is_dir_readable", &filepath::is_dir_readable)
//        .property("is_dir_writable", &filepath::is_dir_writable)
        .property("is_file", &filepath::is_file)
//        .property("is_file_executable", &filepath::is_file_executable)
//        .property("is_file_readable", &filepath::is_file_readable)
//        .property("is_file_writable", &filepath::is_file_writable)
        .def("mkdir", &filepath::mkdir)
        .def("mkdir", &filepath::mkdir0)
        .property("name", &filepath::get_filename)
        .property("parent", &filepath::get_parent)
        .property("parent_path", &filepath::get_parent)
//        .property("path", &filepath::get_dir_path)
        .property("size", &filepath::get_size)
        .property("str", &filepath::get_string)
        .property("string", &filepath::get_string)
//        .property("url", &filepath::get_url)
//        .def("touch", &filepath::touch)
        .scope
        [
          def("create", &filepath::create),
          def("create", &filepath::create0),
          def("create_temp_c", &filepath::create_temp)
        ],
      class_<fs, base>("fs")
//        .property("path", &fs::get_path, &fs::set_path)
        .scope
        [
          def("exists", &fs::exists),
          def("get_current_directory", &fs::get_current_directory),
//          def("get_executable_path", &fs::get_executable_path),
//          def("get_resource_dir", &fs::get_resource_dir),
          def("get_size", &fs::get_size),
          def("get_temp_dir", &fs::get_temp_dir),
          def("is_directory", &fs::is_directory),
          def("is_file", &fs::is_file),
          def("mkdir", &fs::mkdir),
          def("mkdir", &fs::mkdir1),
//          def("open_c", &fs::open, adopt(result)),
          def("remove", &fs::remove),
          def("remove", &fs::remove1),
          def("to_extension", &fs::to_extension),
          def("to_filename", &fs::to_filename),
          def("to_fullpath", &fs::to_fullpath),
          def("to_stem", &fs::to_stem),
          def("touch", &fs::touch)
        ]
    ],
    namespace_("fs") // stub
  ];

  object lev = globals(L)["lev"];
  object classes = lev["classes"];
  object fs = lev["fs"];

  register_to(classes["file"], "read", &impl_file<file>::read_l);
  register_to(classes["memfile"], "read", &impl_file<memfile>::read_l);
  register_to(classes["filepath"], "create_temp", &impl_filepath::create_temp_l);
//  register_to(classes["fs"], "open", &fs::open_l);
//  register_to(classes["fs"], "find", &fs::find_l);
//  register_to(classes["fs"], "find_next", &fs::find_next_l);
  register_to(classes["temp_name"], "create", &temp_name::create_l);

  fs["create_temp"] = classes["filepath"]["create_temp"];
  fs["cwd"] = classes["fs"]["get_current_directory"];
//  fs["exe_path"] = classes["fs"]["get_executable_path"];
  fs["exists"] = classes["fs"]["exists"];
  fs["filepath"] = classes["filepath"]["create"];
  fs["get_current_directory"] = classes["fs"]["get_current_directory"];
  fs["get_cwd"] = classes["fs"]["get_current_directory"];
  fs["get_pwd"] = classes["fs"]["get_current_directory"];
//  fs["get_executable_path"] = classes["fs"]["get_executable_path"];
//  fs["get_resurce_dir"] = classes["fs"]["get_resource_dir"];
  fs["get_size"] = classes["fs"]["get_size"];
  fs["get_temp_dir"] = classes["fs"]["get_temp_dir"];
  fs["is_dir"] = classes["fs"]["is_directory"];
  fs["is_directory"] = classes["fs"]["is_directory"];
  fs["is_file"] = classes["fs"]["is_file"];
  fs["memfile"] = classes["memfile"]["create"];
  fs["mkdir"] = classes["fs"]["mkdir"];
  fs["open"] = classes["file"]["open"];
  fs["path"] = classes["filepath"]["create"];
  fs["pwd"] = classes["fs"]["get_current_directory"];
  fs["remove"] = classes["fs"]["remove"];
//  fs["res_dir"] = classes["fs"]["get_resource_dir"];
//  fs["resorce_dir"] = classes["fs"]["get_resource_dir"];
  fs["temp_name"] = classes["temp_name"]["create"];
  fs["temp_dir"] = classes["fs"]["get_temp_dir"];
  fs["tmp_dir"] = classes["fs"]["get_temp_dir"];
  fs["tmpname"] = classes["temp_name"]["create"];
  fs["to_ext"]       = classes["fs"]["to_extension"];
  fs["to_extension"] = classes["fs"]["to_extension"];
  fs["to_filename"]  = classes["fs"]["to_filename"];
  fs["to_full"]      = classes["fs"]["to_fullpath"];
  fs["to_fullpath"]  = classes["fs"]["to_fullpath"];
  fs["to_name"]      = classes["fs"]["to_filename"];
  fs["to_stem"]      = classes["fs"]["to_stem"];
  fs["touch"]        = classes["fs"]["touch"];

  globals(L)["package"]["loaded"]["lev.fs"] = fs;
  return 0;
}

