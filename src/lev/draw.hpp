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
#include <boost/weak_ptr.hpp>
#include <lua.h>

extern "C" {
  int luaopen_lev_draw(lua_State *L);
}

namespace lev
{

  class image;
  class raster;
  class screen;
  class window;

  class drawable : public base
  {
    protected:
      drawable() : base() { }
    public:
      virtual ~drawable() { }
      virtual bool compile(bool force = false) { return false; }
      virtual bool compile0() { return compile(); }

      virtual bool draw_on_image(image *dst, int x = 0, int y = 0, unsigned char alpha = 255)
      { return false; }

      virtual bool draw_on_image1(image *dst) { return draw_on_image(dst); }
      virtual bool draw_on_image3(image *dst, int x, int y) { return draw_on_image(dst, x, y); }

      virtual bool draw_on_screen(screen *dst, int x = 0, int y = 0, unsigned char alpha = 255)
      { return false; }

      virtual bool draw_on_screen1(screen *dst) { return draw_on_screen(dst); }
      virtual bool draw_on_screen3(screen *dst, int x, int y)
      { return draw_on_screen(dst, x, y); }

      virtual int get_h() const { return 0; }
      virtual type_id get_type_id() const { return LEV_TDRAWABLE; }
      virtual const char *get_type_name() const { return "lev.drawable"; }
      virtual int get_w() const { return 0; }
      virtual bool is_compiled() { false; }
      virtual bool is_texturized() { false; }
      virtual bool texturize(bool force = false) { return false; }
      virtual bool texturize0() { return texturize(); }
  };

  // screen (graphic context manager) class
  class screen : public base
  {
    protected:
      screen();
    public:
      ~screen();
      bool blit(int x, int y, image *src,
                int src_x = 0, int src_y = 0, int w = -1, int h = -1,
                unsigned char alpha = 255);
      bool blit1(image *src) { return blit(0, 0, src); }
      bool blit2(image *src, unsigned char alpha) { return blit(0, 0, src, 0, 0, -1, -1, alpha); }
      bool blit3(int x, int y, image *src) { return blit(x, y, src); }
      bool blit4(int x, int y, image *src, unsigned char alpha) { return blit(x, y, src, 0, 0, -1, -1, alpha); }

      virtual bool clear() { return clear_color(0, 0, 0, 0); }
      virtual bool clear_color(unsigned char r,
                               unsigned char g,
                               unsigned char b,
                               unsigned char a = 255);
      virtual bool clear_color1(const color &c);
      virtual bool clear_color3(unsigned char r, unsigned char g, unsigned char b)
      { return clear_color(r, g, b); }

//      static screen* create(window *holder);
      static boost::shared_ptr<screen> create(boost::shared_ptr<window> holder);
      bool draw(drawable *src, int x = 0, int y = 0, unsigned char alpha = 255);
      static int draw_l(lua_State *L);
      bool draw_pixel(int x, int y, const color &c);
      bool draw_raster(const raster *r, int x = 0, int y = 0, const color *c = NULL);
      bool enable_alpha_blending(bool enable = true);
      bool enable_alpha_blending0() { return enable_alpha_blending(); }
      bool flip();
      void* get_rawobj() { return _obj; }
      virtual type_id get_type_id() const { return LEV_TSCREEN; }
      virtual const char *get_type_name() const { return "lev.screen"; }
      bool map2d_auto();
      bool map2d(int left, int right, int top, int bottom);
      bool set_current();
      bool swap();
    protected:
//      window *holder;
      boost::weak_ptr<window> holder;
      void *_obj;
  };

//  class canvas : public control
//  {
//    protected:
//      canvas();
//    public:
//      ~canvas();
//      bool call_compiled(drawable *img);
//      bool call_texture(drawable *img);
//      bool clear();
//      bool clear_color(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
//      bool clear_color1(const color &c);
//      bool clear_color3(unsigned char r, unsigned char g, unsigned char b)
//      { return clear_color(r, g, b, 0); }
//
//      bool compile(drawable *img, bool force = false);
//      bool compile1(drawable *img) { return compile(img); }
//      static canvas* create(control *parent, int width, int height);
//      static int create_l(lua_State *L);
//      bool draw_image(drawable *img, int x = 0, int y = 0, unsigned char alpha = 255);
//      bool draw_image1(drawable *img) { return draw_image(img); }
//      bool draw_image3(drawable *img, int x, int y) { return draw_image(img, x, y); }
//      bool draw_point(point *pt);
//      static int draw_l(lua_State *L);
//      static int draw_points(lua_State *L);
//      bool fill_rect(int x, int y, int w, int h, color *filling);
//      bool enable_alpha_blending(bool enable);
//      bool enable_alpha_blending0() { return enable_alpha_blending(true); }
//      void flush();
//      virtual type_id get_type_id() const { return LEV_TCANVAS; }
//      virtual const char *get_type_name() const { return "lev.gui.canvas"; }
//      void line(int x1, int y1, int x2, int y2);
//      bool map2d_auto();
//      bool map2d(int left, int right, int top, int bottom);
//      bool print(const char *text);
//      bool redraw();
//      bool set_current();
//      bool swap();
//      bool texturize(drawable *img, bool force = false);
//      bool texturize1(drawable *img) { return texturize(img); }
//  };

}

#endif // _DRAW_HPP

