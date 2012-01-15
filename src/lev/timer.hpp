#ifndef _TIMER_HPP
#define _TIMER_HPP

/////////////////////////////////////////////////////////////////////////////
// Name:        src/lev/timer.hpp
// Purpose:     header for timer handling classes
// Author:      Akiva Miura <akiva.miura@gmail.com>
// Modified by:
// Created:     08/26/2011
// Copyright:   (C) 2010-2012 Akiva Miura
// Licence:     MIT License
/////////////////////////////////////////////////////////////////////////////

#include "base.hpp"
#include <luabind/luabind.hpp>

extern "C"
{
  int luaopen_lev_timer(lua_State *L);
}

namespace lev
{

  class stop_watch : public base
  {
    protected:
      stop_watch();
    public:
      virtual ~stop_watch();
      static stop_watch* create();
      virtual type_id get_type_id() const { return LEV_TSTOP_WATCH; }
      virtual const char *get_type_name() const { return "lev.stop_watch"; }
      long get_time();
      bool pause();
      bool resume();
      bool set_time(long ticks);
      bool start(long initial_msec = 0);
      bool start0() { return start(0); }
    protected:
      void *_obj;
  };

  class timer : public base
  {
    protected:
      timer();
    public:
      virtual ~timer();
      static timer* create();
      int get_interval();
      luabind::object get_notify();
      virtual type_id get_type_id() const { return LEV_TTIMER; }
      virtual const char *get_type_name() const { return "lev.timer"; }
      bool is_one_shot();
      bool is_running();
      bool set_notify(luabind::object func);
      bool start(int milliseconds = -1, bool one_shot = false);
      bool start0() { return start(); }
      bool start1(int milliseconds) { return start(milliseconds); }
      bool stop();
    protected:
      void *_obj;
  };

}

#endif // _TIMER_HPP

