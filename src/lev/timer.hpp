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

  // class dependencies
  class system;

  class stop_watch : public base
  {
    protected:
      stop_watch();
    public:
      virtual ~stop_watch();
      static boost::shared_ptr<stop_watch> create();
      virtual type_id get_type_id() const { return LEV_TSTOP_WATCH; }
      double get_time();
      bool is_running();
      double microseconds();
      double milliseconds();
      bool pause();
      bool resume();
      double seconds() { return get_time(); }
      bool set_time(double seconds);
      bool start(double initial_msec = 0);
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
      static boost::shared_ptr<timer>
        create(double interval_seconds = 1, bool one_shot = false);
      double get_freq() const;
      double get_interval() const;
      long get_count() const;
      long get_id() const;
      luabind::object get_on_tick();
      virtual type_id get_type_id() const { return LEV_TTIMER; }
      bool is_one_shot() const;
      bool is_running() const;
      bool set_count(long count);
      bool set_freq(double freq);
      bool set_interval(double new_interval);
      bool set_on_tick(luabind::object func);
      virtual bool start(double interval_seconds = -1, bool one_shot = false);
      virtual bool start0() { return start(); }
      virtual bool start1(double interval) { return start(interval); }
      bool stop();
    protected:
      void *_obj;
  };

}

#endif // _TIMER_HPP

