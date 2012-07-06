#ifndef _SCREEN_HPP
#define _SCREEN_HPP

/////////////////////////////////////////////////////////////////////////////
// Name:        lev/screen.hpp
// Purpose:     header for screen managing class
// Author:      Akiva Miura <akiva.miura@gmail.com>
// Modified by:
// Created:     01/14/2012
// Copyright:   (C) 2010-2012 Akiva Miura
// Licence:     MIT License
/////////////////////////////////////////////////////////////////////////////

#include "base.hpp"
#include <boost/shared_ptr.hpp>

extern "C" {
  int luaopen_lev_screen(lua_State *L);
}

namespace lev
{

  class screen : public canvas
  {
    public:
      typedef boost::shared_ptr<screen> ptr;
    protected:
      screen() : canvas() { }
    public:
      virtual ~screen() { }

      // blit texture method
      virtual bool blit(int dst_x, int dst_y, texture::ptr src,
                        int src_x = 0, int src_y = 0, int w = -1, int h = -1,
                        unsigned char alpha = 255) = 0;

      virtual bool close() = 0;
      static screen::ptr create(const char *title, int x, int y, int w, int h,
                                const char *style = NULL);
//      bool draw_point(point *pt);
//      static int draw_points(lua_State *L);
      virtual bool enable_alpha_blending(bool enable = true) = 0;
      bool enable_alpha_blending0() { return enable_alpha_blending(); }
//      bool fill_rect(int x, int y, int w, int h, color *filling);
//      void flush();
//      bool print(const char *text);
      virtual long get_id() const = 0;
      virtual bool hide() = 0;
      virtual luabind::object get_on_close() = 0;
      virtual luabind::object get_on_key_down() = 0;
      virtual luabind::object get_on_key_up() = 0;
      virtual luabind::object get_on_left_down() = 0;
      virtual luabind::object get_on_left_up() = 0;
      virtual luabind::object get_on_middle_down() = 0;
      virtual luabind::object get_on_middle_up() = 0;
      virtual luabind::object get_on_motion() = 0;
      virtual luabind::object get_on_right_down() = 0;
      virtual luabind::object get_on_right_up() = 0;
      virtual luabind::object get_on_wheel() = 0;
      virtual luabind::object get_on_wheel_down() = 0;
      virtual luabind::object get_on_wheel_up() = 0;
      virtual boost::shared_ptr<bitmap> get_screenshot() = 0;
      virtual type_id get_type_id() const { return LEV_TSCREEN; }
      virtual bool is_fullscreen() const = 0;
      virtual bool is_shown() const = 0;
      virtual bool map2d_auto() = 0;
      virtual bool map2d(int left, int right, int top, int bottom) = 0;
      virtual bool set_current() = 0;
      virtual bool set_fullscreen(bool enable = true) = 0;
      virtual bool set_fullscreen0() { return set_fullscreen(); }
      virtual bool set_h(int h) = 0;
      virtual bool set_on_close(luabind::object func) = 0;
      virtual bool set_on_key_down(luabind::object func) = 0;
      virtual bool set_on_key_up(luabind::object func) = 0;
      virtual bool set_on_left_down(luabind::object func) = 0;
      virtual bool set_on_left_up(luabind::object func) = 0;
      virtual bool set_on_motion(luabind::object func) = 0;
      virtual bool set_on_middle_down(luabind::object func) = 0;
      virtual bool set_on_middle_up(luabind::object func) = 0;
      virtual bool set_on_right_down(luabind::object func) = 0;
      virtual bool set_on_right_up(luabind::object func) = 0;
      virtual bool set_on_wheel(luabind::object func) = 0;
      virtual bool set_on_wheel_down(luabind::object func) = 0;
      virtual bool set_on_wheel_up(luabind::object func) = 0;
      virtual bool set_w(int w) = 0;
      virtual bool show(bool showing = true) = 0;
      bool show0() { return show(); }
      virtual bool swap() = 0;
      virtual screen::ptr to_screen() = 0;
  };

}

#endif // _SCREEN_HPP

