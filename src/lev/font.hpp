#ifndef _FONT_HPP
#define _FONT_HPP

/////////////////////////////////////////////////////////////////////////////
// Name:        lev/font.hpp
// Purpose:     header for font management classes
// Author:      Akiva Miura <akiva.miura@gmail.com>
// Created:     16/08/2010
// Copyright:   (C) 2010-2011 Akiva Miura
// Licence:     MIT License
/////////////////////////////////////////////////////////////////////////////

#include "base.hpp"
#include "prim.hpp"
#include "string.hpp"
#include <luabind/luabind.hpp>

extern "C" {
  int luaopen_lev_font(lua_State *L);
}

namespace lev
{

  class font : public base
  {
    protected:
      font();
    public:
      virtual ~font();
      boost::shared_ptr<font> clone();
      std::string get_encoding();
      std::string get_family();
      int get_index();
      int get_size();
      std::string get_style();
      void *get_rawobj() { return _obj; }
      virtual type_id get_type_id() const { return LEV_TFONT; }
      static boost::shared_ptr<font> load(const std::string &file = "default.ttf", int index = 0);
      static boost::shared_ptr<font> load0();
      static boost::shared_ptr<font> load1(const std::string &file) { return load(file); }
      boost::shared_ptr<class bitmap> rasterize(const std::string &str,
                                                color::ptr fg, color::ptr bg, color::ptr shade);
      static int rasterize_l(lua_State *L);
      boost::shared_ptr<class bitmap> rasterize_raw(unsigned long code, color::ptr fg);
//      boost::shared_ptr<raster> rasterize(unsigned long code, int spacing = 1);
//      boost::shared_ptr<raster> rasterize1(unsigned long code) { return rasterize(code); }
      boost::shared_ptr<class bitmap> rasterize_utf8(const std::string &str, color::ptr fg);
      boost::shared_ptr<class bitmap> rasterize_utf16(const ustring &str, color::ptr fg);
      bool set_encoding(const std::string &encode);
      bool set_index(int index);
      bool set_size(int size);
    protected:
      void *_obj;
  };

}

#endif // _FONT_HPP

