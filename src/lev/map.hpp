#ifndef _MAP_HPP
#define _MAP_HPP

/////////////////////////////////////////////////////////////////////////////
// Name:        src/lev/map.hpp
// Purpose:     header for image mapping class
// Author:      Akiva Miura <akiva.miura@gmail.com>
// Modified by:
// Created:     02/01/2012
// Copyright:   (C) 2010-2012 Akiva Miura
// Licence:     MIT License
/////////////////////////////////////////////////////////////////////////////

#include "base.hpp"
#include "draw.hpp"
#include <boost/shared_ptr.hpp>
#include <luabind/luabind.hpp>

namespace lev
{

  class map : public drawable
  {
    protected:
      map();
    public:
      virtual ~map();
      bool clear();
      static boost::shared_ptr<map> create();
      virtual bool draw_on_image(image *dst, int x = 0, int y = 0, unsigned char alpha = 255);
      virtual bool draw_on_screen(screen *dst, int x = 0, int y = 0, unsigned char alpha = 255);
      virtual int get_h() const;
      virtual type_id get_type_id() const { return LEV_TMAP; }
      virtual int get_w() const;
      bool map_image(boost::shared_ptr<drawable> img, int x, int y, unsigned char alpha = 255);
      static int map_image_l(lua_State *L);
      bool map_link(boost::shared_ptr<drawable> img, boost::shared_ptr<drawable> hover_img,
                    int x, int y, luabind::object on_lclick, luabind::object on_hover,
                    unsigned char alpha = 255);
      static int map_link_l(lua_State *L);
      bool on_hover(int x, int y);
      bool on_left_click(int x, int y);
      bool pop_back();
      virtual bool texturize(bool force = false);
    protected:
      void *_obj;
  };

}

#endif // _MAP_HPP

