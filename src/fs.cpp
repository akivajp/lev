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
#include "lev/util.hpp"
#include "register.hpp"

// libraries
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <string>

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
      class_<file_path, base, boost::shared_ptr<base> >("file_path")
        .def("__tostring", &file_path::get_full_path)
        .def("clear", &file_path::clear)
        .property("absolute", &file_path::get_full_path)
//        .property("dir_exists", &file_path::dir_exists)
//        .property("dir", &file_path::get_dir_path)
//        .property("dir_path", &file_path::get_dir_path)
//        .property("file_exists", &file_path::file_exists)
//        .property("file_name", &file_path::get_name)
        .property("full_path", &file_path::get_full_path)
        .property("full", &file_path::get_full_path)
//        .property("ext", &file_path::get_ext)
//        .property("is_dir", &file_path::is_dir)
//        .property("is_dir_readable", &file_path::is_dir_readable)
//        .property("is_dir_writable", &file_path::is_dir_writable)
//        .property("is_file_executable", &file_path::is_file_executable)
//        .property("is_file_readable", &file_path::is_file_readable)
//        .property("is_file_writable", &file_path::is_file_writable)
//        .def("mkdir", &file_path::mkdir)
//        .def("mkdir", &file_path::mkdir0)
//        .property("name", &file_path::get_name)
//        .property("path", &file_path::get_dir_path)
//        .property("size", &file_path::get_size)
//        .property("url", &file_path::get_url)
//        .def("touch", &file_path::touch)
        .scope
        [
          def("create_c", &file_path::create),
          def("create_temp_c", &file_path::create_temp)
        ],
      class_<file_system, base>("file_system")
