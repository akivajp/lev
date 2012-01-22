/////////////////////////////////////////////////////////////////////////////
// Name:        src/archive.cpp
// Purpose:     source for archive file handling classes
// Author:      Akiva Miura <akiva.miura@gmail.com>
// Created:     09/06/2011
// Copyright:   (C) 2010-2012 Akiva Miura
// Licence:     MIT License
/////////////////////////////////////////////////////////////////////////////

// pre-compiled header
#include "prec.h"

// declarations
#include "lev/archive.hpp"

// dependencies
#include "lev/fs.hpp"
#include "lev/util.hpp"
#include "register.hpp"

// external libraries
extern "C" {
  #include <unzip.h>
  #include <zip.h>
}
#include <luabind/luabind.hpp>

int luaopen_lev_archive(lua_State *L)
{
  using namespace lev;
  using namespace luabind;

  open(L);
  globals(L)["require"]("lev.base");

  module(L, "lev")
  [
    namespace_("archive"),
    namespace_("classes")
    [
      class_<lev::archive, base>("archive")
//        .def("add_data", &lev::archive::add_data)
//        .def("add_file", &lev::archive::add_file)
//        .def("add_file_to", &lev::archive::add_file_to)
//        .property("compression", &lev::archive::get_compression_name)
//        .property("compression_name", &lev::archive::get_compression_name)
        .def("flush", &lev::archive::flush)
//        .property("format", &lev::archive::get_format_name)
//        .property("format_name", &lev::archive::get_format_name)
//        .def("entry_exists", &lev::archive::entry_exists)
//        .def("extract", &lev::archive::extract)
//        .def("extract_to", &lev::archive::extract_to)
//        .def("set_compression_gzip", &lev::archive::set_compression_gzip)
//        .def("set_compression_none", &lev::archive::set_compression_none)
//        .def("set_compression_xz", &lev::archive::set_compression_xz)
//        .def("set_format_tar", &lev::archive::set_format_tar)
//        .def("set_format_zip", &lev::archive::set_format_zip)
        .scope
        [
//          def("entry_exists_direct", &lev::archive::entry_exists_direct),
//          def("extract_direct", &lev::archive::extract_direct),
//          def("extract_direct_to", &lev::archive::extract_direct_to),
//          def("is_archive", &lev::archive::is_archive),
          def("open", &lev::archive::open, adopt(result))
        ]
    ]
  ];
  object lev = globals(L)["lev"];
  object classes = lev["classes"];
  object arch = lev["archive"];

  register_to(classes["archive"], "add_data", &lev::archive::add_data_l);
//  register_to(classes["archive"], "find", &lev::archive::find_l);
//  register_to(classes["archive"], "find_direct", &lev::archive::find_direct_l);
//  register_to(classes["archive"], "find_next", &lev::archive::find_next_l);
//  register_to(classes["archive"], "read", &lev::archive::read_l);
//  register_to(classes["archive"], "read_direct", &lev::archive::read_direct_l);
//
//  arch["entry_exists"] = classes["archive"]["entry_exists_direct"];
//  arch["extract"] = classes["archive"]["extract_direct"];
//  arch["extract_to"] = classes["archive"]["extract_direct_to"];
//  arch["find"] = classes["archive"]["find_direct"];
//  arch["is_archive"] = classes["archive"]["is_archive"];
  arch["open"] = classes["archive"]["open"];
//  arch["read"] = classes["archive"]["read_direct"];

  globals(L)["package"]["loaded"]["lev.archive"] = true;
}

namespace lev
{

  class myArchive
  {
    public:

      myArchive() :
        compress_level(Z_DEFAULT_COMPRESSION),
        r(NULL), w(NULL)
      { }

      ~myArchive()
      {
        Clean();
      }

      bool Clean()
      {
        if (r)
        {
          unzClose(r);
          r = NULL;
        }
        if (w)
        {
          zipClose(w, NULL);
          w = NULL;
        }
        return true;
      }

