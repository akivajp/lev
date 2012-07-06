/////////////////////////////////////////////////////////////////////////////
// Name:        src/timer.cpp
// Purpose:     source for timer handling classes
// Author:      Akiva Miura <akiva.miura@gmail.com>
// Modified by:
// Created:     08/26/2011
// Copyright:   (C) 2010-2011 Akiva Miura
// Licence:     MIT License
/////////////////////////////////////////////////////////////////////////////

// pre-compiled header
#include "prec.h"

// declarations
#include "lev/timer.hpp"

// dependencies
#include "lev/debug.hpp"
#include "lev/system.hpp"

// libraries
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/shared_ptr.hpp>
#include <SDL2/SDL.h>

int luaopen_lev_timer(lua_State *L)
{
  using namespace lev;
  using namespace luabind;

  open(L);
  // beginning to load
  globals(L)["package"]["loaded"]["lev.timer"] = true;
  // dependencies
  globals(L)["require"]("lev.base");
  globals(L)["require"]("lev.system");

  module(L, "lev")
  [
    namespace_("classes")
    [
      class_<stop_watch, base, boost::shared_ptr<base> >("stop_watch")
        .property("elapsed", &stop_watch::get_time, &stop_watch::set_time)
        .property("is_running", &stop_watch::is_running)
        .property("micro", &stop_watch::microseconds)
        .property("microseconds", &stop_watch::microseconds)
        .property("milli", &stop_watch::milliseconds)
        .property("milliseconds", &stop_watch::milliseconds)
        .def("pause", &stop_watch::pause)
        .def("resume", &stop_watch::resume)
        .property("seconds", &stop_watch::seconds)
        .def("start", &stop_watch::start)
        .def("start", &stop_watch::start0)
        .property("time", &stop_watch::get_time, &stop_watch::set_time)
        .scope
        [
          def("create", &stop_watch::create)
        ],
      class_<timer, base, boost::shared_ptr<base> >("timer")
        .def("close", &timer::close)
        .property("is_one_shot", &timer::is_one_shot)
        .property("is_running", &timer::is_running)
        .property("interval", &timer::get_interval, &timer::set_interval)
        .property("notify", &timer::get_notify, &timer::set_notify)
        .property("on_notify", &timer::get_notify, &timer::set_notify)
        .property("on_tick", &timer::get_notify, &timer::set_notify)
        .def("probe", &timer::probe)
        .def("start", &timer::start)
        .def("start", &timer::start0)
        .def("start", &timer::start1)
        .def("stop", &timer::stop)
        .scope
        [
          def("create", &timer::create)
        ],
      class_<lev::clock, timer, boost::shared_ptr<base> >("clock")
        .property("fps", &clock::get_freq, &clock::set_freq)
        .property("freq", &clock::get_freq, &clock::set_freq)
        .property("frequency", &clock::get_freq, &clock::set_freq)
        .def("start", &clock::start)
        .def("start", &clock::start0)
        .scope
        [
          def("create", &clock::create)
        ]
    ]
  ];
  object lev = globals(L)["lev"];

  lev["stop_watch"] = lev["classes"]["stop_watch"]["create"];
  lev["timer"] = lev["classes"]["timer"]["create"];
  lev["clock"] = lev["classes"]["clock"]["create"];

  globals(L)["package"]["loaded"]["lev.timer"] = true;
  return 0;
}

namespace lev
{

  class myStopWatch
  {
    public:
      myStopWatch(long initial_sec = 0) :
        last_time(boost::posix_time::microsec_clock::local_time()),
        running(true), ticks(initial_sec * 1000000)
      { }

      double Microseconds()
      {
        Update();
        return double(ticks);
      }

      double Milliseconds()
      {
        Update();
        return double(ticks) / 1000;
      }

      bool Pause()
      {
        Update();
        running = false;
        return true;
      }

      bool Resume()
      {
        last_time = boost::posix_time::microsec_clock::local_time();
        running = true;
        return true;
      }

      bool SetTime(double seconds)
      {
        this->ticks = seconds * 1000000;
        return true;
      }

      bool Start(double initial_sec)
      {
        running = true;
        ticks = initial_sec * 1000000;
        last_time = boost::posix_time::microsec_clock::local_time();
        return true;
      }

      double Time()
      {
        Update();
        return double(ticks) / 1000000;
      }

      bool Update()
      {
        if (running)
        {
          boost::posix_time::ptime new_time = boost::posix_time::microsec_clock::local_time();
          boost::posix_time::time_duration d = new_time - last_time;
          ticks += d.total_microseconds();
          last_time = new_time;
          return true;
        }
        return false;
      }

//      long last_time;
      boost::posix_time::ptime last_time;
      bool running;
      long ticks;
  };
  static myStopWatch *cast_watch(void *obj) { return (myStopWatch *)obj; }

  stop_watch::stop_watch() : base(), _obj(NULL) { }

  stop_watch::~stop_watch()
  {
    if (_obj) { delete cast_watch(_obj); }
  }

  boost::shared_ptr<stop_watch> stop_watch::create()
  {
    boost::shared_ptr<stop_watch> sw;
    try {
      sw.reset(new stop_watch);
      if (! sw) { throw -1; }
      sw->_obj = new myStopWatch;
      if (! sw->_obj) { throw -2; }
    }
    catch(...) {
      sw.reset();
      lev::debug_print("error on stop watch instance creation");
    }
    return sw;
  }

