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
#include <boost/filesystem.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <luabind/luabind.hpp>

int luaopen_lev_archive(lua_State *L)
{
  using namespace lev;
  using namespace luabind;

  try {
    open(L);
    globals(L)["package"]["loaded"]["lev.archive"] = true;
    globals(L)["require"]("lev.base");

    module(L, "lev")
    [
      namespace_("archive"),
      namespace_("classes")
      [
        class_<lev::archive, base, boost::shared_ptr<base> >("archive")
  //        .def("add_data", &lev::archive::add_data)
  //        .def("add_file", &lev::archive::add_file)
  //        .def("add_file_to", &lev::archive::add_file_to)
          .def("flush", &lev::archive::flush)
          .def("entry_exists", &lev::archive::entry_exists)
          .def("extract", &lev::archive::extract)
          .def("extract_to", &lev::archive::extract_to)
          .def("get_size", &lev::archive::get_uncompressed_size)
          .def("get_size", &lev::archive::get_uncompressed_size_current)
          .def("get_uncompressed_size", &lev::archive::get_uncompressed_size)
          .def("get_uncompressed_size", &lev::archive::get_uncompressed_size_current)
          .scope
          [
            def("entry_exists_direct", &lev::archive::entry_exists_direct),
            def("extract_direct", &lev::archive::extract_direct),
            def("extract_direct_to", &lev::archive::extract_direct_to),
            def("get_uncompressed_size_direct", &lev::archive::get_uncompressed_size_direct),
            def("is_archive", &lev::archive::is_archive),
            def("open", &lev::archive::open)
          ]
      ]
    ];
    object lev = globals(L)["lev"];
    object classes = lev["classes"];
    object arch = lev["archive"];

    register_to(classes["archive"], "add_data", &lev::archive::add_data_l);
    register_to(classes["archive"], "find", &lev::archive::find_l);
    register_to(classes["archive"], "find_direct", &lev::archive::find_direct_l);
    register_to(classes["archive"], "find_next", &lev::archive::find_next_l);
    register_to(classes["archive"], "read", &lev::archive::read_l);
    register_to(classes["archive"], "read_direct", &lev::archive::read_direct_l);

    arch["entry_exists"] = classes["archive"]["entry_exists_direct"];
    arch["extract"] = classes["archive"]["extract_direct"];
    arch["extract_to"] = classes["archive"]["extract_direct_to"];
    arch["find"] = classes["archive"]["find_direct"];
    arch["get_size"] = classes["archive"]["get_uncompressed_size_direct"];
    arch["get_uncompressed_size"] = classes["archive"]["get_uncompressed_size_direct"];
    arch["is_archive"] = classes["archive"]["is_archive"];
    arch["open"] = classes["archive"]["open"];
    arch["read"] = classes["archive"]["read_direct"];

    globals(L)["package"]["loaded"]["lev.archive"] = true;
  }
  catch (...) {
    fprintf(stderr, "error on initializing \"lev.archive\" library\n");
    fprintf(stderr, "error message: %s\n", lua_tostring(L, -1));
  }
  return 0;
}

namespace lev
{

