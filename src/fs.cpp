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
#include "lev/package.hpp"
#include "lev/system.hpp"
#include "lev/util.hpp"

// libraries
#include <allegro5/allegro.h>
#include <allegro5/allegro_physfs.h>
#include <boost/format.hpp>
#include <physfs.h>
#include <string>

#include <sys/stat.h>
#if defined(__WIN32__)
  #include <io.h>
#endif // defined(__WIN32__)

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
      class_<temp_name, base, boost::shared_ptr<base> >("temp_name")
        .def("__tostring", &temp_name::get_name)
        .property("name", &temp_name::get_name)
        .property("path", &temp_name::get_name)
        .scope
        [
          def("create_c", &temp_name::create)
        ],
      class_<file, base, boost::shared_ptr<base> >("file")
        .def("close", &file::close)
        .def("get_size", &file::get_size)
        .property("size", &file::get_size)
        .scope
        [
          def("open", &file::open),
          def("open", &file::open1)
        ],
      class_<path, base, boost::shared_ptr<base> >("path")
        .def("__tostring", &path::to_str)
        .def("clear", &path::clear)
        .property("absolute", &path::get_full_path)
        .property("dir_exists", &path::dir_exists)
        .property("dir_exists", &path::dir_exists0)
//        .property("dir", &path::get_dir_path)
//        .property("dir_path", &path::get_dir_path)
        .property("exists", &path::exists)
        .property("exists", &path::exists0)
        .property("file_exists", &path::file_exists)
        .property("file_exists", &path::file_exists0)
//        .property("file_name", &path::get_name)
        .property("full_path", &path::get_full_path)
        .property("full", &path::get_full_path)
//        .property("ext", &path::get_ext)
//        .property("is_dir", &path::is_dir)
//        .property("is_dir_readable", &path::is_dir_readable)
//        .property("is_dir_writable", &path::is_dir_writable)
//        .property("is_file_executable", &path::is_file_executable)
//        .property("is_file_readable", &path::is_file_readable)
//        .property("is_file_writable", &path::is_file_writable)
//        .def("mkdir", &path::mkdir)
//        .def("mkdir", &path::mkdir0)
//        .property("name", &path::get_name)
        .property("read", &path::read_all)
        .property("read_all", &path::read_all)
        .property("read_at_once", &path::read_all)
//        .property("path", &path::get_dir_path)
        .property("size", &path::get_size)
//        .property("url", &path::get_url)
//        .def("touch", &path::touch)
        .scope
        [
          def("create_c", &path::create),
          def("create_temp_c", &path::create_temp)
        ],
      class_<file_system, base>("file_system")
//        .property("path", &file_system::get_path, &file_system::set_path)
        .scope
        [
          def("dir_exists", &file_system::dir_exists),
          def("dir_exists", &file_system::dir_exists1),
          def("exists", &file_system::exists),
          def("exists", &file_system::exists1),
          def("file_exists", &file_system::file_exists),
          def("file_exists", &file_system::file_exists1),
          def("get_cwd", &file_system::get_cwd),
          def("get_exe_dir", &file_system::get_exe_dir),
//          def("get_executable_path", &file_system::get_executable_path),
          def("get_ext", &file_system::get_ext),
//          def("get_resource_dir", &file_system::get_resource_dir),
          def("get_size", &file_system::get_size),
          def("get_temp_dir", &file_system::get_temp_dir),
          def("get_user_dir", &file_system::get_user_dir),
          def("mkdir", &file_system::mkdir),
          def("mkdir", &file_system::mkdir1),
//          def("open_c", &file_system::open, adopt(result)),
          def("read", &file_system::read_all),
          def("read_all", &file_system::read_all),
          def("read_at_once", &file_system::read_all),
          def("remove", &file_system::remove),
          def("remove", &file_system::remove1),
//          def("to_path", &file_system::to_path),
//          def("to_full_path", &file_system::to_full_path),
          def("to_name", &file_system::to_name),
          def("to_stem", &file_system::to_stem),
//          def("to_url", &file_system::to_url),
          def("touch", &file_system::touch)
        ]
    ],
    namespace_("fs") // stub
  ];

  object lev = globals(L)["lev"];
  object classes = lev["classes"];
  object fs = lev["fs"];

  register_to(classes["file"], "read", &file::read_l);
  register_to(classes["path"], "create", &path::create_l);
  register_to(classes["path"], "create_temp", &path::create_temp_l);