      static myArchive* Create(const std::string &archive_path)
      {
        if (archive_path == "") { return NULL; }

        myArchive *arc = NULL;
        try {
          zipFile z = NULL;
          if (file_system::file_exists(archive_path))
          {
            z = zipOpen64(archive_path.c_str(), APPEND_STATUS_ADDINZIP);
//if (z) { printf("APPEND MODE!\n"); }
          }
          else
          {
            z = zipOpen64(archive_path.c_str(), APPEND_STATUS_CREATE);
//if (z) { printf("CREATE MODE!\n"); }
          }
          if (! z) { throw -1; }

          arc = new myArchive;
          arc->archive_path = archive_path;
          arc->w = z;
          return arc;
        }
        catch (...) {
//printf("ERROR!\n");
          delete arc;
          return NULL;
        }
      }

//      archive_entry* Find(const std::string &pattern)
//      {
//        archive_entry *entry;
//
//        while (archive_read_next_header(r, &entry) == ARCHIVE_OK)
//        {
//          wxString path(archive_entry_pathname(entry), wxConvUTF8);
//          if (path.Matches(wxString(pattern.c_str(), wxConvUTF8)))
//          {
//            return entry;
//          }
//          else
//          {
//            continue;
//          }
//        }
//        return NULL;
//      }
//
//      bool Extract(const std::string &entry_name)
//      {
//        if (!StartReading()) { return false; }
//
//        archive_entry *entry = Find(entry_name);
//        if (entry == NULL) { return false; }
//
//        if (archive_read_extract(r, entry, 0) == ARCHIVE_OK) { return true; }
//        else { return false; }
//      }
//
//      bool ExtractTo(const std::string &entry_name, const std::string &target)
//      {
//        int fd = 0;
//        file_path *path = NULL;
//
//        if (!StartReading()) { return false; }
//        if (Find(entry_name) == NULL) { return false; }
//        try {
//          path = file_path::create(target);
//          if (path == NULL) { throw -1; }
//
//          std::string dir = path->get_dir_path();
//          file_system::mkdir(dir.c_str(), true);
//
//          int fd = ::open(target.c_str(), O_WRONLY | O_CREAT, S_IREAD | S_IWRITE);
//          if (fd == -1) { throw -2; }
//
//          int size = archive_read_data_into_fd(r, fd);
//          ::close(fd);
//          delete path;
//
//          if (size >= 0) { return true; }
//          else { return false; }
//        }
//        catch (...) {
//          if (fd > 0) { ::close(fd); }
//          delete path;
//          return false;
//        }
//      }
//
//      bool SetCompression()
//      {
//        switch (compress)
//        {
//          case ARCHIVE_COMPRESSION_NONE:
//            archive_write_set_compression_none(w);
//            return true;
//          case ARCHIVE_COMPRESSION_GZIP:
//            archive_write_set_compression_gzip(w);
//            return true;
//          case ARCHIVE_COMPRESSION_XZ:
//            archive_write_set_compression_xz(w);
//            return true;
//          default:
//            return false;
//        }
//      }
//
//      bool StartReading()
//      {
//        Clean();
//
//        /*
//        if (file_system::dir_exists(filename.c_str()))
//        {
//          // filename is (existing) directory
//          r = archive_read_disk_new();
//          if (r == NULL) { return false; }
//          archive_read_disk_set_standard_lookup(r);
//          return true;
//        }
//        */
//
//        r = archive_read_new();
//        archive_read_support_compression_all(r);
//        archive_read_support_format_all(r);
//
//        if (archive_read_open_filename(r, filename.c_str(), 10240) != ARCHIVE_OK)
//        {
//          return false;
//        }
//        return true;
//      }
//
      bool StartWriting()
      {
        Clean();
        w = zipOpen64(archive_path.c_str(), APPEND_STATUS_ADDINZIP);

        if (! w) { return false; }
        return true;
      }

      unzFile r;
      zipFile w;
      std::string archive_path;
      int compress_level;
      std::string last_find;
  };

  static myArchive* cast_arc(void *obj) { return (myArchive *)obj; }

  archive::archive() : _obj(NULL) { }

  archive::~archive()
  {
    if (_obj) { delete cast_arc(_obj); }
  }

