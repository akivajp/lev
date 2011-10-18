/////////////////////////////////////////////////////////////////////////////
// Name:        src/string.cpp
// Purpose:     header for string classes
// Author:      Akiva Miura <akiva.miura@gmail.com>
// Modified by:
// Created:     09/14/2011
// Copyright:   (C) 2010-2011 Akiva Miura
// Licence:     MIT License
/////////////////////////////////////////////////////////////////////////////

#include "prec.h"
#include "lev/string.hpp"


int luaopen_lev_string(lua_State *L)
{
  using namespace lev;
  using namespace luabind;

  open(L);
  globals(L)["require"]("lev.base");
  globals(L)["require"]("string");
  module(L, "lev")
  [
    namespace_("string"),
    namespace_("classes")
    [
      class_<unistr, base>("unistr")
        .def("append", &unistr::append)
        .def("append", &unistr::append_utf8)
        .def("cmp", &unistr::compare_with)
        .def("compare", &unistr::compare_with)
        .def("empty", &unistr::empty)
        .def("find", &unistr::find)
        .def("find", &unistr::find_utf8)
        .def("index_code", &unistr::index)
        .def("index", &unistr::index_str, adopt(result))
        .property("len", &unistr::len)
        .property("length", &unistr::len)
        .property("str", &unistr::to_utf8, &unistr::assign_utf8)
        .property("string", &unistr::to_utf8, &unistr::assign_utf8)
        .def("sub", &unistr::sub_string, adopt(result))
        .def("sub", &unistr::sub_string1, adopt(result))
        .def("__eq", &unistr::compare)
        .def("__concat", &unistr::concat, adopt(result))
        .def("__len", &unistr::len)
        .def("__tostring", &unistr::to_utf8)
        .scope
        [
          def("concat", &unistr::concat, adopt(result)),
          def("create", &unistr::create, adopt(result)),
          def("create", &unistr::from_utf8, adopt(result))
        ]
    ]
  ];
  object lev = globals(L)["lev"];
  object classes = lev["classes"];
  object string = lev["string"];

  string["concat"] = classes["unistr"]["concat"];
  string["create"] = classes["unistr"]["create"];
  string["unistr"] = classes["unistr"]["create"];

  globals(L)["package"]["loaded"]["lev.string"] = string;
  return 0;
}


namespace lev
{

  class myString
  {
    protected:
      myString() : str(), len8(0) { }

    public:
      ~myString()
      {
      }

      static myString* Clone(const myString &src)
      {
        myString *s = NULL;
        try {
          s = new myString;
          s->len8 = src.len8;
          s->str = src.str;
          return s;
        }
        catch (...) {
          delete s;
          return NULL;
        }
      }

      static myString* FromUTF8(const std::string &src)
      {
        myString *s = NULL;
        try {
          s = new myString;
          s->len8 = src.length();
          std::wstring &str16 = s->str;
          unsigned char *str8 = (unsigned char *)src.c_str();
          unsigned long c;
          while (*str8)
          {
            if (!(*str8 & 0x80)) { str16.append(1, *str8++); }
            else if ((*str8 & 0xe0) == 0xc0)
            {
              if (*str8 < 0xc2) { throw -1; }
              c = (*str8++ & 0x1f) << 6;
              if ((*str8 & 0xc0) != 0x80) { throw -2; }
              str16.append(1, c + (*str8++ & 0x3f));
            }
            else if ((*str8 & 0xf0) == 0xe0)
            {
              if (*str8 == 0xe0 && (str8[1] < 0xa0 || str8[1] > 0xbf)) { throw -3; }
              if (*str8 == 0xed && str8[1] > 0x9f)
              {
                // str[1] < 0x80 is checked below
                throw -4;
              }
              c = (*str8++ & 0x0f) << 12;
              if ((*str8 & 0xc0) != 0x80) { throw -4; }
              c += (*str8++ & 0x3f) << 6;
              if ((*str8 & 0xc0) != 0x80) { throw -5; }
              str16.append(1, c + (*str8++ & 0x3f));
            }
            else if ((*str8 & 0xf8) == 0xf0)
            {
              if (*str8 > 0xf4) { throw -6; }
              if (*str8 == 0xf0 && (str8[1] < 0x90 || str8[1] > 0xbf)) { throw -7; }
              if (*str8 == 0xf4 && str8[1] > 0x8f)
              {
                // str[1] < 0x80 is checked below
                throw -8;
              }
              c = (*str8++ & 0x07) << 18;
              if ((*str8 & 0xc0) != 0x80) { throw -8; }
              c += (*str8++ & 0x3f) << 12;
              if ((*str8 & 0xc0) != 0x80) { throw -9; }
              c += (*str8++ & 0x3f) << 6;
              if ((*str8 & 0xc0) != 0x80) { throw -10; }
              c += (*str8++ & 0x3f);
              // utf-8 encodings of values used in surrogate pairs are invalid
              if ((c & 0xFFFFF800) == 0xD800) { throw -11; }
              if (c >= 0x10000)
              {
                c -= 0x10000;
                str16.append(1, 0xD800 | (0x3ff & (c >> 10)));
                str16.append(1, 0xDC00 | (0x3ff & (c      )));
              }
            }
            else { throw -13; }
          }
          return s;
        }
        catch (...) {
          delete s;
          return NULL;
        }
      }

