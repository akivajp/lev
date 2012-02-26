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

  class image;
  class screen;

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

      static boost::shared_ptr<drawable> cast_drawable_from_animation(boost::shared_ptr<class animation> d);
      static boost::shared_ptr<drawable> cast_drawable_from_image(boost::shared_ptr<class image> d);
      static boost::shared_ptr<drawable> cast_drawable_from_layout(boost::shared_ptr<class layout> d);
      static boost::shared_ptr<drawable> cast_drawable_from_map(boost::shared_ptr<class map> d);
      static boost::shared_ptr<drawable> cast_drawable_from_texture(boost::shared_ptr<class texture> d);
      static boost::shared_ptr<drawable> cast_drawable_from_transition(boost::shared_ptr<class transition> d);

      virtual int get_h() const { return 0; }
      virtual type_id get_type_id() const { return LEV_TDRAWABLE; }
      virtual int get_w() const { return 0; }
      virtual bool is_compiled() { false; }
      virtual bool is_texturized() { false; }
      virtual bool texturize(bool force = false) { return false; }
      virtual bool texturize0() { return texturize(); }

  };

}

#endif // _DRAW_HPP