  bool archive::add_data(const std::string &entry_name,
                         const std::string &data,
                         const char *password,
                         const char *comment,
                         const char *compression_method,
                         int compression_level)
  {
    myArchive *arc = cast_arc(_obj);

    if (entry_name.empty()) { return false; }
    if (arc->w == NULL)
    {
      if (! arc->StartWriting()) { return false; }
    }

    bool large = data.length() > 0xffffffff;

    zip_fileinfo zi;
    zi.tmz_date.tm_sec  = zi.tmz_date.tm_min = zi.tmz_date.tm_hour = 0;
    zi.tmz_date.tm_mday = zi.tmz_date.tm_mon = zi.tmz_date.tm_year = 0;
    zi.dosDate = 0;
    zi.internal_fa = 0;
    zi.external_fa = 0;

    unsigned long crc = crc32(0, (const Bytef *)data.c_str(), data.length());
    int method = 0;
    if (compression_method == NULL) { method = Z_DEFLATED; }
    else if (strcmp(compression_method, "gzip") == 0) { method = Z_DEFLATED; }
    else if (strcmp(compression_method, "store") == 0) { method = 0; }

    if (compression_level < 0)
    {
      method = 0;
      compression_level = 0;
    }
    else if (compression_level > 9)
    { compression_level = 9; }

//printf("METHOD: %d\n", method);
//printf("LEVEL: %d\n", compression_level);
    int result = zipOpenNewFileInZip3_64
                 (
                   arc->w, entry_name.c_str(), &zi,
                   NULL, 0, /* no local extra fields */
                   NULL, 0, /* no global extra fields */
                   comment,
                   method, compression_level, 0,
                   -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                   password, crc, large
                 );
    if (result != ZIP_OK) { return false; }

    result = zipWriteInFileInZip(arc->w, data.c_str(), data.length());
    if (result != ZIP_OK) { return false; }

    zipCloseFileInZip(arc->w);
    return true;
  }