      static myString* FromUTF16(const std::wstring &str)
      {
        myString *s = NULL;
        try {
          s = new myString;
          s->str = str;
          return s;
        }
        catch (...) {
          delete s;
          return NULL;
        }
      }

      bool ToUTF8(std::string &buf)
      {
        std::string &str8 = buf;
        std::wstring &str16 = str;

        buf.clear();
        for (int i = 0; i < str16.length() ;)
        {
          if (str16[i] < 0x80)
          {
            str8.append(1, str16[i++]);
          }
          else if (str16[i] < 0x800)
          {
            str8.append(1, 0xc0 + (str16[i] >> 6));
            str8.append(1, 0x80 + (str16[i] & 0x3f));
            i++;
          }
          else if (str16[i] >= 0xd800 && str16[i] < 0xdc00)
          {
            unsigned long c;
            c = ((str16[i] - 0xd800) << 10) + ((str16[i + 1]) - 0xdc00) + 0x10000;
            str8.append(1, 0xf0 + (c >> 18));
            str8.append(1, 0x80 + ((c >> 12) & 0x3f));
            str8.append(1, 0x80 + ((c >>  6) & 0x3f));
            str8.append(1, 0x80 + ((c      ) & 0x3f));
            i += 2;
          }
          else if (str16[i] >= 0xdc00 && str16[i] < 0xe000)
          {
            return false;
          }
          else
          {
            str8.append(1, 0xe0 + ( str16[i] >> 12));
            str8.append(1, 0x80 + ((str16[i] >> 6) & 0x3f));
            str8.append(1, 0x80 + ((str16[i]     ) & 0x3f));
            i++;
          }
        }
        return true;
      }

      std::wstring str;
      long len8;
  };

  static myString* cast_str(void *obj) { return (myString *)obj; }

  unistr::unistr() : base(), _obj(NULL) { }

  unistr::unistr(const std::string &src) : base(), _obj(NULL)
  {
    _obj = myString::FromUTF8(src);
    if (! _obj) { _obj = myString::FromUTF8(""); }
  }

  unistr::unistr(const unistr &src) : base(), _obj(NULL)
  {
    _obj = myString::Clone(*cast_str(src._obj));
    if (! _obj) { _obj = myString::FromUTF8(""); }
  }

  unistr::~unistr()
  {
    if (_obj)
    {
      delete cast_str(_obj);
      _obj = NULL;
    }
  }

  unistr &unistr::append(const unistr &str)
  {
    cast_str(_obj)->str.append(cast_str(str._obj)->str);
    return *this;
  }

  bool unistr::assign_utf8(const std::string &src)
  {
    myString *str = myString::FromUTF8(src);
    if (! str) { return false; }
    delete cast_str(_obj);
    _obj = str;
    return true;
  }

  bool unistr::compare(luabind::object op1, luabind::object op2)
  {
    using namespace luabind;

    lua_State *L = op1.interpreter();
    try {
      const char *str1 = object_cast<const char *>(globals(L)["tostring"](op1));
      const char *str2 = object_cast<const char *>(globals(L)["tostring"](op2));
      if (strcmp(str1, str2) == 0) { return true; }
      else { return false; }
    }
    catch (...) {
      return false;
    }
  }

  bool unistr::compare_with(luabind::object rhs)
  {
    using namespace luabind;

    lua_State *L = rhs.interpreter();
    try {
      const char *r = object_cast<const char *>(globals(L)["tostring"](rhs));
      if (to_utf8().compare(r) == 0) { return true; }
      else false;
    }
    catch (...) {
      return false;
    }
  }

  unistr* unistr::concat(luabind::object op1, luabind::object op2)
  {
    using namespace luabind;

    unistr *uni = NULL;
    lua_State *L = op1.interpreter();
    try {
      uni = new unistr;
      const char *str1 = object_cast<const char *>(globals(L)["tostring"](op1));
      const char *str2 = object_cast<const char *>(globals(L)["tostring"](op2));
      uni->_obj = myString::FromUTF8(std::string(str1) + str2);
      if (! uni->_obj) { throw -1; }
      return uni;
    }
    catch (...) {
      delete uni;
      return NULL;
    }
  }

  bool unistr::empty() const
  {
    return cast_str(_obj)->str.empty();
  }

  int unistr::find(const unistr &str)
  {
    return cast_str(_obj)->str.find(cast_str(str._obj)->str);
  }

  unistr* unistr::from_utf8(const std::string &src)
  {
    unistr *u = NULL;
    try {
      u = new unistr;
      u->_obj = myString::FromUTF8(src);
      if (! u->_obj) { throw -1; }
      return u;
    }
    catch (...) {
      delete u;
      return NULL;
    }
  }

  long unistr::index(size_t pos) const
  {
    if (pos < cast_str(_obj)->str.length()) { return cast_str(_obj)->str[pos]; }
    else { return -1; }
  }

  size_t unistr::len() const
  {
    return cast_str(_obj)->str.length();
  }

  unistr* unistr::sub_string(size_t from, size_t to) const
  {
    unistr *uni = NULL;
    try {
      uni = new unistr;
      uni->_obj = myString::FromUTF16(cast_str(_obj)->str.substr(from, to));
      if (! uni->_obj) { throw -1; }
      return uni;
    }
    catch (...) {
      delete uni;
      return NULL;
    }
  }

  std::string unistr::to_utf8() const
  {
    std::string buf;
    cast_str(_obj)->ToUTF8(buf);
    return buf;
  }

}