//  register_to(classes["file_system"], "open", &file_system::open_l);
//  register_to(classes["file_system"], "find", &file_system::find_l);
//  register_to(classes["file_system"], "find_next", &file_system::find_next_l);
  register_to(classes["temp_name"], "create", &temp_name::create_l);
  fs["base_dir"] = classes["file_system"]["get_exe_dir"];
  fs["create_temp"] = classes["path"]["create_temp"];
  fs["cwd"] = classes["file_system"]["get_cwd"];
  fs["dir_exists"] = classes["file_system"]["dir_exists"];
//  fs["exe_path"] = classes["file_system"]["get_executable_path"];
  fs["exe_dir"] = classes["file_system"]["get_exe_dir"];
  fs["exists"] = classes["file_system"]["exists"];
  fs["path"] = classes["path"]["create"];
  fs["file_exists"] = classes["file_system"]["file_exists"];
  fs["get_cwd"] = classes["file_system"]["get_cwd"];
//  fs["get_executable_path"] = classes["file_system"]["get_executable_path"];
  fs["get_ext"] = classes["file_system"]["get_ext"];
//  fs["get_resurce_dir"] = classes["file_system"]["get_resource_dir"];
  fs["get_size"] = classes["file_system"]["get_size"];
  fs["get_temp_dir"] = classes["file_system"]["get_temp_dir"];
  fs["get_user_dir"] = classes["file_system"]["get_user_dir"];
  fs["is_dir"] = classes["file_system"]["dir_exists"];
  fs["is_file"] = classes["file_system"]["file_exists"];
  fs["mkdir"] = classes["file_system"]["mkdir"];
  fs["open"] = classes["file"]["open"];
  fs["path"] = classes["path"]["create"];
  fs["pwd"] = classes["file_system"]["get_cwd"];
  fs["remove"] = classes["file_system"]["remove"];
//  fs["res_dir"] = classes["file_system"]["get_resource_dir"];
//  fs["resorce_dir"] = classes["file_system"]["get_resource_dir"];
  fs["temp_name"] = classes["temp_name"]["create"];
  fs["temp_dir"] = classes["file_system"]["get_temp_dir"];
  fs["tmp_dir"] = classes["file_system"]["get_temp_dir"];
  fs["tmpname"] = classes["temp_name"]["create"];
  fs["to_file_name"] = classes["file_system"]["to_name"];
  fs["to_filename"] = classes["file_system"]["to_name"];
  fs["to_name"] = classes["file_system"]["to_name"];
//  fs["to_path"] = classes["file_system"]["to_path"];
//  fs["to_path"] = classes["file_system"]["to_path"];
  fs["to_stem"] = classes["file_system"]["to_stem"];
//  fs["to_url"] = classes["file_system"]["to_url"];
  fs["touch"] = classes["file_system"]["touch"];
  fs["user_dir"] = classes["file_system"]["get_user_dir"];

  globals(L)["package"]["loaded"]["lev.fs"] = fs;
  return 0;
}

namespace lev
{

  temp_name::temp_name() : path_str() { }

  temp_name::~temp_name()
  {
    if (! path_str.empty())
    {
      file_system::remove(path_str.c_str(), false);
    }
  }