  bool strmatch(const char *pattern, const char *str)
  {
    switch (*pattern)
    {
      case '\0':
        return *str == '\0';
      case '*':
        return strmatch(pattern + 1, str)
               || ((*str != '\0') && strmatch(pattern, str + 1));
      case '?':
        return (*str != '\0') && strmatch(pattern + 1, str + 1);
      default:
        return (*pattern == *str) && strmatch(pattern + 1, str + 1);
    }
  }

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
          }
          else
          {
            z = zipOpen64(archive_path.c_str(), APPEND_STATUS_CREATE);
          }
          if (! z) { throw -1; }

          arc = new myArchive;
          arc->archive_path = archive_path;
          arc->w = z;
          return arc;
        }
        catch (...) {
          fprintf(stderr, "error on myArchive creation");
          delete arc;
          return NULL;
        }
      }

      bool FindNext(std::string &entry_name)
      {
        const int buffer_size = 1024;
        char buffer[buffer_size];

        if (last_find.empty()) { return false; }
        for ( ; ; )
        {
          if (unzGoToNextFile(r) != UNZ_OK)
          {
            last_find = "";
            return false;
          }
          if (unzGetCurrentFileInfo64(r, /* info */ NULL, buffer, buffer_size,
                /* extra */ NULL, /* extra size */ 0,
                /* comment */ NULL, /* comment size */ 0) == UNZ_OK)
          {
//printf("CURRENT FILE: %s\n", buffer);
            if (strmatch(last_find.c_str(), buffer))
            {
              entry_name = buffer;
              return true;
            }
          }
        }
      }

      bool FindNew(const std::string &pattern, std::string &entry_name)
      {
        const int buffer_size = 1024;
        char buffer[buffer_size];

        if (! StartReading()) { return NULL; }
        last_find = pattern;

        if (unzGetCurrentFileInfo64(r, /* info */ NULL, buffer, buffer_size,
              /* extra */ NULL, /* extra size */ 0,
              /* comment */ NULL, /* comment size */ 0) == UNZ_OK)
        {
          if (strmatch(last_find.c_str(), buffer))
          {
            entry_name = buffer;
            return true;
          }
        }
        return FindNext(entry_name);
      }

      bool StartReading()
      {
        Clean();
        r = unzOpen64(archive_path.c_str());

        if (! r) { return false; }
        return true;
      }

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
  bool archive::entry_exists(const std::string &pattern)
  {
    std::string tmp;
    return find(pattern, tmp);
  }

  bool archive::entry_exists_direct(const std::string &archive_file,
                                    const std::string &entry_name)
  {
    try {
      boost::shared_ptr<archive> arc(lev::archive::open(archive_file));
      return arc->entry_exists(entry_name);
    }
    catch (...) {
      return false;
    }
  }

  bool archive::extract(const std::string &entry_name, const char *password)
  {
    std::string real_name;
    if (! find(entry_name, real_name)) { return false; }
    return extract_to(entry_name, real_name, password);
  }

  bool archive::extract_direct(const std::string &archive_file,
                               const std::string &entry_name,
                               const char *password)
  {
    try {
      boost::shared_ptr<lev::archive> arc(lev::archive::open(archive_file));
      return arc->extract(entry_name);
    }
    catch (...) {
      return false;
    }
  }

  bool archive::extract_direct_to(const std::string &archive_file,
                                  const std::string &entry_name,
                                  const std::string &target,
                                  const char *password)
  {
    try {
      boost::shared_ptr<lev::archive> arc(lev::archive::open(archive_file));
      return arc->extract_to(entry_name, target);
    }
    catch (...) {
      return false;
    }
  }

  bool archive::extract_to(const std::string &entry_name, const std::string &target_path,
                           const char *password)
  {
    try {
      std::string data;
      if (! read(entry_name, data, 0, password)) { return false; }

      boost::filesystem::path target = target_path;
      boost::filesystem::create_directories(target.parent_path());
      FILE *w = fopen(target_path.c_str(), "wb");
      if (! w) { return false; }

      if (fwrite(data.c_str(), 1, data.length(), w) == data.length())
      {
        fclose(w);
        return true;
      }
      fclose(w);
    }
    catch (...) {
      fprintf(stderr, "error on data extracting from archive\n");
    }
    return false;
  }

  bool archive::find(const std::string &pattern, std::string &entry_name)
  {
    return cast_arc(_obj)->FindNew(pattern, entry_name);
  }

  int archive::find_l(lua_State *L)
  {
    using namespace luabind;
    const char *pattern = "*";

    luaL_checktype(L, 1, LUA_TUSERDATA);
    lev::archive *arc = object_cast<lev::archive *>(object(from_stack(L, 1)));
    if (arc == NULL) { luaL_error(L, "object (lev.archive) is not specified"); }
    object t = util::get_merged(L, 2, -1);

    if (t["pattern"]) { pattern = object_cast<const char *>(t["pattern"]); }
    else if (t["lua.string1"]) { pattern = object_cast<const char *>(t["lua.string1"]); }

    std::string name;
    bool res = arc->find(pattern, name);
    if (res) { lua_pushstring(L, name.c_str()); }
    else { lua_pushnil(L); }
    return 1;
  }


  bool archive::find_direct(const std::string &archive_file,
                            const std::string &pattern,
                            std::string &entry_name)
  {
    try {
//      boost::scoped_ptr<lev::archive> arc(lev::archive::open(archive_file));
      boost::shared_ptr<archive> arc(lev::archive::open(archive_file));
      if (arc.get() == NULL) { throw -1; }
      return arc->find(pattern, entry_name);
    }
    catch (...) {
      return false;
    }
  }

  int archive::find_direct_l(lua_State *L)
  {
    using namespace luabind;
    const char *archive = NULL;
    const char *pattern = "*";

    object t = util::get_merged(L, 1, -1);

    if (t["archive_file"]) { archive = object_cast<const char *>(t["archive_file"]); }
    else if (t["archive"]) { archive = object_cast<const char *>(t["archive"]); }
    else if (t["arc"]) { archive = object_cast<const char *>(t["arc"]); }
    else if (t["a"]) { archive = object_cast<const char *>(t["a"]); }
    else if (t["lua.string1"]) { archive = object_cast<const char *>(t["lua.string1"]); }
    if (archive == NULL) { luaL_error(L, "archive (string) is not specified."); }

    if (t["pattern"]) { pattern = object_cast<const char *>(t["pattern"]); }
    else if (t["lua.string2"]) { pattern= object_cast<const char *>(t["lua.string2"]); }

    std::string entry;
    if (archive::find_direct(archive, pattern, entry))
    {
      lua_pushstring(L, entry.c_str());
      return 1;
    }
    else
    {
      lua_pushnil(L);
      return 1;
    }
  }


  bool archive::find_next(std::string &entry_name)
  {
    myArchive *arc = cast_arc(_obj);
    if (arc->last_find.empty()) { return false; }
    return arc->FindNext(entry_name);
  }

  int archive::find_next_l(lua_State *L)
  {
    using namespace luabind;

    luaL_checktype(L, 1, LUA_TUSERDATA);
    lev::archive *arc = object_cast<lev::archive *>(object(from_stack(L, 1)));
    if (arc == NULL) { luaL_error(L, "object (lev.archive) is not specified"); }
//    object t = util::get_merged(L, 2, -1);

    std::string name;
    bool res = arc->find_next(name);
    if (res) { lua_pushstring(L, name.c_str()); }
    else { lua_pushnil(L); }
    return 1;
  }

  bool archive::flush()
  {
    return cast_arc(_obj)->Clean();
  }

  long archive::get_uncompressed_size(const std::string &entry_name)
  {
    if (! entry_exists(entry_name)) { return -1; }
    return get_uncompressed_size_current();
  }

  long archive::get_uncompressed_size_current()
  {
    unz_file_info64 info;

    if (unzGetCurrentFileInfo64(cast_arc(_obj)->r, &info,
          NULL, 0, NULL, 0, NULL, 0) != UNZ_OK) { return -1; }
    return info.uncompressed_size;
  }

  long archive::get_uncompressed_size_direct(const std::string &archive_file,
                                             const std::string &entry_name)
  {
    try {
//      boost::scoped_ptr<lev::archive> arc(archive::open(archive_file));
      boost::shared_ptr<archive> arc(archive::open(archive_file));
      if (! arc.get()) { throw -1; }
      return arc->get_uncompressed_size(entry_name);
    }
    catch (...) {
      return -1;
    }
  }

  bool archive::is_archive(const std::string &filename)
  {
    unzFile r = unzOpen64(filename.c_str());
    if (! r) { return false; }
    return true;
  }

