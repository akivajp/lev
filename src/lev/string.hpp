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
#include <boost/shared_ptr.hpp>
#include <luabind/luabind.hpp>

extern "C" {
  int luaopen_lev_string(lua_State *L);
}

namespace lev
{

  class ustring : public base
  {
    protected:
      ustring();
    public:
      ustring(const std::string &str);
      ustring(const ustring &str);
      virtual ~ustring();

      ustring &append(const ustring &str);
      ustring &append_utf8(const std::string &str) { return append(ustring(str)); }
      bool assign_utf8(const std::string &src);
      static bool compare(luabind::object op1, luabind::object op2);
      bool compare_with(luabind::object rhs);
      static boost::shared_ptr<ustring> concat(lua_State *L, luabind::object op1, luabind::object op2);
      static boost::shared_ptr<ustring> create() { return from_utf8(""); }
      bool empty() const;
      int find(const ustring &str);
      int find_utf8(const std::string &str) { return find(str); }
      static boost::shared_ptr<ustring> from_utf8(const std::string &src);
      long index(size_t pos) const;
      boost::shared_ptr<ustring> index_str(size_t pos) const { return sub_string(pos, 1); }
      bool is_valid() { return _obj != NULL; }
      virtual type_id get_type_id() const { return LEV_TUSTRING; }
      size_t length() const;
      boost::shared_ptr<ustring> sub_string(size_t from, size_t to = -1) const;
      boost::shared_ptr<ustring> sub_string1(size_t from) const { return sub_string(from); }
      std::string to_utf8() const;
    protected:
      void *_obj;
  };

  class sregex : public base
  {
    protected:
      sregex();
    public:
      virtual ~sregex();
      static boost::shared_ptr<sregex> assign(boost::shared_ptr<sregex> self, boost::shared_ptr<sregex> src);
      static boost::shared_ptr<sregex> choose(boost::shared_ptr<sregex> lhs, boost::shared_ptr<sregex> rhs);
      static boost::shared_ptr<sregex> compile(const std::string &exp);
      static boost::shared_ptr<sregex> concat(boost::shared_ptr<sregex> lhs, boost::shared_ptr<sregex> rhs);
      static boost::shared_ptr<sregex> concat_lua(lua_State *L, luabind::object op1, luabind::object op2);
      static boost::shared_ptr<sregex> create();
      static int find_l(lua_State *L);
      virtual type_id get_type_id() const { return LEV_TSREGEX; }
      luabind::object gmatch(lua_State *L, const std::string &str);
      static luabind::object gmatch_static(lua_State *L, const std::string &str, boost::shared_ptr<sregex> rex);
      static luabind::object gmatch_string(lua_State *L, const std::string &str, const std::string &exp);
      static boost::shared_ptr<sregex> mark_tag(boost::shared_ptr<sregex> self, int tag_id);
      std::string replace(const std::string &str, const std::string &format);
      static std::string replace_static(const std::string &str, boost::shared_ptr<sregex> rex, const std::string &format);
      static std::string replace_string(const std::string &str, const std::string &exp, const std::string &format);
      static boost::shared_ptr<sregex> plane(const std::string &str);

      friend class sregex_compiler;
    protected:
      void *_obj;
  };

  class sregex_compiler : public base
  {
    protected:
      sregex_compiler();
    public:
      virtual ~sregex_compiler();
      boost::shared_ptr<sregex> compile(const std::string &exp);
      static boost::shared_ptr<sregex_compiler> create();
      virtual type_id get_type_id() const { return LEV_TSREGEX_COMPILER; }
    protected:
      void *_obj;
  };

}

#endif // _STRING_HPP

