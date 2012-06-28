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
#include "input.hpp"
#include <luabind/luabind.hpp>
#include <boost/shared_ptr.hpp>

extern "C" {
  int luaopen_lev_system(lua_State *L);
}

namespace lev
{

  // class dependencies
  class mixer;
  class screen;
  class timer;

  class event : public base
  {
    public:
      event();
      virtual ~event();
      int get_button() const;
      long get_count() const;
      int get_dw() const;
      int get_dx() const;
      int get_dy() const;
      int get_dz() const;
      long get_id() const;
      std::string get_key() const;
      long get_key_code() const;
      void *get_rawobj() { return _obj; }
      virtual type_id get_type_id() const { return LEV_TEVENT; }
      int get_w() const;
      int get_x() const;
      int get_y() const;
      int get_z() const;
      bool is_pressed() const;
      bool is_released() const;
      bool left_is_down() const;
      bool middle_is_down() const;
      bool right_is_down() const;
    protected:
      void *_obj;
  };

  class system : public base
  {
    protected:
      system();
    public:
      virtual ~system();

      boost::shared_ptr<mixer> create_mixer();

      boost::shared_ptr<timer>
        create_timer(double interval_seconds = 1, bool one_shot = false);
      boost::shared_ptr<timer> create_timer0() { return create_timer(); }
      boost::shared_ptr<timer>
        create_timer1(double interval) { return create_timer(interval); }

      boost::shared_ptr<screen>
        create_screen(const char *title, int w, int h, int x, int y);
      static int create_screen_l(lua_State *L);

      bool delay(unsigned long msec = 1000);
      bool do_event();
      bool do_events();
      bool done();
//      static boost::shared_ptr<system> get();
      double get_elapsed_time() const;
      boost::shared_ptr<input> get_input() { return input::get(); }
      static lua_State *get_interpreter();
      std::string get_name();
      luabind::object get_on_quit();
      luabind::object get_on_tick();
      virtual type_id get_type_id() const { return LEV_TSYSTEM; }
      static boost::shared_ptr<system> init(lua_State *L);
//      static luabind::object init_in_lua(lua_State *L);
      bool is_debugging();
      bool is_running();
      bool quit(bool force = false);
      bool quit0() { return quit(); }

      bool run(boost::shared_ptr<screen> main);
      bool run0();

      bool set_name(const std::string &name);
      bool set_on_quit(luabind::object func);
      bool set_on_tick(luabind::object func);
      bool set_running(bool run = true);
      bool start_debug();
      bool stop_debug();
    protected:
      bool valid;
  };

};

#endif // _SYSTEM_HPP