  int archive::add_data_l(lua_State *L)
  {
    using namespace luabind;
    const char *entry = NULL;
    const char *data = NULL;
    const char *pass = NULL;
    const char *comment = NULL;
    const char *method = NULL;
    int level = 1;

    try {
      luaL_checktype(L, 1, LUA_TUSERDATA);
      lev::archive *arc = object_cast<lev::archive *>(object(from_stack(L, 1)));
      if (arc == NULL) { luaL_error(L, "object (lev.archive) is not specified"); }
      object t = util::get_merged(L, 2, -1);

      if (t["entry_name"]) { entry = object_cast<const char *>(t["entry_name"]); }
      else if (t["entry"]) { entry = object_cast<const char *>(t["entry"]); }
      else if (t["e"]) { entry = object_cast<const char *>(t["e"]); }
      else if (t["lua.string1"]) { entry = object_cast<const char *>(t["lua.string1"]); }
      if (! entry) { luaL_error(L, "entry (string) is not specified"); }

      if (t["data"]) { data = object_cast<const char *>(t["data"]); }
      else if (t["d"]) { data = object_cast<const char *>(t["d"]); }
      else if (t["lua.string2"]) { data = object_cast<const char *>(t["lua.string2"]); }
      if (! data) { luaL_error(L, "data (string) is not specified"); }

      if (t["password"]) { pass = object_cast<const char *>(t["password"]); }
      else if (t["pass"]) { pass = object_cast<const char *>(t["pass"]); }
      else if (t["p"]) { pass = object_cast<const char *>(t["p"]); }
      else if (t["lua.string3"]) { pass = object_cast<const char *>(t["lua.string3"]); }

      if (t["comment"]) { comment = object_cast<const char *>(t["comment"]); }
      else if (t["com"]) { comment = object_cast<const char *>(t["com"]); }
      else if (t["c"]) { comment = object_cast<const char *>(t["c"]); }
      else if (t["lua.string4"]) { comment = object_cast<const char *>(t["lua.string4"]); }

      if (t["method"]) { method = object_cast<const char *>(t["method"]); }
      else if (t["m"]) { method = object_cast<const char *>(t["m"]); }
      else if (t["lua.string5"]) { method = object_cast<const char *>(t["lua.string5"]); }

      if (t["level"]) { level = object_cast<int>(t["level"]); }
      else if (t["l"]) { level = object_cast<int>(t["l"]); }
      else if (t["lua.number1"]) { level = object_cast<int>(t["lua.number1"]); }

      bool res = arc->add_data(entry, data, pass, comment, method, level);
      lua_pushboolean(L, res);
      return 1;
    }
    catch (...) {
      luaL_error(L, "error occured in lev.archive.add_data");
      lua_pushnil(L);
      return 1;
    }
  }

//
//  bool archive::add_file_to(const std::string &file, const std::string &entry_name)
//  {
//    myArchive *arc = cast_arc(_obj);
//    int fd = -1;
//    archive_entry *entry = NULL;
//    ::archive *read_disk = NULL;
//    size_t bytes_read;
//    char buff[8192];
//
//    if (file_system::dir_exists(file)) { return false; }
//    if (entry_name.empty()) { return false; }
//    if (arc->w == NULL)
//    {
//      if (! arc->StartWriting()) { return false; }
//    }
//
//    read_disk = archive_read_disk_new();
//    if (read_disk == NULL) { goto error; }
//    archive_read_disk_set_standard_lookup(read_disk);
//    entry = archive_entry_new();
//    if (entry == NULL) { goto error; }
//    fd = ::open(file.c_str(), O_RDONLY);
//    if (fd < 0) { goto error; }
//    archive_entry_copy_pathname(entry, entry_name.c_str());
//    archive_read_disk_entry_from_file(read_disk, entry, fd, NULL);
//    archive_write_header(arc->w, entry);
//    while ( (bytes_read = ::read(fd, buff, sizeof(buff))) > 0)
//    {
//      archive_write_data(arc->w, buff, bytes_read);
//    }
//    archive_write_finish_entry(arc->w);
//    close(fd);
//    archive_read_finish(read_disk);
//    archive_entry_free(entry);
//    return true;
//
//error:
//    if (fd > 0) { close(fd); }
//    if (read_disk) { archive_read_finish(read_disk); }
//    if (entry) { archive_entry_free(entry); }
//    return false;
//  }
//
//
//  bool archive::entry_exists(const std::string &entry_name)
//  {
//    if (! cast_arc(_obj)->StartReading()) { return false; }
//    if (cast_arc(_obj)->Find(entry_name)) { return true; }
//    return false;
//  }
//
//  bool archive::entry_exists_direct(const std::string &archive_file,
//                                    const std::string &entry_name)
//  {
//    try {
//      boost::shared_ptr<lev::archive> arc(lev::archive::open(archive_file));
//      return arc->entry_exists(entry_name);
//    }
//    catch (...) {
//      return false;
//    }
//  }
//
//  bool archive::extract(const std::string &entry_name)
//  {
//    return cast_arc(_obj)->Extract(entry_name);
//  }
//
//  bool archive::extract_direct(const std::string &archive_file,
//                               const std::string &entry_name)
//  {
//    try {
//      boost::shared_ptr<lev::archive> arc(lev::archive::open(archive_file));
//      return arc->extract(entry_name);
//    }
//    catch (...) {
//      return false;
//    }
//  }
//
//  bool archive::extract_direct_to(const std::string &archive_file,
//                                  const std::string &entry_name,
//                                  const std::string &target)
//  {
//    try {
//      boost::shared_ptr<lev::archive> arc(lev::archive::open(archive_file));
//      return arc->extract_to(entry_name, target);
//    }
//    catch (...) {
//      return false;
//    }
//  }
//
//  bool archive::extract_to(const std::string &entry_name, const std::string &target)
//  {
//    return cast_arc(_obj)->ExtractTo(entry_name, target);
//  }
//
//  bool archive::find(const std::string &pattern, std::string &entry_name)
//  {
//    myArchive *arc = cast_arc(_obj);
//    arc->last_find = pattern;
//    if (!arc->StartReading()) { return false; }
//
//    return this->find_next(entry_name);
//  }
//
//  int archive::find_l(lua_State *L)
//  {
//    using namespace luabind;
//    const char *pattern = "*";
//
//    luaL_checktype(L, 1, LUA_TUSERDATA);
//    lev::archive *arc = object_cast<lev::archive *>(object(from_stack(L, 1)));
//    if (arc == NULL) { luaL_error(L, "object (lev.archive) is not specified"); }
//    object t = util::get_merged(L, 2, -1);
//
//    if (t["pattern"]) { pattern = object_cast<const char *>(t["pattern"]); }
//    else if (t["lua.string1"]) { pattern = object_cast<const char *>(t["lua.string1"]); }
//
//    std::string name;
//    bool res = arc->find(pattern, name);
//    if (res) { lua_pushstring(L, name.c_str()); }
//    else { lua_pushnil(L); }
//    return 1;
//  }
//
//
//  bool archive::find_direct(const std::string &archive_file,
//                            const std::string &pattern,
//                            std::string &entry_name)
//  {
//    try {
//      boost::shared_ptr<lev::archive> arc(lev::archive::open(archive_file));
//      if (arc.get() == NULL) { throw -1; }
//      return arc->find(pattern, entry_name);
//    }
//    catch (...) {
//      return false;
//    }
//  }
//
//  int archive::find_direct_l(lua_State *L)
//  {
//    using namespace luabind;
//    const char *archive = NULL;
//    const char *pattern = "*";
//
//    object t = util::get_merged(L, 1, -1);
//
//    if (t["archive_file"]) { archive = object_cast<const char *>(t["archive_file"]); }
//    else if (t["archive"]) { archive = object_cast<const char *>(t["archive"]); }
//    else if (t["arc"]) { archive = object_cast<const char *>(t["arc"]); }
//    else if (t["a"]) { archive = object_cast<const char *>(t["a"]); }
//    else if (t["lua.string1"]) { archive = object_cast<const char *>(t["lua.string1"]); }
//    if (archive == NULL) { luaL_error(L, "archive (string) is not specified."); }
//
//    if (t["pattern"]) { pattern = object_cast<const char *>(t["pattern"]); }
//    else if (t["lua.string2"]) { pattern= object_cast<const char *>(t["lua.string2"]); }
//
//    std::string entry;
//    if (archive::find_direct(archive, pattern, entry))
//    {
//      lua_pushstring(L, entry.c_str());
//      return 1;
//    }
//    else
//    {
//      lua_pushnil(L);
//      return 1;
//    }
//  }
//
//
//  bool archive::find_next(std::string &entry_name)
//  {
//    myArchive *arc = cast_arc(_obj);
//    if (arc->last_find.empty()) { return false; }
//    archive_entry *entry = arc->Find(arc->last_find.c_str());
//    if (entry == NULL)
//    {
//      arc->last_find = "";
//      return false;
//    }
//    entry_name = archive_entry_pathname(entry);
//    return true;
//  }
//
//  int archive::find_next_l(lua_State *L)
//  {
//    using namespace luabind;
//
//    luaL_checktype(L, 1, LUA_TUSERDATA);
//    lev::archive *arc = object_cast<lev::archive *>(object(from_stack(L, 1)));
//    if (arc == NULL) { luaL_error(L, "object (lev.archive) is not specified"); }
////    object t = util::get_merged(L, 2, -1);
//
//    std::string name;
//    bool res = arc->find_next(name);
//    if (res) { lua_pushstring(L, name.c_str()); }
//    else { lua_pushnil(L); }
//    return 1;
//  }

