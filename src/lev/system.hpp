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
#include <luabind/luabind.hpp>
#include <boost/shared_ptr.hpp>

extern "C" {
  int luaopen_lev_system(lua_State *L);
}

namespace lev
{

  // class dependencies
  class clock;
  class debug_window;
  class window;
  class timer;

  class input
  {
    public:
      static const char *to_keyname(long code);
  };

  class event : public base
  {
    public:
      event();
      virtual ~event();
      std::string get_button() const;
      int get_dx() const;
      int get_dy() const;
      int get_id() const;
      std::string get_key() const;
      long get_key_code() const;
      void *get_rawobj() { return _obj; }
      long get_scan_code() const;
      virtual type_id get_type_id() const { return LEV_TEVENT; }
      virtual const char *get_type_name() const { return "lev.event"; }
      int get_x() const;
      int get_y() const;
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
      static boost::shared_ptr<clock> create_clock(boost::shared_ptr<system> sys,
                                                   double freq = 50);
      static boost::shared_ptr<clock> create_clock1(boost::shared_ptr<system> sys)
      { return create_clock(sys); }

      boost::shared_ptr<window>
        create_window(const char *title, int x, int y, int w, int h, unsigned long flags);
      static int create_window_l(lua_State *L);

      static boost::shared_ptr<timer> create_timer(boost::shared_ptr<system> sys,
                                                   double interval = 1000);
      static boost::shared_ptr<timer> create_timer1(boost::shared_ptr<system> sys)
      { return create_timer(sys); }

      bool delay(unsigned long msec = 1000);
      bool detach_timer(timer *t);
      bool do_event();
      bool do_events();
      bool done();
      static boost::shared_ptr<system> get() { return singleton; }
      lua_State *get_interpreter();
      std::string get_name();
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
      unsigned long get_ticks();
      virtual type_id get_type_id() const { return LEV_TSYSTEM; }
      virtual const char *get_type_name() const { return "lev.system"; }
      static boost::shared_ptr<system> init(lua_State *L);
      bool is_debugging();
      bool is_running();
      bool quit(bool force = false);
      bool quit0() { return quit(); }
      bool run();
      bool set_name(const std::string &name);
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
      boost::shared_ptr<debug_window> start_debug();
      bool stop_debug();
    protected:
      void *_obj;
      static boost::shared_ptr<system> singleton;
  };

};

#endif // _SYSTEM_HPP

