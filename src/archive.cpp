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
#include "lev/debug.hpp"
#include "lev/entry.hpp"
#include "lev/fs.hpp"
#include "lev/util.hpp"

// external libraries
extern "C" {
  #include <minizip/unzip.h>
  #include <minizip/zip.h>
}
#include <boost/filesystem.hpp>
#include <boost/scoped_array.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <luabind/luabind.hpp>

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

  // archive class implementation
  class impl_archive : public archive
  {
    public:
      typedef boost::shared_ptr<impl_archive> ptr;
    protected:
      impl_archive() :
        archive(),
        r(NULL), w(NULL),
        current_opened(false)
      { }
    public:
      virtual ~impl_archive()
      {
        flush();
      }

      virtual bool add_data(const std::string &entry_name,
                            const std::string &data,
                            const char *password = NULL,
                            const char *comment = NULL,
                            const char *compression_method = "gzip",
                            int compression_level = 1)
      {
        if (entry_name.empty()) { return false; }
        if (! w)
        {
          if (! start_writing()) { return false; }
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
                       w, entry_name.c_str(), &zi,
                       NULL, 0, /* no local extra fields */
                       NULL, 0, /* no global extra fields */
                       comment,
                       method, compression_level, 0,
                       -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                       password, crc, large
                     );
        if (result != ZIP_OK) { return false; }

        result = zipWriteInFileInZip(w, data.c_str(), data.length());
        if (result != ZIP_OK) { return false; }

        zipCloseFileInZip(w);
        return true;
      }

      static int add_data_l(lua_State *L)
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
        }
        catch (...) {
          luaL_error(L, "error occured in archive data adding");
          lua_pushnil(L);
        }
        return 1;
      }

      virtual bool add_file_to(const std::string &file, const std::string &entry_name)
      {
        file::ptr f = file::open(file);
        if (! f) { return false; }
        std::string data;
        if (! f->read_all(data)) { return false; }
        return add_data(entry_name, data);
      }

      virtual bool entry_exists(const std::string &pattern)
      {
        std::string tmp;
        return find(pattern, tmp);
      }

      static bool entry_exists_direct(const std::string &archive_file,
                                      const std::string &entry_name)
      {
        archive::ptr arc = archive::open(archive_file);
        if (! arc) { return false; }
        return arc->entry_exists(entry_name);
      }

      virtual file::ptr extract(const std::string &entry_name, const char *password = NULL)
      {
        if (! entry_exists(entry_name)) { return memfile::ptr(); }
        int size = get_uncompressed_size_current();
        memfile::ptr mem = memfile::create(size);
        if (! mem) { return memfile::ptr(); }
        if (read_raw(mem->get_buffer(), size, password) < 0) { return memfile::ptr(); }
        return mem;
      }