  bool archive::flush()
  {
    return cast_arc(_obj)->Clean();
  }

//
//  std::string archive::get_compression_name()
//  {
//    if (cast_arc(_obj)->r == NULL)
//    {
//      if (! cast_arc(_obj)->StartReading()) { return ""; }
//    }
//    const char *compress = archive_compression_name(cast_arc(_obj)->r);
//    if (compress) { return compress; }
//    return "";
//  }
//
//  std::string archive::get_format_name()
//  {
//    if (cast_arc(_obj)->r == NULL)
//    {
//      if (! cast_arc(_obj)->StartReading()) { return ""; }
//    }
//    const char *format = archive_format_name(cast_arc(_obj)->r);
//    if (format) { return format; }
//    return "";
//  }
//
//  bool archive::is_archive(const std::string &filename)
//  {
//    ::archive *r = archive_read_new();
//    archive_read_support_compression_all(r);
//    archive_read_support_format_all(r);
//    int res = archive_read_open_filename(r, filename.c_str(), 10240);
//    if (res != ARCHIVE_OK)
//    {
//      archive_read_finish(r);
//      return false;
//    }
//
//    archive_entry *ent;
//    res = archive_read_next_header(r, &ent);
//    archive_read_finish(r);
//    if (res != ARCHIVE_OK)
//    {
//      return false;
//    }
//    return true;
//  }
//
  archive* archive::open(const std::string &archive_path)
  {
    archive *arc = NULL;
    try {
      arc = new archive;
      arc->_obj = myArchive::Create(archive_path);
      if (! arc->_obj) { throw -1; }
      return arc;
    }
    catch (...) {
      delete arc;
      return NULL;
    }
  }
//
//  bool archive::read(const std::string &entry_name, std::string &data, int block_size)
//  {
//    myArchive *arc = cast_arc(_obj);
//    archive_entry *entry;
//
//    if (!arc->StartReading()) { return NULL; }
//    while (arc->Find(entry_name))
//    {
//      data = "";
//      char buffer[block_size];
//      for ( ; ; )
//      {
//        int size = archive_read_data(arc->r, buffer, block_size);
//        if (size < 0) { break; }
//        else if (size == 0) { return true; }
//        else // if (size > 0)
//        { data.append(buffer, size); }
//      }
//    }
//    return false;
//  }
//
//  int archive::read_l(lua_State *L)
//  {
//    using namespace luabind;
//    const char *entry = NULL;
//    int bs = 4096;
//
//    luaL_checktype(L, 1, LUA_TUSERDATA);
//    lev::archive *arc = object_cast<lev::archive *>(object(from_stack(L, 1)));
//    if (arc == NULL) { return 0; }
//    object t = util::get_merged(L, 2, -1);
//
//    if (t["entry_name"]) { entry = object_cast<const char *>(t["entry_name"]); }
//    else if (t["entry"]) { entry = object_cast<const char *>(t["entry"]); }
//    else if (t["e"]) { entry = object_cast<const char *>(t["e"]); }
//    else if (t["lua.string1"]) { entry = object_cast<const char *>(t["lua.string1"]); }
//    if (entry == NULL) { luaL_error(L, "entry (string) is not specified."); }
//
//    if (t["block_size"]) { bs = object_cast<int>(t["block_size"]); }
//    else if (t["block"]) { bs = object_cast<int>(t["block"]); }
//    else if (t["bs"]) { bs = object_cast<int>(t["bs"]); }
//    else if (t["lua.number1"]) { bs = object_cast<int>(t["lua.number1"]); }
//
//    std::string data;
//    if (arc->read(entry, data, bs))
//    {
//      lua_pushstring(L, data.c_str());
//      return 1;
//    }
//    else
//    {
//      lua_pushnil(L);
//      return 1;
//    }
//  }
//
//  bool archive::read_direct(const std::string &archive_file,
//                            const std::string &entry_name,
//                            std::string &data,
//                            int block_size)
//  {
//    try {
//      boost::shared_ptr<lev::archive> arc(lev::archive::open(archive_file));
//      if (arc.get() == NULL) { throw -1; }
//      return arc->read(entry_name, data, block_size);
//    }
//    catch (...) {
//      return false;
//    }
//  }
//
//  int archive::read_direct_l(lua_State *L)
//  {
//    using namespace luabind;
//    const char *archive = NULL;
//    const char *entry = NULL;
//    int bs = 4096;
//
//    object t = util::get_merged(L, 1, -1);
//
//    if (t["archive_file"]) { archive = object_cast<const char *>(t["archive_file"]); }
//    else if (t["archive"]) { archive = object_cast<const char *>(t["archive"]); }
//    else if (t["arc"]) { archive = object_cast<const char *>(t["arc"]); }
//    else if (t["a"]) { archive = object_cast<const char *>(t["a"]); }
//    else if (t["lua.string1"]) { archive = object_cast<const char *>(t["lua.string1"]); }
//    if (archive == NULL) { luaL_error(L, "archive (string) is not specified."); }
//
//    if (t["entry_name"]) { entry = object_cast<const char *>(t["entry_name"]); }
//    else if (t["entry"]) { entry = object_cast<const char *>(t["entry"]); }
//    else if (t["e"]) { entry = object_cast<const char *>(t["e"]); }
//    else if (t["lua.string2"]) { entry = object_cast<const char *>(t["lua.string2"]); }
//    if (entry == NULL) { luaL_error(L, "entry (string) is not specified."); }
//
//    if (t["block_size"]) { bs = object_cast<int>(t["block_size"]); }
//    else if (t["block"]) { bs = object_cast<int>(t["block"]); }
//    else if (t["bs"]) { bs = object_cast<int>(t["bs"]); }
//    else if (t["lua.number1"]) { bs = object_cast<int>(t["lua.number1"]); }
//
//    std::string data;
//    if (archive::read_direct(archive, entry, data, bs))
//    {
//      lua_pushstring(L, data.c_str());
//      return 1;
//    }
//    else
//    {
//      lua_pushnil(L);
//      return 1;
//    }
//  }
//
//  bool archive::set_compression_gzip()
//  {
//    cast_arc(_obj)->compress = ARCHIVE_COMPRESSION_GZIP;
//    return true;
//  }
//
//  bool archive::set_compression_none()
//  {
//    cast_arc(_obj)->compress = ARCHIVE_COMPRESSION_NONE;
//    return true;
//  }
//
//  bool archive::set_compression_xz()
//  {
//    cast_arc(_obj)->compress = ARCHIVE_COMPRESSION_XZ;
//    return true;
//  }
//
//  bool archive::set_format_tar()
//  {
//    cast_arc(_obj)->format = ARCHIVE_FORMAT_TAR_PAX_RESTRICTED;
//    return true;
//  }
//
//  bool archive::set_format_zip()
//  {
//    cast_arc(_obj)->format = ARCHIVE_FORMAT_ZIP;
//    return true;
//  }

}

