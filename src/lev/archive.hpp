#ifndef _ARCHIVE_HPP
#define _ARCHIVE_HPP

/////////////////////////////////////////////////////////////////////////////
// Name:        lev/archive.hpp
// Purpose:     header for archive file handling classes
// Author:      Akiva Miura <akiva.miura@gmail.com>
// Created:     09/06/2011
// Copyright:   (C) 2010-2012 Akiva Miura
// Licence:     MIT License
/////////////////////////////////////////////////////////////////////////////

#include "base.hpp"
#include <boost/shared_ptr.hpp>

extern "C"
{
  int luaopen_lev_archive(lua_State *L);
}

namespace lev
{

  class archive : public base
  {
    protected:
      archive();
    public:
      ~archive();

      bool add_data(const std::string &entry_name,
                    const std::string &data,
                    const char *password = "",
                    const char *comment = "",
                    const char *method = "gzip",
                    int compression_level = 1);
      static int add_data_l(lua_State *L);

      bool add_file(const std::string &file) { return add_file_to(file, file); }
      bool add_file_to(const std::string &file, const std::string &entry_name);
      bool entry_exists(const std::string &pattern);
      static bool entry_exists_direct(const std::string &archive_file,
                                      const std::string &entry_name);
      bool extract(const std::string &entry_name);
      static bool extract_direct(const std::string &archive_file,
                                 const std::string &entry_name);
      static bool extract_direct_to(const std::string &archive_file,
                                    const std::string &entry_name,
                                    const std::string &target_path);
      bool extract_to(const std::string &entry_name, const std::string &target_path);
      bool find(const std::string &pattern, std::string &entry_name);
      static int find_l(lua_State *L);

      static bool find_direct(const std::string &archive_file,
                              const std::string &pattern,
                              std::string &entry_name);
      static int find_direct_l(lua_State *L);

      bool find_next(std::string &entry_name);
      static int find_next_l(lua_State *L);
      bool flush();
      long get_uncompressed_size(const std::string &entry_name);
      long get_uncompressed_size_current();
      static long get_uncompressed_size_direct(const std::string &archive_file,
                                               const std::string &entry_name);
      virtual type_id get_type_id() const { return LEV_TARCHIVE; }
      virtual const char *get_type_name() const { return "lev.archive"; }
      static bool is_archive(const std::string &filename);
//      static archive* open(const std::string &archive_path);
      static boost::shared_ptr<archive> open(const std::string &archive_path);
      bool read(const std::string &entry_name, std::string &data,
                int block_size = 4096, const char *password = NULL);
      bool read_current(std::string &data, int block_size = 4096, const char *password = NULL);
      static int read_l(lua_State *L);
      static bool read_direct(const std::string &archive_file,
                              const std::string &entry_name,
                              std::string &data,
                              int block_size = 4096,
                              const char *password = NULL);
      static int read_direct_l(lua_State *L);
    protected:
      void *_obj;
  };

}

#endif // _ARCHIVE_HPP