//        .property("path", &file_system::get_path, &file_system::set_path)
        .scope
        [
          def("dir_exists", &file_system::dir_exists),
          def("exists", &file_system::exists),
          def("file_exists", &file_system::file_exists),
//          def("get_cwd", &file_system::get_cwd),
//          def("get_executable_path", &file_system::get_executable_path),
          def("get_ext", &file_system::get_ext),
//          def("get_resource_dir", &file_system::get_resource_dir),
          def("get_size", &file_system::get_size),
          def("get_temp_dir", &file_system::get_temp_dir),
          def("mkdir", &file_system::mkdir),
          def("mkdir", &file_system::mkdir1),
//          def("open_c", &file_system::open, adopt(result)),
          def("remove", &file_system::remove),
          def("remove", &file_system::remove1),
//          def("to_file_path", &file_system::to_file_path),
//          def("to_full_path", &file_system::to_full_path),
//          def("to_name", &file_system::to_name),
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
  register_to(classes["file_path"], "create", &file_path::create_l);
  register_to(classes["file_path"], "create_temp", &file_path::create_temp_l);
//  register_to(classes["file_system"], "open", &file_system::open_l);
//  register_to(classes["file_system"], "find", &file_system::find_l);
//  register_to(classes["file_system"], "find_next", &file_system::find_next_l);
  register_to(classes["temp_name"], "create", &temp_name::create_l);
  fs["create_temp"] = classes["file_path"]["create_temp"];
//  fs["cwd"] = classes["file_system"]["get_cwd"];
  fs["dir_exists"] = classes["file_system"]["dir_exists"];
//  fs["exe_path"] = classes["file_system"]["get_executable_path"];
  fs["exists"] = classes["file_system"]["exists"];
  fs["file_path"] = classes["file_path"]["create"];
  fs["file_exists"] = classes["file_system"]["file_exists"];
//  fs["get_cwd"] = classes["file_system"]["get_cwd"];
//  fs["get_executable_path"] = classes["file_system"]["get_executable_path"];
  fs["get_ext"] = classes["file_system"]["get_ext"];
//  fs["get_resurce_dir"] = classes["file_system"]["get_resource_dir"];
  fs["get_size"] = classes["file_system"]["get_size"];
  fs["get_temp_dir"] = classes["file_system"]["get_temp_dir"];
  fs["is_dir"] = classes["file_system"]["dir_exists"];
  fs["is_file"] = classes["file_system"]["file_exists"];
  fs["mkdir"] = classes["file_system"]["mkdir"];
//  fs["open"] = classes["file_system"]["open"];
  fs["path"] = classes["file_path"]["create"];
  fs["remove"] = classes["file_system"]["remove"];
//  fs["res_dir"] = classes["file_system"]["get_resource_dir"];
//  fs["resorce_dir"] = classes["file_system"]["get_resource_dir"];
  fs["temp_name"] = classes["temp_name"]["create"];
  fs["temp_dir"] = classes["file_system"]["get_temp_dir"];
  fs["tmp_dir"] = classes["file_system"]["get_temp_dir"];
  fs["tmpname"] = classes["temp_name"]["create"];
//  fs["to_file_path"] = classes["file_system"]["to_file_path"];
//  fs["to_path"] = classes["file_system"]["to_file_path"];
  fs["to_stem"] = classes["file_system"]["to_stem"];
//  fs["to_url"] = classes["file_system"]["to_url"];
  fs["touch"] = classes["file_system"]["touch"];

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
    boost::shared_ptr<temp_name> tmp;
    try {
      tmp.reset(new temp_name);
      if (! tmp) { throw -1; }

      for (int i = 0 ; ; i++)
      {
        std::string format = (boost::format("%s%d%s") % prefix % i % suffix).str();
        boost::filesystem::path new_path = file_system::get_temp_dir() + "/" + format;
        new_path = boost::filesystem::absolute(new_path);

        if (! file_system::dir_exists(new_path.parent_path().generic_string()))
        {
          file_system::mkdir(new_path.parent_path().generic_string(), true);
        }

        if (file_system::exists(new_path.generic_string())) { continue; }
        if (! file_system::touch(new_path.generic_string())) { continue; }
        tmp->path_str = new_path.generic_string();
        break;
      }
    }
    catch (...) {
      tmp.reset();
      fprintf(stderr, "error on temp name instance creation\n");
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


  class myFilePath
  {
    public:
      myFilePath(const std::string &path,
                 boost::shared_ptr<temp_name> tmp = boost::shared_ptr<temp_name>())
        : p(path), tmp(tmp)
      { }

      virtual ~myFilePath()
      {
      }

      boost::filesystem::path p;
      boost::shared_ptr<temp_name> tmp;
  };
  static myFilePath* cast_path(void *obj) { return (myFilePath *)obj; }

  file_path::file_path() : _obj(NULL) { }

  file_path::~file_path()
  {
    if (_obj)
    {
      delete cast_path(_obj);
      _obj = NULL;
    }
  }

  bool file_path::clear()
  {
    cast_path(_obj)->p.clear();
    return true;
  }

  boost::shared_ptr<file_path> file_path::create(const std::string &path)
  {
    boost::shared_ptr<file_path> fpath;
    try {
      fpath.reset(new file_path);
      if (! fpath) { throw -1; }
      fpath->_obj = new myFilePath(path);
      if (! fpath->_obj) { throw -2; }
    }
    catch (...) {
      fpath.reset();
      fprintf(stderr, "error on file path instance creation\n");
    }
    return fpath;
  }

  int file_path::create_l(lua_State *L)
  {
    using namespace luabind;
    const char *path = "";

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

    if (t["filepath"]) { path = object_cast<const char *>(t["filepath"]); }
    else if (t["file_path"]) { path = object_cast<const char *>(t["file_path"]); }
    else if (t["file"]) { path = object_cast<const char *>(t["file"]); }
    else if (t["path"]) { path = object_cast<const char *>(t["path"]); }
    else if (t["f"]) { path = object_cast<const char *>(t["f"]); }
    else if (t["p"]) { path = object_cast<const char *>(t["p"]); }
    else if (t["lua.string1"]) { path = object_cast<const char *>(t["lua.string1"]); }

    object fpath = globals(L)["lev"]["classes"]["file_path"]["create_c"](path);
    fpath.push(L);
    return 1;
  }

  boost::shared_ptr<file_path> file_path::create_temp(const std::string &prefix,
                                                      const std::string &suffix)
  {
    boost::shared_ptr<file_path> fpath;
    try {
      boost::shared_ptr<temp_name> tmp(temp_name::create(prefix, suffix));
      if (! tmp) { throw -1; }
      fpath.reset(new file_path);
      if (! fpath) { throw -2; }
      fpath->_obj = new myFilePath(tmp->get_name(), tmp);
      if (! fpath->_obj) { throw -3; }
    }
    catch (...) {
      fpath.reset();
      fprintf(stderr, "error on temp file path instance creation\n");
    }
    return fpath;
  }

  int file_path::create_temp_l(lua_State *L)
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

    object fpath = globals(L)["lev"]["classes"]["file_path"]["create_temp_c"](prefix, suffix);
    fpath.push(L);
    return 1;
  }

  /*
  bool file_path::dir_exists()
  {
    return cast_path(_obj)->DirExists();
  }

  bool file_path::file_exists()
  {
    return cast_path(_obj)->FileExists();
  }

  std::string file_path::get_dir_path()
  {
    return (const char *)cast_path(_obj)->GetPath().mb_str();
  }

  */
  std::string file_path::get_full_path()
  {
    return boost::filesystem::absolute(cast_path(_obj)->p).generic_string();
  }

  /*
  std::string file_path::get_ext()
  {
    return (const char *)cast_path(_obj)->GetExt().mb_str();
  }

  std::string file_path::get_name()
  {
    return (const char *)cast_path(_obj)->GetName().mb_str();
  }

  long file_path::get_size()
  {
    wxULongLong size = cast_path(_obj)->GetSize();
    if (size == wxInvalidSize) { return -1; }
    return size.GetValue();
  }

  std::string file_path::get_url()
  {
    return file_system::to_url(get_full_path().c_str());
  }

  bool file_path::is_dir()
  {
    return cast_path(_obj)->IsDir();
  }

  bool file_path::is_dir_readable()
  {
    return cast_path(_obj)->IsDirReadable();
  }

  bool file_path::is_dir_writable()
  {
    return cast_path(_obj)->IsDirWritable();
  }

  bool file_path::is_file_executable()
  {
    return cast_path(_obj)->IsFileExecutable();
  }

  bool file_path::is_file_readable()
  {
    return cast_path(_obj)->IsFileReadable();
  }

  bool file_path::is_file_writable()
  {
    return cast_path(_obj)->IsFileWritable();
  }

  bool file_path::mkdir(bool force)
  {
    int flags = 0;
    if (force) { flags = wxPATH_MKDIR_FULL; }
    return cast_path(_obj)->Mkdir(0755, flags);
  }

  bool file_path::touch()
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

  bool file_system::dir_exists(const std::string &dir_path)
  {
    try {
      return boost::filesystem::is_directory(dir_path);
    }
    catch (...) {
      return false;
    }
  }

  bool file_system::exists(const std::string &path)
  {
    try {
      return boost::filesystem::exists(path);
    }
    catch (...) {
      return false;
    }
  }

  bool file_system::file_exists(const std::string &file_path)
  {
    try {
      return boost::filesystem::is_regular_file(file_path);
    }
    catch (...) {
      return false;
    }
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

  std::string file_system::get_cwd()
  {
    return (const char *)wxGetCwd().mb_str();
  }

  std::string file_system::get_executable_path()
  {
    return (const char *)wxStandardPaths::Get().GetExecutablePath().mb_str();
  }

  */
  std::string file_system::get_ext(const std::string &path)
  {
    return boost::filesystem::path(path).extension().generic_string();
  }

  /*
  std::string file_system::get_resource_dir()
  {
    return (const char *)wxStandardPaths::Get().GetResourcesDir().mb_str();
  }

  */

  long file_system::get_size(const std::string &file_path)
  {
    try {
      return boost::filesystem::file_size(file_path);
    }
    catch (...) {
      return -1;
    }
  }


  std::string file_system::get_temp_dir()
  {
    try {
      return boost::filesystem::temp_directory_path().generic_string();
    }
    catch (...) {
      return "";
    }
  }

  /*
  std::string file_system::get_path()
  {
    return (const char *)cast_fs(_obj)->GetPath().mb_str();
  }

*/
  bool file_system::mkdir(const std::string &path, bool force)
  {
    try {
      if (force)
      {
        boost::filesystem::create_directories(path);
        return true;
      }
      else { return boost::filesystem::create_directory(path); }
    }
    catch (...) { return false; }
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
  bool file_system::remove(const std::string &path, bool force)
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
  bool file_system::set_path(const std::string &path)
  {
    std::string full = file_system::to_full_path(path);
    cast_fs(_obj)->ChangePathTo(wxString(full.c_str(), wxConvUTF8));
    return true;
  }

  std::string file_system::to_file_path(const std::string &url)
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

  std::string file_system::to_name(const std::string &path_to_file)
  {
    wxString p(path_to_file.c_str(), wxConvUTF8);
    wxString u = wxFileSystem::FileNameToURL(p);
    return (const char *)wxFileSystem::URLToFileName(u).GetName().mb_str();
  }

  */
  std::string file_system::to_stem(const std::string &file_path)
  {
    try {
      return boost::filesystem::path(file_path).stem().generic_string();
    }
    catch (...) {
      return "";
    }
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


