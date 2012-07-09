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

  class input
  {
    public:
      static const char *to_keyname(long code);
  };

  class event : public base
  {
    public:
      typedef boost::shared_ptr<event> ptr;
    protected:
      event() { }
    public:
      virtual ~event() { }
      virtual event::ptr clone() = 0;
      virtual std::string get_button() const = 0;
      virtual int get_dx() const = 0;
      virtual int get_dy() const = 0;
      virtual int get_id() const = 0;
      virtual std::string get_key() const = 0;
      virtual long get_key_code() const = 0;
      virtual long get_scancode() const = 0;
      virtual double get_timestamp() const = 0;
      virtual type_id get_type_id() const { return LEV_TEVENT; }
      virtual int get_x() const = 0;
      virtual int get_y() const = 0;
      virtual bool is_pressed() const = 0;
      virtual bool is_released() const = 0;
      virtual bool left_is_down() const = 0;
      virtual bool middle_is_down() const = 0;
      virtual bool right_is_down() const = 0;
  };

  class system : public base
  {
    public:
      typedef boost::shared_ptr<system> ptr;
    protected:
      system() : base() { }
    public:
      virtual ~system() { }

      // attach methods
      virtual bool attach(boost::shared_ptr<class debugger> d) = 0;
      virtual bool attach(boost::shared_ptr<class screen> s) = 0;
      virtual bool attach(boost::shared_ptr<class timer> t) = 0;

      // delay methods
      virtual bool delay(unsigned long msec = 1000) = 0;
      bool delay0() { return delay(); }

      // do event methods
      virtual bool do_event() = 0;
      virtual bool do_events() = 0;

      virtual bool done() = 0;

      static system::ptr get();
      virtual boost::shared_ptr<class debugger> get_debugger() = 0;

      static lua_State *get_interpreter();

      virtual std::string get_name() const = 0;
      virtual luabind::object get_on_button_down() = 0;
      virtual luabind::object get_on_button_up() = 0;
      virtual luabind::object get_on_idle() = 0;
      virtual luabind::object get_on_key_down() = 0;
      virtual luabind::object get_on_key_up() = 0;
      virtual luabind::object get_on_left_down() = 0;
      virtual luabind::object get_on_left_up() = 0;
      virtual luabind::object get_on_middle_down() = 0;
      virtual luabind::object get_on_middle_up() = 0;
      virtual luabind::object get_on_motion() = 0;
      virtual luabind::object get_on_quit() = 0;
      virtual luabind::object get_on_right_down() = 0;
      virtual luabind::object get_on_right_up() = 0;
      virtual double get_elapsed() const = 0;
      virtual type_id get_type_id() const { return LEV_TSYSTEM; }

      static boost::shared_ptr<system> init(lua_State *L);

      virtual bool is_debugging() const = 0;
      virtual bool is_running() const = 0;
      virtual bool quit(bool force = false) = 0;
      bool quit0() { return quit(); }
      virtual bool run() = 0;
      virtual bool set_name(const std::string &name) = 0;
      virtual bool set_on_button_down(luabind::object func) = 0;
      virtual bool set_on_button_up(luabind::object func) = 0;
      virtual bool set_on_idle(luabind::object func) = 0;
      virtual bool set_on_key_down(luabind::object func) = 0;
      virtual bool set_on_key_up(luabind::object func) = 0;
      virtual bool set_on_left_down(luabind::object func) = 0;
      virtual bool set_on_left_up(luabind::object func) = 0;
      virtual bool set_on_middle_down(luabind::object func) = 0;
      virtual bool set_on_middle_up(luabind::object func) = 0;
      virtual bool set_on_motion(luabind::object func) = 0;
      virtual bool set_on_quit(luabind::object func) = 0;
      virtual bool set_on_right_down(luabind::object func) = 0;
      virtual bool set_on_right_up(luabind::object func) = 0;
      virtual bool set_running(bool run = true) = 0;
      virtual boost::shared_ptr<class debugger> start_debug() = 0;
      virtual bool stop_debug() = 0;
  };

};

#endif // _SYSTEM_HPP

