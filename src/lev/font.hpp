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
#include "string.hpp"
#include <luabind/luabind.hpp>

extern "C" {
  int luaopen_lev_font(lua_State *L);
}

namespace lev
{

  class raster: public base
  {
    protected:
      raster();
    public:
      virtual ~raster();
      static boost::shared_ptr<raster> concat(const std::vector<boost::shared_ptr<raster> > &array);
      static boost::shared_ptr<raster> create(int width, int height);
      int get_h() const { return h; }
      unsigned char get_pixel(int x, int y) const;
      virtual type_id get_type_id() const { return LEV_TRASTER; }
      virtual const char *get_type_name() const { return "lev.raster"; }
      int get_w() const { return w; }
      bool set_pixel(int x, int y, unsigned char gray = 255);
    protected:
      int w, h;
      unsigned char *bitmap;
  };

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
      int get_pixel_size();
      std::string get_style();
      void *get_rawobj() { return _obj; }
      virtual type_id get_type_id() const { return LEV_TFONT; }
      virtual const char *get_type_name() const { return "lev.font"; }
      static boost::shared_ptr<font> load(const std::string &file = "default.ttf", int index = 0);
      static boost::shared_ptr<font> load0();
      static boost::shared_ptr<font> load1(const std::string &file) { return load(file); }
      boost::shared_ptr<raster> rasterize(unsigned long code, int spacing = 1);
      boost::shared_ptr<raster> rasterize1(unsigned long code) { return rasterize(code); }
      boost::shared_ptr<raster> rasterize_utf8(const std::string &str, int spacing = 1);
      boost::shared_ptr<raster> rasterize_utf8_1(const std::string &str) { return rasterize_utf8(str); }
      boost::shared_ptr<raster> rasterize_utf16(const unicode &str, int spacing = 1);
      boost::shared_ptr<raster> rasterize_utf16_1(const unicode &str) { return rasterize_utf16(str); }
      bool set_encoding(const std::string &encode);
      bool set_index(int index);
      bool set_pixel_size(int size);
    protected:
      void *_obj;
  };

}

#endif // _FONT_HPP

