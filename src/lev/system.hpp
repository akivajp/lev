#ifndef _SYSTEM_HPP
#define _SYSTEM_HPP

/////////////////////////////////////////////////////////////////////////////
// Name:        lev/system.hpp
// Purpose:     header for system managing class
// Author:      Akiva Miura <akiva.miura@gmail.com>
// Modified by:
// Created:     13/10/2011
// Copyright:   (C) 2010-2011 Akiva Miura
// Licence:     MIT License
/////////////////////////////////////////////////////////////////////////////

#include "base.hpp"
#include "image.hpp"
#include <luabind/luabind.hpp>

extern "C" {
  int luaopen_lev_system(lua_State *L);
}

namespace lev
{

  class system : public base
  {
    protected:
      system();
    public:
      virtual ~system();
      bool delay(unsigned long msec = 1000);
      bool do_event();
      bool do_events();
      static system* get() { return init(); }
      luabind::object get_on_quit();
      screen* get_screen() { return screen::get(); }
      unsigned long get_ticks();
      virtual type_id get_type_id() const { return LEV_TSYSTEM; }
      virtual const char *get_type_name() const { return "lev.system"; }
      static system* init();
      bool set_on_quit(luabind::object func);
      screen* set_video_mode(int width, int height, int depth = 32);
      screen* set_video_mode2(int width, int height) { return set_video_mode(width, height); }
    protected:
      void *_obj;
  };

};

#endif // _SYSTEM_HPP

