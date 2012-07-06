/////////////////////////////////////////////////////////////////////////////
// Name:        src/string.cpp
// Purpose:     header for string classes
// Author:      Akiva Miura <akiva.miura@gmail.com>
// Modified by:
// Created:     09/14/2011
// Copyright:   (C) 2010-2011 Akiva Miura
// Licence:     MIT License
/////////////////////////////////////////////////////////////////////////////

// pre-compiled header
#include "prec.h"

// declarations
#include "lev/string.hpp"

// dependencies
#include "lev/debug.hpp"
#include "lev/util.hpp"

// libraries
#include <luabind/raw_policy.hpp>
#include <boost/xpressive/xpressive.hpp>

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
      class_<ustring, base, boost::shared_ptr<base> >("ustring")
        .def("append", &ustring::append)
        .def("append", &ustring::append_utf8)
        .def("cmp", &ustring::compare_with)
        .def("compare", &ustring::compare_with)
        .def("empty", &ustring::empty)
        .def("find", &ustring::find)
        .def("find", &ustring::find_utf8)
        .def("index_code", &ustring::index)
        .def("index", &ustring::index_str)
        .property("len", &ustring::length)
        .property("length", &ustring::length)
        .property("str", &ustring::to_utf8, &ustring::assign_utf8)
        .property("string", &ustring::to_utf8, &ustring::assign_utf8)
        .def("sub", &ustring::sub_string)
        .def("sub", &ustring::sub_string1)
        .def("__eq", &ustring::compare)
        .def("__concat", &ustring::concat, raw(_1))
        .def("__len", &ustring::length)
        .def("__tostring", &ustring::to_utf8)
        .scope
        [
          def("concat", &ustring::concat, raw(_1)),
          def("create", &ustring::create),
          def("create", &ustring::from_utf8)
        ],
      class_<sregex, base, boost::shared_ptr<base> >("sregex")
        .def("assign", &sregex::assign)
        .def("concat", &sregex::concat_lua, raw(_1))
        .def("gmatch", &sregex::gmatch, raw(_2))
        .def("mark", &sregex::mark_tag)
        .def("mark_tag", &sregex::mark_tag)
        .def("op_or", &sregex::choose)
        .def("replace", &sregex::replace)
        .def("__concat", &sregex::concat_lua, raw(_1))
        .def("__div", &sregex::choose)
        .scope
        [
          def("choose", &sregex::choose),
          def("compile", &sregex::compile),
          def("concat", &sregex::concat_lua, raw(_1)),
          def("create", &sregex::create),
          def("gmatch", &sregex::gmatch_static, raw(_1)),
          def("gmatch", &sregex::gmatch_string, raw(_1)),
          def("mark_tag", &sregex::mark_tag),
          def("plane", &sregex::plane),
          def("replace", &sregex::replace_static),
          def("replace", &sregex::replace_string)
        ],
      class_<sregex_compiler, base, boost::shared_ptr<base> >("sregex_compiler")
        .def("compile", &sregex_compiler::compile)
        .scope
        [
          def("create", &sregex_compiler::create)
        ]
    ]
  ];
  object lev = globals(L)["lev"];
  object classes = lev["classes"];
  object string = lev["string"];

  string["compiler"] = classes["sregex_compiler"]["create"];
  string["concat"] = classes["ustring"]["concat"];
  string["create"] = classes["ustring"]["create"];
  string["gmatch"] = classes["sregex"]["gmatch"];
  string["mark_tag"] = classes["sregex"]["mark_tag"];
  string["regex"] = classes["sregex"]["create"];
  string["regex_choose"] = classes["sregex"]["choose"];
  string["regex_compile"] = classes["sregex"]["compile"];
  string["regex_concat"] = classes["sregex"]["concat"];
  string["regex_or"] = classes["sregex"]["choose"];
  string["regex_plane"] = classes["sregex"]["plane"];
  string["replace"] = classes["sregex"]["replace"];
  string["sregex"] = classes["sregex"]["compile"];
  string["unicode"] = classes["ustring"]["create"];
  string["unistr"] = classes["ustring"]["create"];
  string["utf16"] = classes["ustring"]["create"];

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

  ustring::ustring() : base(), _obj(NULL) { }

  ustring::ustring(const std::string &src) : base(), _obj(NULL)
  {
    _obj = myString::FromUTF8(src);
    if (! _obj) { _obj = myString::FromUTF8(""); }
  }

  ustring::ustring(const ustring &src) : base(), _obj(NULL)
  {
    _obj = myString::Clone(*cast_str(src._obj));
    if (! _obj) { _obj = myString::FromUTF8(""); }
  }

  ustring::~ustring()
  {
    if (_obj)
    {
      delete cast_str(_obj);
      _obj = NULL;
    }
  }

  ustring &ustring::append(const ustring &str)
  {
    cast_str(_obj)->str.append(cast_str(str._obj)->str);
    return *this;
  }

  bool ustring::assign_utf8(const std::string &src)
  {
    myString *str = myString::FromUTF8(src);
    if (! str) { return false; }
    delete cast_str(_obj);
    _obj = str;
    return true;
  }

  bool ustring::compare(luabind::object op1, luabind::object op2)
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

  bool ustring::compare_with(luabind::object rhs)
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

  boost::shared_ptr<ustring>
    ustring::concat(lua_State *L, luabind::object op1, luabind::object op2)
  {
    using namespace luabind;

    boost::shared_ptr<ustring> uni;
    if (! L) { return uni; }
    try {
      uni.reset(new ustring);
      if (! uni) { throw -1; }
      const char *str1 = object_cast<const char *>(globals(L)["tostring"](op1));
      const char *str2 = object_cast<const char *>(globals(L)["tostring"](op2));
      uni->_obj = myString::FromUTF8(std::string(str1) + str2);
      if (! uni->_obj) { throw -2; }
    }
    catch (...) {
      uni.reset();
      lev::debug_print("error on unicode string concatination of two strings");
    }
    return uni;
  }

  bool ustring::empty() const
  {
    return cast_str(_obj)->str.empty();
  }

  int ustring::find(const ustring &str)
  {
    return cast_str(_obj)->str.find(cast_str(str._obj)->str);
  }

  boost::shared_ptr<ustring> ustring::from_utf8(const std::string &src)
  {
    boost::shared_ptr<ustring> u;
    try {
      u.reset(new ustring);
      if (! u) { throw -1; }
      u->_obj = myString::FromUTF8(src);
      if (! u->_obj) { throw -2; }
    }
    catch (...) {
      u.reset();
      lev::debug_print("error on unicode string instance creation from utf8 string");
    }
    return u;
  }

  long ustring::index(size_t pos) const
  {
    if (pos < cast_str(_obj)->str.length()) { return cast_str(_obj)->str[pos]; }
    else { return -1; }
  }

  size_t ustring::length() const
  {
    return cast_str(_obj)->str.length();
  }

  boost::shared_ptr<ustring> ustring::sub_string(size_t from,
                                                               size_t to) const
  {
    boost::shared_ptr<ustring> uni;
    try {
      uni.reset(new ustring);
      if (! uni) { throw -1; }
      uni->_obj = myString::FromUTF16(cast_str(_obj)->str.substr(from, to));
      if (! uni->_obj) { throw -2; }
    }
    catch (...) {
      uni.reset();
      lev::debug_print("error on unicode sub string creation");
    }
    return uni;
  }

  std::string ustring::to_utf8() const
  {
    std::string buf;
    cast_str(_obj)->ToUTF8(buf);
    return buf;
  }


  static boost::xpressive::sregex *cast_regex(void *obj) { return (boost::xpressive::sregex *)obj; }

  sregex::sregex() : _obj(NULL) { }

  sregex::~sregex()
  {
    if (_obj)
    {
      delete cast_regex(_obj);
      _obj = NULL;
    }
  }

  boost::shared_ptr<sregex> sregex::assign(boost::shared_ptr<sregex> self, boost::shared_ptr<sregex> src)
  {
    if (! self || ! src) { return boost::shared_ptr<sregex>(); }
    try {
      *cast_regex(self->_obj) = *cast_regex(src->_obj);
    }
    catch (...) {
      lev::debug_print("error on static regex assignment");
      return boost::shared_ptr<sregex>();
    }
    return self;
  }

  boost::shared_ptr<sregex> sregex::choose(boost::shared_ptr<sregex> lhs, boost::shared_ptr<sregex> rhs)
  {
    boost::shared_ptr<lev::sregex> re;
    if (! lhs || ! rhs) { return re; }
    try {
      re.reset(new lev::sregex);
      if (! re) { throw -1; }
      boost::xpressive::sregex r =
        boost::xpressive::by_ref(*cast_regex(lhs->_obj)) |
        boost::xpressive::by_ref(*cast_regex(rhs->_obj));
      re->_obj = new boost::xpressive::sregex(r);
      if (! re->_obj) { throw -2; }
    }
    catch (...) {
      re.reset();
      lev::debug_print("error on static regex \"or\" concatination");
    }
    return re;
  }

  boost::shared_ptr<sregex> sregex::compile(const std::string &exp)
  {
    boost::shared_ptr<lev::sregex> re;
    try {
      re.reset(new lev::sregex);
      if (! re) { throw -1; }
      boost::xpressive::sregex r = boost::xpressive::sregex::compile(exp);
      re->_obj = new boost::xpressive::sregex(r);
      if (! re->_obj) { throw -2; }
    }
    catch (...) {
      re.reset();
      lev::debug_print("error on static regex instance compilation");
    }
    return re;
  }

  boost::shared_ptr<sregex> sregex::concat(boost::shared_ptr<sregex> lhs, boost::shared_ptr<sregex> rhs)
  {
    boost::shared_ptr<lev::sregex> re;
    if (! lhs || ! rhs) { return re; }
    try {
      re.reset(new lev::sregex);
      if (! re) { throw -1; }
      boost::xpressive::sregex r =
        boost::xpressive::by_ref(*cast_regex(lhs->_obj)) >>
        boost::xpressive::by_ref(*cast_regex(rhs->_obj));
      re->_obj = new boost::xpressive::sregex(r);
      if (! re->_obj) { throw -2; }
    }
    catch (...) {
      re.reset();
      lev::debug_print("error on static regex instance concatination");
    }
    return re;
  }

  boost::shared_ptr<sregex> sregex::concat_lua(lua_State *L, luabind::object op1, luabind::object op2)
  {
    using namespace luabind;
    if (! op1.is_valid() || ! op2.is_valid()) { return boost::shared_ptr<sregex>(); }
    boost::shared_ptr<sregex> lhs, rhs;
    try {
      if (type(op1) == LUA_TUSERDATA)
      {
        lhs = object_cast<boost::shared_ptr<sregex> >(op1);
      }
      else { lhs = sregex::plane( util::tostring(op1) ); }
      if (type(op2) == LUA_TUSERDATA)
      {
        rhs = object_cast<boost::shared_ptr<sregex> >(op2);
      }
      else { rhs = sregex::plane( util::tostring(op2) ); }
    }
    catch (...) {
      lev::debug_print("lua error on static regex instance concatination");
      return boost::shared_ptr<sregex>();
    }
    return sregex::concat(lhs, rhs);
  }

  boost::shared_ptr<sregex> sregex::create()
  {
    boost::shared_ptr<lev::sregex> re;
    try {
      re.reset(new lev::sregex);
      if (! re) { throw -1; }
      re->_obj = new boost::xpressive::sregex();
      if (! re->_obj) { throw -2; }
    }
    catch (...) {
      re.reset();
      lev::debug_print("error on static regex instance creation");
    }
    return re;
  }

  int sregex::find_l(lua_State *L)
  {
  }

  luabind::object sregex::gmatch(lua_State *L, const std::string &str)
  {
    using namespace luabind;
    boost::xpressive::smatch m;
    object t;
    try {
      boost::xpressive::sregex_iterator cur(str.begin(), str.end(), *cast_regex(_obj)), end;
      if (cur == end) { return t; }
      t = newtable(L);
      for (; cur != end; cur++)
      {
        object hit = newtable(L);
        for (int i = 0; i < cur->size(); i++)
        {
          object match = newtable(L);
          match["len"]      = cur->length(i);
          match["length"]   = cur->length(i);
          match["pos"]      = cur->position(i);
          match["position"] = cur->position(i);
          match["str"]      = cur->str(i);
          match["string"]   = cur->str(i);
          hit[i] = match;
        }
        globals(L)["table"]["insert"](t, hit);
      }
    }
    catch (...) {
      lev::debug_print("error on static regex matching");
      t = object();
    }
    return t;
  }

  luabind::object sregex::gmatch_static(lua_State *L, const std::string &str, boost::shared_ptr<sregex> rex)
  {
    if (! rex) { return luabind::object(); }
    return rex->gmatch(L, str);
  }

  luabind::object sregex::gmatch_string(lua_State *L, const std::string &str, const std::string &exp)
  {
    return gmatch_static(L, str, lev::sregex::compile(exp));
  }

  boost::shared_ptr<sregex> sregex::mark_tag(boost::shared_ptr<sregex> self, int tag_id)
  {
    boost::shared_ptr<lev::sregex> re;
    if (! self) { return re; }
    try {
      boost::xpressive::mark_tag tag(tag_id);
      re.reset(new lev::sregex);
      if (! re) { throw -1; }
      boost::xpressive::sregex r = (tag = *cast_regex(self->_obj));
      re->_obj = new boost::xpressive::sregex(r);
      if (! re->_obj) { throw -2; }
    }
    catch (...) {
      re.reset();
      lev::debug_print("error on static regex instance concatination");
    }
    return re;
  }

  std::string sregex::replace(const std::string &str, const std::string &format)
  {
    return boost::xpressive::regex_replace(str, *cast_regex(_obj), format);
  }

  std::string sregex::replace_static(const std::string &str, boost::shared_ptr<sregex> rex, const std::string &format)
  {
    if (! rex) { return ""; }
    return rex->replace(str, format);
  }

  std::string sregex::replace_string(const std::string &str, const std::string &exp, const std::string &format)
  {
    return sregex::replace_static(str, lev::sregex::compile(exp), format);
  }

  boost::shared_ptr<sregex> sregex::plane(const std::string &str)
  {
    boost::shared_ptr<lev::sregex> re;
    try {
      re.reset(new lev::sregex);
      if (! re) { throw -1; }
      re->_obj = new boost::xpressive::sregex(boost::xpressive::as_xpr(str));
      if (! re->_obj) { throw -2; }
    }
    catch (...) {
      re.reset();
      lev::debug_print("error on static regex instance creation");
    }
    return re;
  }


  static boost::xpressive::sregex_compiler* cast_cmp(void *obj)
  {
    return (boost::xpressive::sregex_compiler *)obj;
  }

  sregex_compiler::sregex_compiler() : _obj(NULL) { }

  sregex_compiler::~sregex_compiler()
  {
    if (_obj)
    {
      delete cast_cmp(_obj);
      _obj = NULL;
    }
  }

  boost::shared_ptr<sregex> sregex_compiler::compile(const std::string &exp)
  {
    namespace xpressive = boost::xpressive;
    boost::shared_ptr<sregex> re;
    try {
      re.reset(new sregex);
      if (! re) { throw -1; }
      xpressive::sregex r = cast_cmp(_obj)->compile(exp, xpressive::regex_constants::ignore_white_space);
      re->_obj = new xpressive::sregex(r);
      if (! re->_obj) { throw -2; }
    }
    catch (...) {
      re.reset();
      lev::debug_print("error on static regex compilation by compiler\n");
    }
    return re;
  }

  boost::shared_ptr<sregex_compiler> sregex_compiler::create()
  {
    boost::shared_ptr<sregex_compiler> comp;
    try {
      comp.reset(new sregex_compiler);
      if (! comp) { throw -1; }
      comp->_obj = new boost::xpressive::sregex_compiler;
      if (! comp->_obj) { throw -2; }
    }
    catch (...) {
      comp.reset();
      lev::debug_print("error on static regex compiler instance creation\n");
    }
    return comp;
  }

}