//      virtual bool extract(const std::string &entry_name, const char *password)
//      {
//        std::string real_name;
//        if (! find(entry_name, real_name)) { return false; }
//        return extract_to(entry_name, real_name, password);
//      }

      static file::ptr extract_direct(const std::string &archive_file,
                                      const std::string &entry_name,
                                      const char *password)
      {
        archive::ptr arc = archive::open(archive_file);
        if (! arc) { return memfile::ptr(); }
        return arc->extract(entry_name);
      }

      static bool extract_direct_to(const std::string &archive_file,
                                     const std::string &entry_name,
                                     const std::string &target,
                                     const char *password)
      {
        archive::ptr arc = archive::open(archive_file);
        if (! arc) { return false; }
        return arc->extract_to(entry_name, target);
      }

      virtual bool extract_to(const std::string &entry_name, const std::string &target_path,
                              const char *password)
      {
        try {
          std::string data;
          if (! read(entry_name, data, 0, password)) { return false; }

          filepath::ptr target = filepath::create(target_path);
          target->get_parent()->mkdir(true);
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
          lev::debug_print("error on data extracting from archive");
        }
        return false;
      }

      virtual bool find(const std::string &pattern, std::string &entry_name)
      {
        const int buffer_size = 1024;
        char buffer[buffer_size];

        if (! start_reading()) { return NULL; }
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
        return find_next(entry_name);
      }

      static int find_l(lua_State *L)
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

      virtual bool find_next(std::string &entry_name)
      {
        const int buffer_size = 1024;
        char buffer[buffer_size];
        current_opened = false;

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

      static int find_next_l(lua_State *L)
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

      static bool find_direct(const std::string &archive_file,
                              const std::string &pattern,
                              std::string &entry_name)
      {
        archive::ptr arc = archive::open(archive_file);
        if (! arc) { return false; }
        return arc->find(pattern, entry_name);
      }

      static int find_direct_l(lua_State *L)
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

      virtual bool flush()
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
        current_opened = false;
        return true;
      }

      virtual long get_uncompressed_size(const std::string &entry_name)
      {
        if (! entry_exists(entry_name)) { return -1; }
        return get_uncompressed_size_current();
      }

      virtual long get_uncompressed_size_current()
      {
        unz_file_info64 info;

        if (unzGetCurrentFileInfo64(r, &info,
                                    NULL, 0, NULL, 0, NULL, 0) != UNZ_OK)
        { return -1; }
        return info.uncompressed_size;
      }

      static long get_uncompressed_size_direct(const std::string &archive_file,
                                               const std::string &entry_name)
      {
        archive::ptr arc = archive::open(archive_file);
        if (! arc) { return -1; }
        return arc->get_uncompressed_size(entry_name);
      }

      static bool is_archive(const std::string &filename)
      {
        unzFile r = unzOpen64(filename.c_str());
        if (! r) { return false; }
        return true;
      }

      static impl_archive::ptr open(const std::string &archive_path)
      {
        impl_archive::ptr arc;
        if (archive_path.empty()) { return arc; }
        try {
          arc.reset(new impl_archive);
          if (! arc) { throw -1; }
          if (fs::is_file(archive_path))
          {
            arc->w = zipOpen64(archive_path.c_str(), APPEND_STATUS_ADDINZIP);
          }
          else
          {
            arc->w = zipOpen64(archive_path.c_str(), APPEND_STATUS_CREATE);
          }
          if (! arc->w) { throw -2; }
          arc->archive_path = archive_path;
          return arc;
        }
        catch (...) {
          arc.reset();
          lev::debug_print("error on archive instance creation");
        }
        return arc;
      }

      virtual bool read(const std::string &entry_name, std::string &data,
                        int block_size, const char *password)
      {
        if (! entry_exists(entry_name)) { return false; }
        return read_current(data, block_size, password);
      }

      virtual bool read_current(std::string &data, int block_size, const char *password)
      {
        try {
          if (block_size < 0) { throw -1; }
          else if (block_size == 0)
          {
            block_size = get_uncompressed_size_current();
            if (block_size < 0) { throw -2; }
          }
          boost::scoped_array<unsigned char> buffer(new unsigned char[block_size]);
          if (! buffer) { throw -3; }

          int readed = read_raw(buffer.get(), block_size, password);
          if (readed <= 0) { throw -4; }
//printf("READED: %d\n", readed);
          if (readed >= 0)
          {
            data.assign((const char *)buffer.get(), readed);
            return true;
          }
        }
        catch (...) {
          lev::debug_print("error on read data from archive");
          return false;
        }
        return false;
      }

      int read_raw(unsigned char *buffer, int len, const char *pass = NULL)
      {
        if (! current_opened)
        {
          if (unzOpenCurrentFile(r) == UNZ_OK) { current_opened = true; }
        }
        if (! current_opened)
        {
          if (unzOpenCurrentFilePassword(r, pass) == UNZ_OK) { current_opened = true; }
        }
        if (! current_opened) { return -1; }
        return unzReadCurrentFile(r, buffer, len);
      }

      static int read_l(lua_State *L)
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
            lua_pushlstring(L, data.c_str(), data.length());
            return 1;
          }
        }
        else
        {
          if (arc->read_current(data, bs, pass))
          {
            lua_pushlstring(L, data.c_str(), data.length());
            return 1;
          }
        }

        lua_pushnil(L);
        return 1;
      }

      static bool read_direct(const std::string &archive_file,
                              const std::string &entry_name,
                              std::string &data,
                              int block_size,
                              const char *password)
      {
        archive::ptr arc = archive::open(archive_file);
        if (! arc) { throw -1; }
        return arc->read(entry_name, data, block_size, password);
      }

      static int read_direct_l(lua_State *L)
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
          lua_pushlstring(L, data.c_str(), data.length());
          return 1;
        }
        else
        {
          lua_pushnil(L);
          return 1;
        }
      }

      bool start_reading()
      {
        flush();
        r = unzOpen64(archive_path.c_str());

        if (! r) { return false; }
        return true;
      }

      bool start_writing()
      {
        flush();
        w = zipOpen64(archive_path.c_str(), APPEND_STATUS_ADDINZIP);

        if (! w) { return false; }
        return true;
      }

      unzFile r;
      zipFile w;
      std::string archive_path;
      std::string last_find;
      bool current_opened;
  };

  bool archive::entry_exists_direct(const std::string &archive_file,
                                    const std::string &entry_name)
  {
    return impl_archive::entry_exists_direct(archive_file, entry_name);
  }

  file::ptr archive::extract_direct(const std::string &archive_file,
                                    const std::string &entry_name,
                                    const char *password)
  {
    return impl_archive::extract_direct(archive_file, entry_name, password);
  }

  bool archive::extract_direct_to(const std::string &archive_file,
                                  const std::string &entry_name,
                                  const std::string &target_path,
                                  const char *pass)
  {
    return impl_archive::extract_direct_to(archive_file, entry_name, target_path, pass);
  }

  bool archive::find_direct(const std::string &archive_file,
                            const std::string &pattern,
                            std::string &entry_name)
  {
    return impl_archive::find_direct(archive_file, pattern, entry_name);
  }

  long archive::get_uncompressed_size_direct(const std::string &archive_file,
                                             const std::string &entry_name)
  {
    return impl_archive::get_uncompressed_size_direct(archive_file, entry_name);
  }

  bool archive::is_archive(const std::string &filename)
  {
    return impl_archive::is_archive(filename);
  }

  archive::ptr archive::open(const std::string &archive_path)
  {
    return impl_archive::open(archive_path);
  }

  bool archive::read_direct(const std::string &archive_file,
                            const std::string &entry_name,
                            std::string &data,
                            int block_size,
                            const char *password)
  {
    return impl_archive::read_direct(archive_file, entry_name,
                                     data, block_size, password);
  }

}

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
        class_<lev::archive, base, base::ptr>("archive")
  //        .def("add_data", &lev::archive::add_data)
          .def("add_file", &archive::add_file)
          .def("add_file_to", &archive::add_file_to)
          .def("flush", &archive::flush)
          .def("entry_exists", &archive::entry_exists)
          .def("extract", &archive::extract)
          .def("extract_to", &archive::extract_to)
          .def("get_size", &archive::get_uncompressed_size)
          .def("get_size", &archive::get_uncompressed_size_current)
          .def("get_uncompressed_size", &archive::get_uncompressed_size)
          .def("get_uncompressed_size", &archive::get_uncompressed_size_current)
          .scope
          [
            def("entry_exists_direct", &archive::entry_exists_direct),
            def("extract_direct", &archive::extract_direct),
            def("extract_direct_to", &archive::extract_direct_to),
            def("get_uncompressed_size_direct", &archive::get_uncompressed_size_direct),
            def("is_archive", &archive::is_archive),
            def("open", &archive::open)
          ]
      ]
    ];
    object lev = globals(L)["lev"];
    object classes = lev["classes"];
    object arch = lev["archive"];

    register_to(classes["archive"], "add_data",    &impl_archive::add_data_l);
    register_to(classes["archive"], "find",        &impl_archive::find_l);
    register_to(classes["archive"], "find_direct", &impl_archive::find_direct_l);
    register_to(classes["archive"], "find_next",   &impl_archive::find_next_l);
    register_to(classes["archive"], "read",        &impl_archive::read_l);
    register_to(classes["archive"], "read_direct", &impl_archive::read_direct_l);

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
    lev::debug_print(lua_tostring(L, -1));
    lev::debug_print("error on initializing \"lev.archive\" library");
  }
  return 0;
}

