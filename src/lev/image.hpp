#ifndef _IMAGE_HPP
#define _IMAGE_HPP

/////////////////////////////////////////////////////////////////////////////
// Name:        lev/image.hpp
// Purpose:     header for image handling classes
// Author:      Akiva Miura <akiva.miura@gmail.com>
// Modified by:
// Created:     01/25/2011
// Copyright:   (C) 2010-2011 Akiva Miura
// Licence:     MIT License
/////////////////////////////////////////////////////////////////////////////

#include "base.hpp"
#include "draw.hpp"
#include "prim.hpp"
#include <boost/shared_ptr.hpp>
#include <luabind/luabind.hpp>

extern "C" {
  int luaopen_lev_image(lua_State *L);
}

namespace lev
{

  // class dependencies
  class path;
  class font;
//  class raster;
  class screen;

  class bitmap : public drawable
  {
    protected:
      bitmap();
    public:
      virtual ~bitmap();
      bool blit(int x, int y, bitmap *src,
                int src_x = 0, int src_y = 0, int w = -1, int h = -1,
                unsigned char alpha = 255);
      bool blit1(bitmap *src) { return blit(0, 0, src); }
      bool blit2(bitmap *src, unsigned char alpha) { return blit(0, 0, src, 0, 0, -1, -1, alpha); }
      bool blit3(int x, int y, bitmap *src) { return blit(x, y, src); }
      bool blit4(int x, int y, bitmap *src, unsigned char alpha) { return blit(x, y, src, 0, 0, -1, -1, alpha); }

      virtual bool clear(unsigned char r = 0, unsigned char g = 0, unsigned char b = 0, unsigned char a = 0);
      bool clear0() { return clear(); }
      bool clear3(unsigned char r, unsigned char g, unsigned char b) { return clear(r, g, b); }
      bool clear_color(boost::shared_ptr<color> c = color::transparent());
      bool clear_rect(int x, int y, int w, int h, boost::shared_ptr<color> c = color::transparent());
      bool clear_rect1(const rect &r) { return clear_rect2(r); }
      bool clear_rect2(const rect &r, boost::shared_ptr<color> c = color::transparent());

      virtual boost::shared_ptr<bitmap> clone();
      static boost::shared_ptr<bitmap> create(int width, int height);
      virtual bool draw(drawable *src, int x = 0, int y = 0, unsigned char alpha = 255);
      virtual bool draw_on(bitmap *dst, int x = 0, int y = 0, unsigned char alpha = 255);
      bool draw_pixel(int x, int y, const color &c);
//      bool draw_raster(const raster *r, int x = 0, int y = 0,
//                       boost::shared_ptr<const color> c = boost::shared_ptr<const color>());
      static int draw_l(lua_State *L);
      bool fill_circle(int cx, int cy, int radius, boost::shared_ptr<color> filling);
      bool fill_rectangle(int x1, int y1, int x2, int y2, boost::shared_ptr<color> filling);
      virtual int get_h() const;
      boost::shared_ptr<color> get_pixel(int x, int y);
      void* get_rawobj() const { return _obj; }
      boost::shared_ptr<rect> get_rect() const;
      boost::shared_ptr<size> get_size() const;
      virtual boost::shared_ptr<bitmap> get_sub_bitmap(int x, int y, int w, int h);
      static int get_sub_bitmap_l(lua_State *L);
      virtual type_id get_type_id() const { return LEV_TBITMAP; }
      virtual int get_w() const;
      static bitmap* levana_icon();
      static boost::shared_ptr<bitmap> load(const std::string &filename);
      static boost::shared_ptr<bitmap> load_path(boost::shared_ptr<path> p);
      bool reload(const std::string &filename);
      boost::shared_ptr<bitmap> resize(int width, int height);
      virtual bool save(const std::string &filename) const;
      virtual bool set_as_target();
      bool set_pixel(int x, int y, const color &c);
      static boost::shared_ptr<bitmap> string(boost::shared_ptr<font> f, const std::string &str,
                                             boost::shared_ptr<color> fore  = boost::shared_ptr<color>(),
                                             boost::shared_ptr<color> shade = boost::shared_ptr<color>(),
                                             boost::shared_ptr<color> back  = boost::shared_ptr<color>(),
                                             int spacing = 1);
      static int string_l(lua_State *L);
      bool stroke_circle(int x, int y, int radius, boost::shared_ptr<color> border, int width);
      bool stroke_line(int x1, int y1, int x2, int y2, boost::shared_ptr<color> c, int width);
      bool stroke_rectangle(int x1, int y1, int x2, int y2, boost::shared_ptr<color> border, int width);
      bool swap(boost::shared_ptr<bitmap> img);
    protected:
      void *_obj;

