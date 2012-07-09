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
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
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

  // stop watch implementation
  class impl_stop_watch : public stop_watch
  {
    public:
      typedef boost::shared_ptr<impl_stop_watch> ptr;
    protected:
      impl_stop_watch() :
        stop_watch(),
        running(true),
        total(0)
      { }
    public:
      virtual ~impl_stop_watch()
      {
      }

      static impl_stop_watch::ptr create()
      {
        impl_stop_watch::ptr sw;
        try {
          sw.reset(new impl_stop_watch);
          if (! sw) { throw -1; }
          sw->last = double(SDL_GetPerformanceCounter()) / SDL_GetPerformanceFrequency();
        }
        catch (...) {
          sw.reset();
          lev::debug_print("error on stop watch instance creation");
        }
        return sw;
      }

      virtual double get_time()
      {
        update();
        return total;
      }

      virtual bool is_running() const
      {
        return running;
      }

      virtual double microseconds()
      {
        update();
        return total * 1000000;
      }

      virtual double milliseconds()
      {
        update();
        return total * 1000;
      }

      virtual bool pause()
      {
        update();
        running = false;
        return true;
      }

      virtual bool resume()
      {
        if (running) { return false; }
        last = double(SDL_GetPerformanceCounter()) / SDL_GetPerformanceFrequency();
        running = true;
        return true;
      }

      virtual bool set_time(double seconds)
      {
        total = seconds;
        last = double(SDL_GetPerformanceCounter()) / SDL_GetPerformanceFrequency();
        return true;
      }

      virtual bool start(double init = 0)
      {
        running = true;
        total = init;
        last = double(SDL_GetPerformanceCounter()) / SDL_GetPerformanceFrequency();
        return true;
      }

      bool update()
      {
        if (running)
        {
          double new_time = double(SDL_GetPerformanceCounter()) / SDL_GetPerformanceFrequency();
          double d = new_time - last;
          last = new_time;
          if (d > 0)
          {
            total += d;
          }
          return true;
        }
        return false;
      }

      double total, last;
      bool running;
  };

  stop_watch::ptr stop_watch::create()
  {
    return impl_stop_watch::create();
  }


  // timer class implementation
  template <typename T>
  class impl_timer : public T
  {
    public:
      typedef boost::shared_ptr<impl_timer> ptr;
    protected:
      impl_timer() :
        T(),
        base_time(0), func_notify(),
        one_shot(false),
        running(false), wptr()
      { }
    public:
      virtual ~impl_timer() { }

      virtual bool close()
      {
        return false;
      }

      static impl_timer::ptr create(double interval = 1)
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
        if (sys->get_elapsed() - base_time >= interval)
        {
          notify();
          base_time = sys->get_elapsed();
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

      virtual bool start(double new_interval = -1, bool new_one_shot = false)
      {
        if (new_interval > 0) { interval = new_interval; }
        system::ptr sys = system::get();
        if (! sys) { return false; }

        base_time = sys->get_elapsed();
        one_shot = new_one_shot;
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

  // clock class implementation
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
        return 1.0 / get_interval();
      }

      virtual bool probe()
      {
        if (! running) { return false; }
        system::ptr sys = system::get();
        if (! sys) { return false; }
        if (sys->get_elapsed() - base_time > interval)
        {
          base_time = base_time + interval;
          notify();
          if (one_shot) { running = false; }
          return true;
        }
      }

      virtual bool set_freq(double freq)
      {
        return set_interval(1.0 / freq);
      }

      virtual bool start(double freq)
      {
        if (freq <= 0)
        {
          return impl_timer<clock>::start(-1, false);
        }
        return impl_timer<clock>::start(1.0 / freq, false);
      }

  };

  clock::ptr clock::create(double freq)
  {
    return impl_clock::create(freq);
  }

}

