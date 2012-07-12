#ifndef _DRAW_HPP
#define _DRAW_HPP

/////////////////////////////////////////////////////////////////////////////
// Name:        src/lev/draw.hpp
// Purpose:     header for drawing features
// Author:      Akiva Miura <akiva.miura@gmail.com>
// Modified by:
// Created:     12/23/2010
// Copyright:   (C) 2010-2011 Akiva Miura
// Licence:     MIT License
/////////////////////////////////////////////////////////////////////////////

#include "base.hpp"
#include "prim.hpp"

#include <boost/shared_ptr.hpp>
#include <lua.h>

extern "C" {
  int luaopen_lev_draw(lua_State *L);
}

namespace lev
{

  // type dependencies
  typedef boost::shared_ptr<class canvas> canvas_ptr;
  typedef boost::shared_ptr<class bitmap> bitmap_ptr;
  typedef boost::shared_ptr<class texture> texture_ptr;

  class drawable : public base
  {
    public:
      typedef boost::shared_ptr<drawable> ptr;
    protected:
      drawable() : base() { }
    public:
      virtual ~drawable() { }

      virtual bool compile(bool force = false) { return false; }
      virtual bool compile0() { return compile(); }

      // draw on method
      virtual bool draw_on(canvas_ptr dst, int x = 0, int y = 0, unsigned char alpha = 255)
      { return false; }
      virtual bool draw_on1(canvas_ptr dst) { return draw_on(dst); }
      virtual bool draw_on3(canvas_ptr dst, int x, int y) { return draw_on(dst, x, y); }

      int get_ascent() const { return get_h() - get_descent(); }
      virtual int get_descent() const { return 0; }
      virtual int get_h() const { return 0; }
      virtual type_id get_type_id() const { return LEV_TDRAWABLE; }
      virtual int get_w() const { return 0; }
      virtual bool is_compiled() const { return false; }
      virtual bool is_texturized() const { return false; }
      virtual bool set_descent(int d) { return false; }
      virtual bool texturize(bool force = false) { return false; }
      virtual bool texturize0() { return texturize(); }

      virtual drawable::ptr to_drawable() { drawable::ptr(); }
  };

  class spacer : public drawable
  {
    public:
      typedef boost::shared_ptr<spacer> ptr;
    protected:
      spacer(int w, int h, int descent = 0) :
        drawable(), w(w), h(h), descent(descent)
      { }
    public:
      virtual ~spacer() { }

      static spacer::ptr create(int w, int h, int descent = 0);

      virtual int get_descent() const { return descent; }
      virtual int get_h() const { return h; }
      virtual type_id get_type_id() const { return LEV_TSPACER; }
      virtual int get_w() const { return w; }
      int w, h, descent;
  };

  class canvas : public drawable
  {
    public:
      typedef boost::shared_ptr<canvas> ptr;
    protected:
      canvas() : drawable() { }
    public:
      virtual ~canvas() { }

      // blit method
      virtual bool blit(int x, int y, bitmap_ptr src,
                        int src_x = 0, int src_y = 0, int w = -1, int h = -1,
                        unsigned char alpha = 255) = 0;
      bool blit1(bitmap_ptr src) { return blit(0, 0, src); }
      bool blit2(bitmap_ptr src, unsigned char alpha)
      { return blit(0, 0, src, 0, 0, -1, -1, alpha); }
      bool blit3(int x, int y, bitmap_ptr src)
      { return blit(x, y, src); }
      bool blit4(int x, int y, bitmap_ptr src, unsigned char alpha)
      { return blit(x, y, src, 0, 0, -1, -1, alpha); }

      // clear methods
      virtual bool clear(unsigned char r = 0, unsigned char g = 0,
                         unsigned char b = 0, unsigned char a = 0) = 0;
      bool clear0() { return clear(); }
      bool clear3(unsigned char r, unsigned char g, unsigned char b)
      { return clear(r, g, b); }
      bool clear_color(color::ptr c = color::transparent());

      // draw methods
      virtual bool draw(drawable::ptr src, int x = 0, int y = 0, unsigned char alpha = 255) = 0;
      virtual bool draw_pixel(int x, int y, const color &c) = 0;
      static int draw_l(lua_State *L);

      // fill methods
      virtual bool fill_circle(int cx, int cy, int radius, color::ptr filling);
      virtual bool fill_rect(int x, int y, int w, int h, color::ptr filling);

      // get methods
      virtual color::ptr get_pixel(int x, int y) const { return color::ptr(); }
      virtual type_id get_type_id() const { return LEV_TCANVAS; }

      // stroke methods
      virtual bool stroke_circle(int x, int y, int radius, color *border, int width) { return false; }
      virtual bool stroke_line(int x1, int y1, int x2, int y2, color::ptr c, int width,
                       const std::string &style = "");
      virtual bool stroke_line6(int x1, int y1, int x2, int y2, color::ptr c, int width)
      { return stroke_line(x1, y1, x2, y2, c, width); }

      virtual bool stroke_rect(int x, int y, int w, int h, color::ptr border, int width);

      virtual canvas::ptr to_canvas() = 0;
  };

}

#endif // _DRAW_HPP

