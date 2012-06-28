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
#include "draw.hpp"
#include "string.hpp"
#include <luabind/luabind.hpp>

extern "C" {
  int luaopen_lev_font(lua_State *L);
}

namespace lev
{

  class bitmap;
  class color;

  class word : public drawable
  {
    protected:
      word();
    public:
      virtual ~word();

      static boost::shared_ptr<word>
        create(boost::shared_ptr<class font> f,
               boost::shared_ptr<color> fg,
               boost::shared_ptr<color> bg,
               boost::shared_ptr<color> shade,
               const std::string &text, const std::string &ruby = "");
//      static boost::shared_ptr<word>
//        create2(boost::shared_ptr<class font> f, const std::string &text)
//      { return create(f, text); }

      virtual bool draw_on(bitmap *dst, int x = 0, int y = 0, unsigned char alpha = 255);
//      virtual type_id get_type_id() const { return LEV_TWORD; }

      virtual int get_h() const { return h; }
      virtual int get_w() const { return w; }
      bool set_under_line(bool set = true) { underlined = set; return true; }

    protected:
      int w, h;
      std::string text, ruby;
      boost::shared_ptr<class font> ft_text, ft_ruby;
      boost::shared_ptr<color> fg, bg, shade;
      bool underlined;
  };

  class font : public base
  {
    protected:
      font();
    public:
      virtual ~font();
      int calc_width(const std::string &str) const;
      boost::shared_ptr<font> clone();
      static bool destroy_cache();
      int get_ascent() const;
      int get_descent() const;
      int get_height() const;
//      std::string get_encoding();
//      std::string get_family();
//      int get_index();
      std::string get_path() const;
      int get_size();
//      std::string get_style();
      void *get_rawobj();
      virtual type_id get_type_id() const { return LEV_TFONT; }
      static boost::shared_ptr<font> load(const std::string &file = "default.ttf", int size = 20);
      static boost::shared_ptr<font> load0();
      static boost::shared_ptr<font> load1(const std::string &file) { return load(file); }
      boost::shared_ptr<bitmap> rasterize(const std::string &str, boost::shared_ptr<color> fg,
                                          boost::shared_ptr<color> bg = boost::shared_ptr<color>(),
                                          boost::shared_ptr<color> shade = boost::shared_ptr<color>());
      static int rasterize_l(lua_State *L);
//      boost::shared_ptr<bitmap> rasterize_shaded(const std::string &str, boost::shared_ptr<color> fg,
//                                                 boost::shared_ptr<color> shade);
//      boost::shared_ptr<bitmap> rasterize_bg(const std::string &str, boost::shared_ptr<color> fg,
//                                             boost::shared_ptr<color> bg);

//      boost::shared_ptr<raster> rasterize(unsigned long code, int spacing = 1);
//      boost::shared_ptr<raster> rasterize1(unsigned long code) { return rasterize(code); }
//      boost::shared_ptr<raster> rasterize_utf8(const std::string &str, int spacing = 1);
//      boost::shared_ptr<raster> rasterize_utf8_1(const std::string &str) { return rasterize_utf8(str); }
//      boost::shared_ptr<raster> rasterize_utf16(const ustring &str, int spacing = 1);
//      boost::shared_ptr<raster> rasterize_utf16_1(const ustring &str) { return rasterize_utf16(str); }
//      bool set_encoding(const std::string &encode);
//      bool set_index(int index);
      bool set_size(int size);
    protected:
      boost::shared_ptr<void> _obj;
//      void *_obj;
  };

}

#endif // _FONT_HPP

