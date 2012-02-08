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
#include "draw.hpp"
#include <boost/shared_ptr.hpp>

extern "C" {
  int luaopen_lev_window(lua_State *L);
}

namespace lev
{

  class window : public base
  {
    protected:
      window();
    public:
      ~window();
      bool close();
//      static window* create(const char *title, int x, int y, int w, int h, unsigned long flags);
      static boost::shared_ptr<window> create(const char *title, int x, int y, int w, int h, unsigned long flags);
      static int create_l(lua_State *L);
//      screen* create_context();
      int get_h();
      long get_id();
      void* get_rawobj() { return _obj; }
      int get_w();
      virtual type_id get_type_id() const { return LEV_TWINDOW; }
      virtual const char *get_type_name() const { return "lev.window"; }
      bool hide();
      bool is_fullscreen();
      bool set_fullscreen(bool enable = true);
      bool set_fullscreen0() { return set_fullscreen(); }
      bool set_h(int h);
      bool set_w(int w);
      bool show(bool showing = true);
      bool show0() { return show(); }
    protected:
      void* _obj;
  };

}

#endif // _WINDOW_HPP

