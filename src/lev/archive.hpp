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
#include <string>
#include <boost/shared_ptr.hpp>

extern "C"
{
  int luaopen_lev_archive(lua_State *L);
}

namespace lev
{

  class archive : public base
  {
    public:
      typedef boost::shared_ptr<archive> ptr;
    protected:
      archive() : base() { }
    public:
      virtual ~archive() { }

      // add data methods
      virtual bool add_data(const std::string &entry_name,
                            const std::string &data,
                            const char *password = NULL,
                            const char *comment = NULL,
                            const char *method = "gzip",
                            int compression_level = 1) = 0;

      // add file methods
      bool add_file(const std::string &file) { return add_file_to(file, file); }
      virtual bool add_file_to(const std::string &file, const std::string &entry_name) = 0;

      // entry exists methods
      virtual bool entry_exists(const std::string &pattern) = 0;
      static bool entry_exists_direct(const std::string &archive_file,
                                      const std::string &entry_name);

      // extract methods
      virtual boost::shared_ptr<class file>
        extract(const std::string &entry_name, const char *password = NULL) = 0;
      static boost::shared_ptr<class file>
        extract_direct(const std::string &archive_file,
                       const std::string &entry_name,
                       const char *password = NULL);

//      virtual bool extract(const std::string &entry_name, const char *password = NULL) = 0;
      virtual bool extract_to(const std::string &entry_name, const std::string &target_path,
                              const char *password = NULL) = 0;
      static bool extract_direct_to(const std::string &archive_file,
                                    const std::string &entry_name,
                                    const std::string &target_path,
                                    const char *password = NULL);

      // find methods
      virtual bool find(const std::string &pattern, std::string &entry_name) = 0;
      static bool find_direct(const std::string &archive_file,
                              const std::string &pattern,
                              std::string &entry_name);

      virtual bool find_next(std::string &entry_name) = 0;

      // flush method
      virtual bool flush() = 0;

      // get methods
      virtual long get_uncompressed_size(const std::string &entry_name) = 0;
      virtual long get_uncompressed_size_current() = 0;
      static long get_uncompressed_size_direct(const std::string &archive_file,
                                               const std::string &entry_name);
      virtual type_id get_type_id() const { return LEV_TARCHIVE; }

      // is method
      static bool is_archive(const std::string &filename);

      // open method
      static archive::ptr open(const std::string &archive_path);

      // read methods
      virtual bool read(const std::string &entry_name, std::string &data,
                        int read_len = 0, const char *password = NULL) = 0;
      virtual bool read_current(std::string &data, int read_len = 0,
                                const char *password = NULL) = 0;
      static bool read_direct(const std::string &archive_file,
                              const std::string &entry_name,
                              std::string &data,
                              int block_size = 0,
                              const char *password = NULL);
  };

}

#endif // _ARCHIVE_HPP