  boost::shared_ptr<temp_name> temp_name::create(const std::string &prefix,
                                                 const std::string &suffix)
  {
//    boost::shared_ptr<temp_name> tmp;
//    try {
//      tmp.reset(new temp_name);
//      if (! tmp) { throw -1; }
//
//      for (int i = 0 ; ; i++)
//      {
//        std::string format = (boost::format("%s%d%s") % prefix % i % suffix).str();
//        boost::filesystem::path new_path = file_system::get_temp_dir() + "/" + format;
//        new_path = boost::filesystem::absolute(new_path);
//
//        if (! file_system::dir_exists(new_path.parent_path().generic_string()))
//        {
//          file_system::mkdir(new_path.parent_path().generic_string(), true);
//        }
//
//        if (file_system::exists(new_path.generic_string())) { continue; }
//        if (! file_system::touch(new_path.generic_string())) { continue; }
//        tmp->path_str = new_path.generic_string();
//        break;
//      }
//    }
//    catch (...) {
//      tmp.reset();
//      lev::debug_print("error on temp name instance creation");
//    }
//    return tmp;
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

  static ALLEGRO_FILE *cast_file(void *obj)
  {
    return (ALLEGRO_FILE *)obj;
  }

  file::file() : _obj(NULL) { }

  file::~file()
  {
    if (_obj)
    {
      al_fclose(cast_file(_obj));
      _obj = NULL;
    }
  }

  bool file::close()
  {
    if (_obj)
    {
      al_fclose(cast_file(_obj));
      _obj = NULL;
      return true;
    }
    return false;
  }

  int64_t file::get_size() const
  {
    if (! _obj) { return -1; }
    return al_fsize(cast_file(_obj));
  }

  boost::shared_ptr<file> file::open(const std::string &file_path,
                                     const std::string &mode)
  {
    boost::shared_ptr<file> f;
    lua_State *L = system::get_interpreter();
    try {
      f.reset(new file);
      if (! f) { throw -1; }
      if (! L)
      {
        f->_obj = al_fopen(file_path.c_str(), mode.c_str());
        if (! f->_obj) { throw -2; }
      }
      boost::shared_ptr<path> p = package::resolve(L, file_path);
      if (! p) { throw -3; }
      f->_obj = al_fopen(p->to_str().c_str(), mode.c_str());
      if (! f->_obj) { throw -4; }
    }
    catch (...) {
      f.reset();
      lev::debug_print("error on file opening");
    }
    return f;
  }

  bool file::read_all(std::string &content)
  {
    if (get_size() < 0) { return false; }
    char buf[get_size() + 1];
    size_t count = al_fread(cast_file(_obj), buf, get_size() + 1);
    if (count == 0) { return false; }
//printf("COUNT: %d\n", (int)count);
    content.assign(buf, count);
    return true;
  }
  bool file::read_line(std::string &content)
  {
    if (get_size() < 0) { return false; }
    char buf[get_size() + 1];
    if (! al_fgets(cast_file(_obj), buf, get_size() + 1)) { return false; }
    content = buf;

    // chomping
    if (content[content.length() - 1] == '\n')
    {
      content.erase(content.length() - 1, 1);
    }
    if (content[content.length() - 1] == '\r')
    {
      content.erase(content.length() - 1, 1);
    }
    return true;
  }

  int file::read_l(lua_State *L)
  {
    using namespace luabind;
    const char *format_str = NULL;
    int num = -1;

    try {
      luaL_checktype(L, 1, LUA_TUSERDATA);
      file *f = object_cast<file *>(object(from_stack(L, 1)));
      if (! f) { throw -1; }
      object t = util::get_merged(L, 2, -1);

      if (t["lua.string1"]) { format_str = object_cast<const char *>(t["lua.string1"]); }

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

//  class myFilePath
//  {
//    public:
//      myFilePath(const std::string &path,
//                 boost::shared_ptr<temp_name> tmp = boost::shared_ptr<temp_name>())
//        : p(path), tmp(tmp)
//      { }
//
//      virtual ~myFilePath()
//      {
//      }
//
//      boost::filesystem::path p;
//      boost::shared_ptr<temp_name> tmp;
//  };
//  static myFilePath* cast_path(void *obj) { return (myFilePath *)obj; }
  static ALLEGRO_FS_ENTRY *cast_path(void *obj)
  {
    return (ALLEGRO_FS_ENTRY *)obj;
  }
  struct fs_interface_locker
  {
    fs_interface_locker(bool transparent) : transparent(transparent)
    {
      if (PHYSFS_isInit() && !transparent)
      {
//printf("STD\n");
        al_set_standard_file_interface();
        al_set_standard_fs_interface();
      }
    }

    ~fs_interface_locker()
    {
      if (PHYSFS_isInit() && !transparent)
      {
//printf("PHYSFS\n");
        al_set_physfs_file_interface();
      }
    }

    bool transparent;
  };

  path::path() : _obj(NULL) { }

  path::~path()
  {
    if (_obj)
    {
      al_destroy_fs_entry(cast_path(_obj));
      _obj = NULL;
    }
  }

  bool path::clear()
  {
    ALLEGRO_FS_ENTRY *entry = al_create_fs_entry(".");
    if (! entry) { return false; }
    al_destroy_fs_entry(cast_path(_obj));
    _obj = entry;
    return true;
  }

  boost::shared_ptr<path> path::create(const std::string &path_string)
  {
    boost::shared_ptr<path> p;
    try {
      p.reset(new path);
      if (! p) { throw -1; }
      p->_obj = al_create_fs_entry(path_string.c_str());
      if (! p->_obj) { throw -2; }
    }
    catch (...) {
      p.reset();
      lev::debug_print("error on file path instance creation");
    }
    return p;
  }

  int path::create_l(lua_State *L)
  {
    using namespace luabind;
    const char *path = ".";

    object t = util::get_merged(L, 1, -1);

    if (t["filepath"]) { path = object_cast<const char *>(t["filepath"]); }
    else if (t["file_path"]) { path = object_cast<const char *>(t["file_path"]); }
    else if (t["file"]) { path = object_cast<const char *>(t["file"]); }
    else if (t["path"]) { path = object_cast<const char *>(t["path"]); }
    else if (t["f"]) { path = object_cast<const char *>(t["f"]); }
    else if (t["p"]) { path = object_cast<const char *>(t["p"]); }
    else if (t["lua.string1"]) { path = object_cast<const char *>(t["lua.string1"]); }

    object fpath = globals(L)["lev"]["classes"]["path"]["create_c"](path);
    fpath.push(L);
    return 1;
  }

  boost::shared_ptr<path> path::create_temp(const std::string &prefix,
                                                      const std::string &suffix)
  {
//    boost::shared_ptr<path> fpath;
//    try {
//      boost::shared_ptr<temp_name> tmp(temp_name::create(prefix, suffix));
//      if (! tmp) { throw -1; }
//      fpath.reset(new path);
//      if (! fpath) { throw -2; }
//      fpath->_obj = new myFilePath(tmp->get_name(), tmp);
//      if (! fpath->_obj) { throw -3; }
//    }
//    catch (...) {
//      fpath.reset();
//      lev::debug_print("error on temp file path instance creation");
//    }
//    return fpath;
  }

  int path::create_temp_l(lua_State *L)
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

    object fpath = globals(L)["lev"]["classes"]["path"]["create_temp_c"](prefix, suffix);
    fpath.push(L);
    return 1;
  }

  bool path::dir_exists(bool transparent)
  {
    return file_system::dir_exists(to_str(), transparent);
  }

  bool path::exists(bool transparent)
  {
    return file_system::exists(to_str(), transparent);
  }

  bool path::file_exists(bool transparent)
  {
    return file_system::file_exists(to_str(), transparent);
  }
/*
  std::string path::get_dir_path()
  {
    return (const char *)cast_path(_obj)->GetPath().mb_str();
  }

  */
  std::string path::get_full_path() const
  {
    return al_get_fs_entry_name(cast_path(_obj));
  }

//  std::string path::get_ext()
//  {
//    return (const char *)cast_path(_obj)->GetExt().mb_str();
//  }
//
//  std::string path::get_name()
//  {
//    return (const char *)cast_path(_obj)->GetName().mb_str();
//  }

  long path::get_size()
  {
    return al_get_fs_entry_size(cast_path(_obj));
  }

  /*
  std::string path::get_url()
  {
    return file_system::to_url(get_full_path().c_str());
  }

  bool path::is_dir()
  {
    return cast_path(_obj)->IsDir();
  }

  bool path::is_dir_readable()
  {
    return cast_path(_obj)->IsDirReadable();
  }

  bool path::is_dir_writable()
  {
    return cast_path(_obj)->IsDirWritable();
  }

  bool path::is_file_executable()
  {
    return cast_path(_obj)->IsFileExecutable();
  }

  bool path::is_file_readable()
  {
    return cast_path(_obj)->IsFileReadable();
  }

  bool path::is_file_writable()
  {
    return cast_path(_obj)->IsFileWritable();
  }

  bool path::mkdir(bool force)
  {
    int flags = 0;
    if (force) { flags = wxPATH_MKDIR_FULL; }
    return cast_path(_obj)->Mkdir(0755, flags);
  }

  */

  bool path::read_all(std::string &data)
  {
    boost::shared_ptr<file> f = file::open(to_str(), "rb");
    if (! f) { return false; }
    return f->read_all(data);
  }

  std::string path::to_str() const
  {
    return al_get_fs_entry_name(cast_path(_obj));
  }
  /*

  bool path::touch()
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

  file_system::file_system() : _obj(NULL) { }

  file_system::~file_system()
  {
    if (_obj)
    {
      delete cast_fs(_obj);
      _obj = NULL;
    }
  }

  */

  bool file_system::dir_exists(const std::string &dir_path, bool transparent)
  {
    if (transparent && PHYSFS_isInit()){ return PHYSFS_isDirectory(dir_path.c_str()); }
    struct stat buf;
    if (stat(dir_path.c_str(), &buf) != 0) { return false; }
    return S_ISDIR(buf.st_mode);
  }

  bool file_system::exists(const std::string &path, bool transparent)
  {
    if (transparent && PHYSFS_isInit()) { return PHYSFS_exists(path.c_str()); }
    struct stat buf;
    if (stat(path.c_str(), &buf) != 0) { return false; }
    return S_ISDIR(buf.st_mode) || S_ISREG(buf.st_mode);
  }

  bool file_system::file_exists(const std::string &p, bool transparent)
  {
    if (transparent && PHYSFS_isInit())
    {
      return PHYSFS_exists(p.c_str()) && !PHYSFS_isDirectory(p.c_str());
    }
    struct stat buf;
    if (stat(p.c_str(), &buf) != 0) { return false; }
    return S_ISREG(buf.st_mode);
  }

  /*
  bool file_system::find(const std::string &pattern, std::string &file_name)
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

  int file_system::find_l(lua_State *L)
  {
    using namespace luabind;
    const char *pattern = "*";

    luaL_checktype(L, 1, LUA_TUSERDATA);
    file_system *fs = object_cast<file_system *>(object(from_stack(L, 1)));
    if (fs == NULL) { return 0; }
    object t = util::get_merged(L, 2, -1);

    if (t["pattern"]) { pattern = object_cast<const char *>(t["pattern"]); }
    else if (t["lua.string1"]) { pattern = object_cast<const char *>(t["lua.string1"]); }

    std::string file_name;
    if (fs->find(pattern, file_name)) { lua_pushstring(L, file_name.c_str()); }
    else { lua_pushnil(L); }
    return 1;
  }

  bool file_system::find_next(std::string &file_name)
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

  int file_system::find_next_l(lua_State *L)
  {
    using namespace luabind;

    luaL_checktype(L, 1, LUA_TUSERDATA);
    file_system *fs = object_cast<file_system *>(object(from_stack(L, 1)));
    if (fs == NULL) { return 0; }
//    object t = util::get_merged(L, 2, -1);

    std::string file_name;
    if (fs->find_next(file_name)) { lua_pushstring(L, file_name.c_str()); }
    else { lua_pushnil(L); }
    return 1;
  }

  */
  std::string file_system::get_cwd()
  {
    fs_interface_locker lock(false);
    char *tmp = al_get_current_directory();
    std::string cwd = tmp;
    al_free(tmp);
    return cwd;
  }

  std::string file_system::get_exe_dir()
  {
    return PHYSFS_getBaseDir();
  }

  /*
  std::string file_system::get_executable_path()
  {
    return (const char *)wxStandardPaths::Get().GetExecutablePath().mb_str();
  }

  */
  std::string file_system::get_ext(const std::string &path)
  {
//    return boost::filesystem::path(path).extension().generic_string();
  }

  /*
  std::string file_system::get_resource_dir()
  {
    return (const char *)wxStandardPaths::Get().GetResourcesDir().mb_str();
  }

  */

  long file_system::get_size(const std::string &str_path)
  {
    boost::shared_ptr<path> p = path::create(str_path);
    if (! p) { return -1; }
    return p->get_size();
  }


  std::string file_system::get_temp_dir()
  {
//    try {
//      return boost::filesystem::temp_directory_path().generic_string();
//    }
//    catch (...) {
//      return "";
//    }
  }

  std::string file_system::get_user_dir()
  {
    return PHYSFS_getUserDir();
  }

//  std::string file_system::get_path()
//  {
//    return (const char *)cast_fs(_obj)->GetPath().mb_str();
//  }

  bool file_system::mkdir(const std::string &path, bool force)
  {
    return al_make_directory(path.c_str());
  }

/*
  file_system* file_system::open(const std::string &path)
  {
    file_system *fs = NULL;
    myFileSystem *obj = NULL;
    try {
      fs = new file_system;
      fs->_obj = obj = new myFileSystem;
      fs->set_path(path);
      return fs;
    }
    catch (...) {
      delete fs;
      return NULL;
    }
  }

  int file_system::open_l(lua_State *L)
  {
    using namespace luabind;
    const char *path = ".";

    object t = util::get_merged(L, 1, -1);
    if (t["path"]) { path = object_cast<const char *>(t["path"]); }
    else if (t["lua.string1"]) { path = object_cast<const char *>(t["lua.string1"]); }

    object fs = globals(L)["lev"]["classes"]["file_system"]["open_c"](path);
    fs.push(L);
    return 1;
  }

  */

  bool file_system::read_all(const std::string &file_path, std::string &data)
  {
    boost::shared_ptr<file> f = file::open(file_path, "rb");
    if (! f) { return false; }
    return f->read_all(data);
  }

  bool file_system::remove(const std::string &path, bool force)
  {
    return al_remove_filename(path.c_str());
  }

  /*
  bool file_system::set_path(const std::string &path)
  {
    std::string full = file_system::to_full_path(path);
    cast_fs(_obj)->ChangePathTo(wxString(full.c_str(), wxConvUTF8));
    return true;
  }

  std::string file_system::to_path(const std::string &url)
  {
    wxString u(url.c_str(), wxConvUTF8);
    return (const char *)wxFileSystem::URLToFileName(u).GetFullPath().mb_str();
  }

  std::string file_system::to_full_path(const std::string &path)
  {
    wxString p(path.c_str(), wxConvUTF8);
    wxString u = wxFileSystem::FileNameToURL(p);
    return (const char *)wxFileSystem::URLToFileName(u).GetFullPath().mb_str();
  }

  */
  std::string file_system::to_name(const std::string &p)
  {
    std::string::size_type pos = p.find_last_of("/");
    if (pos == std::string::npos) { return p; }
    else { return p.substr(pos + 1); }
  }

  std::string file_system::to_stem(const std::string &p)
  {
    std::string filename = to_name(p);
    std::string::size_type pos = filename.find_first_of(".");
    if (pos == std::string::npos) { return filename; }
    else { return filename.substr(0, pos); }
  }

  /*
  std::string file_system::to_url(const std::string &filename)
  {
    wxString file(filename.c_str(), wxConvUTF8);
    return (const char *)wxFileSystem::FileNameToURL(file).mb_str();
  }

  */
  bool file_system::touch(const std::string &path)
  {
    FILE *w = fopen(path.c_str(), "a+");
    if (! w) { return false; }
    fclose(w);
    return true;
  }

}


