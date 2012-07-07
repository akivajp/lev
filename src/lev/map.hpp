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

extern "C" {
  int luaopen_lev_map(lua_State *L);
}

namespace lev
{

  class map : public drawable
  {
    public:
      typedef boost::shared_ptr<map> ptr;
    protected:
      map() : drawable() { }
    public:
      virtual ~map() { }
      virtual bool clear() = 0;
      static boost::shared_ptr<map> create();
      virtual type_id get_type_id() const { return LEV_TMAP; }
      virtual bool map_image(drawable::ptr img, int x, int y, unsigned char alpha = 255) = 0;
      virtual bool map_link(drawable::ptr img, drawable::ptr hover_img,
                            int x, int y, luabind::object on_lclick, luabind::object on_hover,
                            unsigned char alpha = 255) = 0;
      virtual bool on_hover(int x, int y) = 0;
      virtual bool on_left_click(int x, int y) = 0;
      virtual bool pop_back() = 0;
  };

}

#endif // _MAP_HPP

