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

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <lua.h>

extern "C" {
  int luaopen_lev_draw(lua_State *L);
}

namespace lev
{

  class bitmap;

  class drawable : public base
  {
    protected:
      drawable() : base(), descent(0) { }
    public:
      virtual ~drawable() { }

      virtual bool draw_on(bitmap *dst, int x = 0, int y = 0, unsigned char alpha = 255)
      { return false; }
      virtual bool draw_on1(bitmap *dst) { return draw_on(dst); }
      virtual bool draw_on3(bitmap *dst, int x, int y) { return draw_on(dst, x, y); }

      static boost::shared_ptr<drawable> as_drawable_from_animation(boost::shared_ptr<class animation> d);
      static boost::shared_ptr<drawable> as_drawable_from_bitmap(boost::shared_ptr<class bitmap> d);
      static boost::shared_ptr<drawable> as_drawable_from_layout(boost::shared_ptr<class layout> d);
      static boost::shared_ptr<drawable> as_drawable_from_map(boost::shared_ptr<class map> d);
      static boost::shared_ptr<drawable> as_drawable_from_screen(boost::shared_ptr<class screen> d);
      static boost::shared_ptr<drawable> as_drawable_from_transition(boost::shared_ptr<class transition> d);

      int get_ascent() const { return get_h() - descent; }
      int get_descent() const { return descent; }
      virtual int get_h() const { return 0; }
      virtual type_id get_type_id() const { return LEV_TDRAWABLE; }
      virtual int get_w() const { return 0; }
      bool set_descent(int descent);
    protected:
      int descent;
  };

  class spacer : public drawable
  {
    protected:
      spacer(int w, int h) : drawable(), w(w), h(h) { }
    public:
      static boost::shared_ptr<spacer> create(int w, int h);
      virtual int get_h() const { return h; }
      virtual type_id get_type_id() const { return LEV_TSPACER; }
      virtual int get_w() const { return w; }
    protected:
      int h, w;
  };

}

#endif // _DRAW_HPP