  double stop_watch::get_time()
  {
    return cast_watch(_obj)->Time();
  }

  bool stop_watch::is_running()
  {
    return cast_watch(_obj)->running;
  }

  double stop_watch::microseconds()
  {
    return cast_watch(_obj)->Microseconds();
  }

  double stop_watch::milliseconds()
  {
    return cast_watch(_obj)->Milliseconds();
  }

  bool stop_watch::pause()
  {
    return cast_watch(_obj)->Pause();
  }

  bool stop_watch::resume()
  {
    return cast_watch(_obj)->Resume();
  }

  bool stop_watch::set_time(double seconds)
  {
    return cast_watch(_obj)->SetTime(seconds);
  }

  bool stop_watch::start(double initial_sec)
  {
    return cast_watch(_obj)->Start(initial_sec);
  }


//  // prototype of timer callback function
//  static Uint32 timer_callback(Uint32 interval, void *param);

  template <typename T>
  class impl_timer : public T
  {
    public:
      typedef boost::shared_ptr<impl_timer> ptr;
    protected:
      impl_timer() :
        T(),
        base_time(0), interval(1000), func_notify(),
        one_shot(false),
        running(false), wptr()
      { }
    public:
      virtual ~impl_timer() { }

      virtual bool close()
      {
        return false;
      }

      static impl_timer::ptr create(double interval = 1000)
      {
        impl_timer::ptr t;
        system::ptr sys = system::get();
        if (! sys) { return t; }
        try {
          t.reset(new impl_timer);
          if (! t) { throw -1; }
          t->wptr = t;
          t->start(interval, false);
          if (! sys->attach(t->to_timer())) { throw -2; }
        }
        catch (...) {
          t.reset();
          lev::debug_print("error on timer class creation");
        }
        return t;
      }

      virtual double get_interval() const
      {
        return interval;
      }

      virtual luabind::object get_notify()
      {
        return func_notify;
      }

      virtual bool is_one_shot() const
      {
        return one_shot;
      }

      virtual bool is_running() const
      {
        return running;
      }

      virtual bool notify()
      {
        if (func_notify && luabind::type(func_notify) == LUA_TFUNCTION)
        {
          func_notify();
          return true;
        }
        return false;
      }

      virtual bool probe()
      {
        if (! running) { return false; }
        system::ptr sys = system::get();
        if (! sys) { return false; }
        if (sys->get_ticks() - base_time > interval)
        {
          notify();
          base_time = sys->get_ticks();
          if (one_shot) { running = false; }
          return true;
        }
        return false;
      }

      virtual bool set_interval(double new_interval)
      {
        if (new_interval < 0) { return false; }
        interval = new_interval;
        return true;
      }

      virtual bool set_notify(luabind::object func)
      {
        func_notify = func;
        return true;
      }

      virtual bool start(double milliseconds = -1, bool one_shot = false)
      {
        if (milliseconds > 0) { interval = milliseconds; }
        system::ptr sys = system::get();
        if (! sys) { return false; }

        base_time = sys->get_ticks();
        this->one_shot = one_shot;
        running = true;
        return true;
      }

      virtual bool stop()
      {
        running = false;
        return true;
      }

      virtual timer::ptr to_timer()
      {
        return timer::ptr(wptr);
      }

      double base_time, interval;
      bool one_shot;
      bool running;
      luabind::object func_notify;
      boost::weak_ptr<timer> wptr;
      boost::weak_ptr<system> sys;
  };

  timer::ptr timer::create(double interval)
  {
    return impl_timer<timer>::create(interval);
  }

  class impl_clock : public impl_timer<clock>
  {
    public:
      typedef boost::shared_ptr<impl_clock> ptr;
    protected:
      impl_clock() : impl_timer<clock>() { }
    public:
      virtual ~impl_clock() { }

      static impl_clock::ptr create(double freq)
      {
        impl_clock::ptr c;
        system::ptr sys = system::get();
        if (! sys) { return c; }
        try {
          c.reset(new impl_clock);
          if (! c) { throw -1; }
          c->wptr = c;
          c->start(freq);
          if (! sys->attach(c->to_timer())) { throw -1; }
        }
        catch (...) {
          c.reset();
          lev::debug_print("error on clock instance creation");
        }
        return c;
      }

      virtual double get_freq() const
      {
        return 1000 / get_interval();
      }

      virtual bool probe()
      {
        if (! running) { return false; }
        system::ptr sys = system::get();
        if (! sys) { return false; }
        if (sys->get_ticks() - base_time > interval)
        {
          base_time = base_time + interval;
          notify();
          if (one_shot) { running = false; }
          return true;
        }
      }

      virtual bool set_freq(double freq)
      {
        return set_interval(1000 / freq);
      }

      virtual bool start(double freq)
      {
        if (freq <= 0)
        {
          return impl_timer<clock>::start(-1, false);
        }
        return impl_timer<clock>::start(1000 / freq, false);
      }

  };

  clock::ptr clock::create(double freq)
  {
    return impl_clock::create(freq);
  }

}