//  archive* archive::open(const std::string &archive_path)
  boost::shared_ptr<archive> archive::open(const std::string &archive_path)
  {
//    archive *arc = NULL;
    boost::shared_ptr<archive> arc;
    try {
      arc.reset(new archive);
      if (! arc) { throw -1; }
      arc->_obj = myArchive::Create(archive_path);
      if (! arc->_obj) { throw -2; }
    }
    catch (...) {
      arc.reset();
      fprintf(stderr, "error on archive instance creation\n");
    }
    return arc;
  }

  bool archive::read(const std::string &entry_name, std::string &data, int block_size, const char *password)
  {
    if (! entry_exists(entry_name)) { return false; }
    return read_current(data, block_size, password);
  }

  bool archive::read_current(std::string &data, int block_size, const char *password)
  {
    boost::scoped_array<unsigned char> buffer;
    try {
      if (block_size < 0) { throw -1; }
      else if (block_size == 0)
      {
        block_size = get_uncompressed_size_current();
        if (block_size < 0) { throw -2; }
      }
      buffer.reset(new unsigned char[block_size]);
      if (! buffer) { throw -3; }

      if (unzOpenCurrentFile(cast_arc(_obj)->r) == UNZ_OK)
      {
        int readed = unzReadCurrentFile(cast_arc(_obj)->r, buffer.get(), block_size);
        if (readed >= 0)
        {
          data.assign(reinterpret_cast<char *>(buffer.get()), readed);
          return true;
        }
      }
      if (unzOpenCurrentFilePassword(cast_arc(_obj)->r, password) == UNZ_OK)
      {
        int readed = unzReadCurrentFile(cast_arc(_obj)->r, buffer.get(), block_size);
        if (readed >= 0)
        {
          data.assign(reinterpret_cast<char *>(buffer.get()), readed);
          return true;
        }
      }
      data = "";
    }
    catch (...) {
      fprintf(stderr, "error on read data from archive\n");
      return false;
    }

    return false;
  }

  int archive::read_l(lua_State *L)
  {
    using namespace luabind;
    const char *entry = NULL;
    const char *pass = NULL;
    int bs = 0;

    luaL_checktype(L, 1, LUA_TUSERDATA);
    lev::archive *arc = object_cast<lev::archive *>(object(from_stack(L, 1)));
    if (arc == NULL) { return 0; }
    object t = util::get_merged(L, 2, -1);

    if (t["entry_name"]) { entry = object_cast<const char *>(t["entry_name"]); }
    else if (t["entry"]) { entry = object_cast<const char *>(t["entry"]); }
    else if (t["e"]) { entry = object_cast<const char *>(t["e"]); }
    else if (t["lua.string1"]) { entry = object_cast<const char *>(t["lua.string1"]); }
//    if (entry == NULL) { luaL_error(L, "entry (string) is not specified."); }

    if (t["password"]) { pass = object_cast<const char *>(t["password"]); }
    else if (t["pass"]) { pass = object_cast<const char *>(t["pass"]); }
    else if (t["p"]) { pass = object_cast<const char *>(t["p"]); }
    else if (t["lua.string2"]) { pass = object_cast<const char *>(t["lua.string2"]); }

    if (t["block_size"]) { bs = object_cast<int>(t["block_size"]); }
    else if (t["block"]) { bs = object_cast<int>(t["block"]); }
    else if (t["bs"]) { bs = object_cast<int>(t["bs"]); }
    else if (t["lua.number1"]) { bs = object_cast<int>(t["lua.number1"]); }

    std::string data;
    if (entry)
    {
      if (arc->read(entry, data, bs, pass))
      {
        lua_pushstring(L, data.c_str());
        return 1;
      }
    }
    else
    {
      if (arc->read_current(data, bs, pass))
      {
        lua_pushstring(L, data.c_str());
        return 1;
      }
    }

    lua_pushnil(L);
    return 1;
  }

  bool archive::read_direct(const std::string &archive_file,
                            const std::string &entry_name,
                            std::string &data,
                            int block_size,
                            const char *password)
  {
    try {
//      boost::scoped_ptr<lev::archive> arc(lev::archive::open(archive_file));
      boost::shared_ptr<archive> arc(lev::archive::open(archive_file));
      if (! arc) { throw -1; }
      return arc->read(entry_name, data, block_size, password);
    }
    catch (...) {
      return false;
    }
  }

  int archive::read_direct_l(lua_State *L)
  {
    using namespace luabind;
    const char *archive = NULL;
    const char *entry = NULL;
    const char *pass = NULL;
    int bs = 0;

    object t = util::get_merged(L, 1, -1);

    if (t["archive_file"]) { archive = object_cast<const char *>(t["archive_file"]); }
    else if (t["archive"]) { archive = object_cast<const char *>(t["archive"]); }
    else if (t["arc"]) { archive = object_cast<const char *>(t["arc"]); }
    else if (t["a"]) { archive = object_cast<const char *>(t["a"]); }
    else if (t["lua.string1"]) { archive = object_cast<const char *>(t["lua.string1"]); }
    if (archive == NULL) { luaL_error(L, "archive (string) is not specified."); }

    if (t["entry_name"]) { entry = object_cast<const char *>(t["entry_name"]); }
    else if (t["entry"]) { entry = object_cast<const char *>(t["entry"]); }
    else if (t["e"]) { entry = object_cast<const char *>(t["e"]); }
    else if (t["lua.string2"]) { entry = object_cast<const char *>(t["lua.string2"]); }
    if (entry == NULL) { luaL_error(L, "entry (string) is not specified."); }

    if (t["password"]) { pass = object_cast<const char *>(t["password"]); }
    else if (t["pass"]) { pass = object_cast<const char *>(t["pass"]); }
    else if (t["p"]) { pass = object_cast<const char *>(t["p"]); }
    else if (t["lua.string3"]) { pass = object_cast<const char *>(t["lua.string3"]); }

    if (t["block_size"]) { bs = object_cast<int>(t["block_size"]); }
    else if (t["block"]) { bs = object_cast<int>(t["block"]); }
    else if (t["bs"]) { bs = object_cast<int>(t["bs"]); }
    else if (t["lua.number1"]) { bs = object_cast<int>(t["lua.number1"]); }

    std::string data;
    if (archive::read_direct(archive, entry, data, bs, pass))
    {
      lua_pushstring(L, data.c_str());
      return 1;
    }
    else
    {
      lua_pushnil(L);
      return 1;
    }
  }

}

