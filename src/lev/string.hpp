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

  class unistr : public base
  {
    protected:
      unistr();
      unistr(const std::string &str);
      unistr(const unistr &str);
    public:
      virtual ~unistr();
      unistr &append(const unistr &str);
      unistr &append_utf8(const std::string &str) { return append(unistr(str)); }
      bool assign_utf8(const std::string &src);
      static bool compare(luabind::object op1, luabind::object op2);
      bool compare_with(luabind::object rhs);
      static unistr* concat(luabind::object op1, luabind::object op2);
      static unistr* create() { return from_utf8(""); }
      bool empty() const;
      int find(const unistr &str);
      int find_utf8(const std::string &str) { return find(str); }
      static unistr* from_utf8(const std::string &src);
      unistr* index(size_t pos) { return sub_string(pos, 1); }
      bool is_valid() { return _obj != NULL; }
      virtual type_id get_type_id() const { return LEV_TUNISTR; }
      virtual const char *get_type_name() const { return "lev.unistr"; }
      size_t len() const;
      unistr *sub_string(size_t from, size_t to);
      unistr *sub_string1(size_t from) { return sub_string(from, -1); }
      std::string to_utf8() const;
    protected:
      void *_obj;
  };

}

