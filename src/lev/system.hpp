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

  class event : public base
  {
    public:
      event();
      virtual ~event();
      std::string get_button() const;
      int get_device() const;
      int get_dx() const;
      int get_dy() const;
      std::string get_key() const;
      long get_key_code() const;
      bool get_left() const;
      bool get_middle() const;
      bool get_right() const;
      virtual type_id get_type_id() const { return LEV_TEVENT; }
      virtual const char *get_type_name() const { return "lev.event"; }
      int get_x() const;
      int get_y() const;
      bool is_pressed() const;
      bool is_released() const;
      bool set_button_code(int button);
      bool set_device(int device);
      bool set_dx(int dx);
      bool set_dy(int dy);
      bool set_key_code(long key);
      bool set_left(bool pressed = 0);
      bool set_middle(bool pressed = 0);
      bool set_pressed(bool pressed = 0);
      bool set_released(bool released = 0);
      bool set_right(bool pressed = 0);
      bool set_x(int x);
      bool set_y(int y);
    protected:
      void *_obj;
  };

  class system : public base
  {
    protected:
      system();
    public:
      virtual ~system();
      bool delay(unsigned long msec = 1000);
      bool do_event();
      bool do_events();
      bool done();
      static system* get() { return init(); }
      static int get_l(lua_State *L);
      luabind::object get_on_button_down();
      luabind::object get_on_button_up();
      luabind::object get_on_key_down();
      luabind::object get_on_key_up();
      luabind::object get_on_left_down();
      luabind::object get_on_left_up();
      luabind::object get_on_middle_down();
      luabind::object get_on_middle_up();
      luabind::object get_on_motion();
      luabind::object get_on_quit();
      luabind::object get_on_right_down();
      luabind::object get_on_right_up();
      luabind::object get_on_tick();
      screen* get_screen() { return screen::get(); }
      unsigned long get_ticks();
      virtual type_id get_type_id() const { return LEV_TSYSTEM; }
      virtual const char *get_type_name() const { return "lev.system"; }
      static system* init();
      bool is_running();
      bool quit(bool force = false);
      bool quit0() { return quit(); }
      bool run();
      bool set_on_button_down(luabind::object func);
      bool set_on_button_up(luabind::object func);
      bool set_on_key_down(luabind::object func);
      bool set_on_key_up(luabind::object func);
      bool set_on_left_down(luabind::object func);
      bool set_on_left_up(luabind::object func);
      bool set_on_middle_down(luabind::object func);
      bool set_on_middle_up(luabind::object func);
      bool set_on_motion(luabind::object func);
      bool set_on_quit(luabind::object func);
      bool set_on_right_down(luabind::object func);
      bool set_on_right_up(luabind::object func);
      bool set_on_tick(luabind::object func);
      bool set_running(bool run = true);
      screen* set_video_mode(int width, int height, int depth = 32);
      screen* set_video_mode2(int width, int height) { return set_video_mode(width, height); }
      bool toggle_full_screen();
    protected:
      void *_obj;
  };

};

#endif // _SYSTEM_HPP

