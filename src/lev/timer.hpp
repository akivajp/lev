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
    public:
      typedef boost::shared_ptr<stop_watch> ptr;
    protected:
      stop_watch() : base() { }
    public:
      virtual ~stop_watch() { }
      static stop_watch::ptr create();
      virtual type_id get_type_id() const { return LEV_TSTOP_WATCH; }
      virtual double get_time() = 0;
      virtual bool is_running() const = 0;
      virtual double microseconds() = 0;
      virtual double milliseconds() = 0;
      virtual bool pause() = 0;
      virtual bool resume() = 0;
      double seconds() { return get_time(); }
      virtual bool set_time(double seconds) = 0;
      virtual bool start(double init = 0) = 0;
      bool start0() { return start(0); }
  };

  class timer : public base
  {
    public:
      typedef boost::shared_ptr<timer> ptr;
    protected:
      timer() : base() { }
    public:
      virtual ~timer() { }

      virtual bool close() = 0;
      static boost::shared_ptr<timer> create(double interval = 1);
      virtual double get_interval() const = 0;
      virtual luabind::object get_notify() = 0;
      virtual type_id get_type_id() const { return LEV_TTIMER; }
      virtual bool is_one_shot() const = 0;
      virtual bool is_running() const = 0;
      virtual bool notify() = 0;
      virtual bool probe() = 0;
      virtual bool set_interval(double new_interval) = 0;
      virtual bool set_notify(luabind::object func) = 0;
      virtual bool start(double interval = -1, bool one_shot = false) = 0;
      bool start0() { return start(); }
      bool start1(double interval) { return start(interval); }
      virtual bool stop() = 0;
      virtual timer::ptr to_timer() = 0;
  };

  class clock : public timer
  {
    public:
      typedef boost::shared_ptr<clock> ptr;
    protected:
      clock() : timer() { }
    public:
      virtual ~clock() { }

      static boost::shared_ptr<clock> create(double freq = 50);
      virtual double get_freq() const = 0;
      virtual type_id get_type_id() const { return LEV_TCLOCK; }
      virtual bool set_freq(double freq) = 0;
      virtual bool start(double freq = -1) = 0;
      bool start0() { return start(); }
  };

}

#endif // _TIMER_HPP

