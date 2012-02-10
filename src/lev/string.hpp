#ifndef _STRING_HPP
#define _STRING_HPP

/////////////////////////////////////////////////////////////////////////////
// Name:        lev/string.hpp
// Purpose:     header for string classes
// Author:      Akiva Miura <akiva.miura@gmail.com>
// Modified by:
// Created:     09/14/2011
// Copyright:   (C) 2010-2011 Akiva Miura
// Licence:     MIT License
/////////////////////////////////////////////////////////////////////////////

#include "base.hpp"
#include <luabind/luabind.hpp>

extern "C" {
  int luaopen_lev_string(lua_State *L);
}

namespace lev
{

  class unicode_string : public base
  {
    protected:
      unicode_string();
    public:
      unicode_string(const std::string &str);
      unicode_string(const unicode_string &str);
      virtual ~unicode_string();

      unicode_string &append(const unicode_string &str);
      unicode_string &append_utf8(const std::string &str) { return append(unicode_string(str)); }
      bool assign_utf8(const std::string &src);
      static bool compare(luabind::object op1, luabind::object op2);
      bool compare_with(luabind::object rhs);
      static boost::shared_ptr<unicode_string> concat(lua_State *L, luabind::object op1, luabind::object op2);
      static boost::shared_ptr<unicode_string> create() { return from_utf8(""); }
      bool empty() const;
      int find(const unicode_string &str);
      int find_utf8(const std::string &str) { return find(str); }
      static boost::shared_ptr<unicode_string> from_utf8(const std::string &src);
      long index(size_t pos) const;
      boost::shared_ptr<unicode_string> index_str(size_t pos) const { return sub_string(pos, 1); }
      bool is_valid() { return _obj != NULL; }
      virtual type_id get_type_id() const { return LEV_TUNICODE_STRING; }
      virtual const char *get_type_name() const { return "lev.unicode_string"; }
      size_t length() const;
      boost::shared_ptr<unicode_string> sub_string(size_t from, size_t to = -1) const;
      boost::shared_ptr<unicode_string> sub_string1(size_t from) const { return sub_string(from); }
      std::string to_utf8() const;
    protected:
      void *_obj;
  };

}

#endif // _STRING_HPP