      friend class screen;
  };

  class animation : public drawable
  {
    protected:
      animation();
    public:
      virtual ~animation();
      bool append(boost::shared_ptr<drawable> img, double duration);
      bool append_file(const std::string &filename, double duration);
      bool append_path(boost::shared_ptr<path> p, double duration);
      static int append_l(lua_State *L);
      static boost::shared_ptr<animation> create(bool repeating = true);
      static boost::shared_ptr<animation> create0() { return create(); }
      virtual bool draw_on(bitmap *dst, int x = 0, int y = 0, unsigned char alpha = 255);
      boost::shared_ptr<drawable> get_current();
      virtual int get_h() const;
      virtual type_id get_type_id() const { return LEV_TANIMATION; }
      virtual int get_w() const;
    protected:
      void *_obj;
  };

  class transition : public drawable
  {
    protected:
      transition();
    public:
      virtual ~transition();
      virtual bool draw_on(bitmap *dst, int x = 0, int y = 0, unsigned char alpha = 255);
      static boost::shared_ptr<transition> create(boost::shared_ptr<drawable> img);
      static boost::shared_ptr<transition> create_with_path(boost::shared_ptr<path> p);
      static boost::shared_ptr<transition> create_with_string(const std::string &bitmap_path);
      static boost::shared_ptr<transition> create0() { return create(boost::shared_ptr<drawable>()); }
      boost::shared_ptr<drawable> get_current();
      virtual int get_h() const;
      virtual type_id get_type_id() const { return LEV_TTRANSITION; }
      virtual int get_w() const;
      bool is_running();
      bool rewind();
      bool set_current(boost::shared_ptr<drawable> current);
      bool set_current(const std::string &bitmap_path);
      static int set_current_l(lua_State *L);
      bool set_next(boost::shared_ptr<drawable> next, double duration = 1, const std::string &mode = "");
      bool set_next(const std::string &bitmap_path, double duration = 1, const std::string &mode = "");
      static int set_next_l(lua_State *L);
    protected:
      void *_obj;
  };

  class layout : public drawable
  {
    protected:
      layout();
    public:
      virtual ~layout();
      virtual bool clear();
      bool complete();
      static boost::shared_ptr<layout> create(int width_stop = -1);
      static boost::shared_ptr<layout> create0() { return create(); }
      virtual bool draw_on(bitmap *dst, int x = 0, int y = 0, unsigned char alpha = 255);
      boost::shared_ptr<color> get_fg_color();
      boost::shared_ptr<font> get_font();
      virtual int get_h() const;
      boost::shared_ptr<font> get_ruby_font();
      int get_spacing();
      boost::shared_ptr<color> get_shade_color();
      virtual type_id get_type_id() const { return LEV_TLAYOUT; }
      virtual int get_w() const;
      bool is_done();
      bool on_hover(int x, int y);
      bool on_left_click(int x, int y);
      bool rearrange();
      bool reserve_clickable(boost::shared_ptr<bitmap> normal, boost::shared_ptr<bitmap> hover,
                             luabind::object lclick_func, luabind::object hover_func);
      bool reserve_clickable_text(const std::string &text,
                                  luabind::object lclick_func, luabind::object hover_func);
      bool reserve_bitmap(boost::shared_ptr<bitmap> img);
      bool reserve_new_line();
      bool reserve_word(const std::string &word, const std::string &ruby = "");
      bool reserve_word_lua(luabind::object word, luabind::object ruby);
      bool reserve_word_lua1(luabind::object word);
      bool set_fg_color(boost::shared_ptr<color> fg);
      bool set_font(boost::shared_ptr<font> f);
      bool set_shade_color(const color *c);
      bool set_ruby_font(boost::shared_ptr<font> f);
      bool set_spacing(int space = 1);
      bool show_next();
    protected:
      void *_obj;
  };

}

#endif // _IMAGE_HPP

