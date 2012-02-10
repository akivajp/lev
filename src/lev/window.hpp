#ifndef _WINDOW_HPP
#define _WINDOW_HPP

/////////////////////////////////////////////////////////////////////////////
// Name:        lev/window.hpp
// Purpose:     header for window managing class
// Author:      Akiva Miura <akiva.miura@gmail.com>
// Modified by:
// Created:     01/14/2012
// Copyright:   (C) 2010-2012 Akiva Miura
// Licence:     MIT License
/////////////////////////////////////////////////////////////////////////////

#include "base.hpp"
#include <boost/shared_ptr.hpp>

extern "C" {
  int luaopen_lev_window(lua_State *L);
}

namespace lev
{

  class color;
  class drawable;
  class image;
  class raster;

  class window : public base
  {
    protected:
      window();
    public:
      ~window();
      virtual bool close();
      static boost::shared_ptr<window> create(const char *title, int x, int y, int w, int h, unsigned long flags);
      int get_h();
      long get_id();
      virtual luabind::object get_on_close();
      virtual luabind::object get_on_key_down();
      virtual luabind::object get_on_key_up();
      virtual luabind::object get_on_left_down();
      virtual luabind::object get_on_left_up();
      virtual luabind::object get_on_middle_down();
      virtual luabind::object get_on_middle_up();
      virtual luabind::object get_on_motion();
      virtual luabind::object get_on_right_down();
      virtual luabind::object get_on_right_up();
      virtual luabind::object get_on_wheel();
      virtual luabind::object get_on_wheel_down();
      virtual luabind::object get_on_wheel_up();
      void* get_rawobj() { return _obj; }
      int get_w();
      virtual type_id get_type_id() const { return LEV_TWINDOW; }
      virtual const char *get_type_name() const { return "lev.window"; }
      bool hide();
      bool is_fullscreen();
      bool set_fullscreen(bool enable = true);
      bool set_fullscreen0() { return set_fullscreen(); }
      bool set_h(int h);
      virtual bool set_on_close(luabind::object func);
      virtual bool set_on_key_down(luabind::object func);
      virtual bool set_on_key_up(luabind::object func);
      virtual bool set_on_left_down(luabind::object func);
      virtual bool set_on_left_up(luabind::object func);
      virtual bool set_on_motion(luabind::object func);
      virtual bool set_on_middle_down(luabind::object func);
      virtual bool set_on_middle_up(luabind::object func);
      virtual bool set_on_right_down(luabind::object func);
      virtual bool set_on_right_up(luabind::object func);
      virtual bool set_on_wheel(luabind::object func);
      virtual bool set_on_wheel_down(luabind::object func);
      virtual bool set_on_wheel_up(luabind::object func);
      bool set_w(int w);
      bool show(bool showing = true);
      bool show0() { return show(); }
    protected:
      void* _obj;
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

      static boost::shared_ptr<screen> create(boost::shared_ptr<window> holder);
      bool draw(drawable *src, int x = 0, int y = 0, unsigned char alpha = 255);
      static int draw_l(lua_State *L);
//      bool draw_point(point *pt);
//      static int draw_points(lua_State *L);
//      void draw_line(int x1, int y1, int x2, int y2);
      bool draw_pixel(int x, int y, const color &c);
      bool draw_raster(const raster *r, int x = 0, int y = 0, const color *c = NULL);
      bool enable_alpha_blending(bool enable = true);
      bool enable_alpha_blending0() { return enable_alpha_blending(); }
//      bool fill_rect(int x, int y, int w, int h, color *filling);
      bool flip();
//      void flush();
      void* get_rawobj() { return _obj; }
//      bool print(const char *text);
//      bool redraw();
      boost::shared_ptr<image> get_screenshot();
      virtual type_id get_type_id() const { return LEV_TSCREEN; }
      virtual const char *get_type_name() const { return "lev.screen"; }
      bool map2d_auto();
      bool map2d(int left, int right, int top, int bottom);
      bool set_current();
      bool swap();
    protected:
      boost::weak_ptr<window> holder;
      void *_obj;
  };

  class debug_window : public window
  {
    protected:
      debug_window();
    public:
      virtual ~debug_window();
      bool clear();
//      virtual bool close();
      static boost::shared_ptr<debug_window> get();
      std::string get_log() { return log; }
      virtual type_id get_type_id() const { return LEV_TDEBUG_WINDOW; }
      virtual const char *get_type_name() const { return "lev.debug_window"; }
      static boost::shared_ptr<debug_window> init();
      bool print(const std::string &message_utf8, int font_size = 18);
      bool print1(const std::string &message_utf8) { return print(message_utf8); }
      static bool stop();
    protected:
      std::string log, buffer;
      boost::shared_ptr<screen> ptr_screen;
      boost::shared_ptr<layout> ptr_layout;
      static boost::shared_ptr<debug_window> singleton;
  };

}

#endif // _WINDOW_HPP

