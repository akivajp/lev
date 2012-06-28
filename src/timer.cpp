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
//#include <boost/date_time/posix_time/posix_time.hpp>
#include <allegro5/allegro.h>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

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
        .def("restart", &stop_watch::start)
        .def("restart", &stop_watch::start0)
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
        .property("count", &timer::get_count, &timer::set_count)
        .property("fps", &timer::get_freq, &timer::set_freq)
        .property("freq", &timer::get_freq, &timer::set_freq)
        .property("frequency", &timer::get_freq, &timer::set_freq)
        .property("is_one_shot", &timer::is_one_shot)
        .property("is_running", &timer::is_running)
        .property("interval", &timer::get_interval, &timer::set_interval)
        .property("notify", &timer::get_on_tick, &timer::set_on_tick)
        .property("on_notify", &timer::get_on_tick, &timer::set_on_tick)
        .property("on_tick", &timer::get_on_tick, &timer::set_on_tick)
        .def("start", &timer::start)
        .def("start", &timer::start0)
        .def("start", &timer::start1)
        .def("stop", &timer::stop)
    ]
  ];
  object lev = globals(L)["lev"];

  lev["stop_watch"] = lev["classes"]["stop_watch"]["create"];

  globals(L)["package"]["loaded"]["lev.timer"] = true;
  return 0;
}

namespace lev
{

  class myStopWatch
  {
    public:
      myStopWatch(long initial_seconds = 0) :
        last_time(al_get_time()),
        running(true), total(initial_seconds)
      { }

      double Microseconds()
      {
        Update();
        return Time() * 1000000;
      }

      double Milliseconds()
      {
        Update();
        return Time() * 1000;
      }

      bool Pause()
      {
        Update();
        running = false;
        return true;
      }

      bool Resume()
      {
        if (! running)
        {
          last_time = al_get_time();
          running = true;
        }
        return false;
      }

      bool SetTime(double seconds)
      {
        total = seconds;
        return true;
      }

      bool Start(double initial_seconds)
      {
        running = true;
        total = initial_seconds;
        last_time = al_get_time();
        return true;
      }

      double Time()
      {
        Update();
        return total;
      }

      bool Update()
      {
        if (running)
        {
          double new_time = al_get_time();
          double duration = new_time - last_time;
          total += duration;
          last_time = new_time;
          return true;
        }
        return false;
      }

      double last_time, total;
      bool running;
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

  class myTimer
  {

    public:

      myTimer(bool one_shot) :
        t(NULL), on_tick(), one_shot(one_shot), running(false) { }

      ~myTimer()
      {
        if (t && system::get_interpreter())
        {
          al_destroy_timer(t);
          t = NULL;
        }
      }

      static myTimer *Create(double interval_seconds = 1, bool one_shot = false)
      {
        myTimer *timer = NULL;
        try {
          timer = new myTimer(one_shot);
          if (! timer) { throw -1; }
          timer->t = al_create_timer(interval_seconds);
          if (! timer->t) { throw -2; }
          return timer;
        }
        catch (...) {
          delete timer;
          return NULL;
        }
      }

      bool Start(double interval_seconds, bool one_shot)
      {
        if (interval_seconds > 0) { al_set_timer_speed(t, interval_seconds); }
        al_start_timer(t);
        this->one_shot = one_shot;
        running = true;
        return true;
      };

      bool Stop()
      {
        al_stop_timer(t);
        running = false;
        return true;
      }

      ALLEGRO_TIMER *t;
      luabind::object on_tick;
      bool one_shot, running;
  };
  static myTimer* cast_timer(void *obj) { return (myTimer *)obj; }

  timer::timer() : _obj(NULL) { }

  timer::~timer()
  {
    if (_obj)
    {
      delete cast_timer(_obj);
      _obj = NULL;
    }
  }

  boost::shared_ptr<timer> timer::create(double interval, bool one_shot)
  {
    boost::shared_ptr<timer> t;
    try {
      t.reset(new timer);
      if (! t) { throw -1; }
      t->_obj = myTimer::Create(interval, one_shot);
      if (! t->_obj) { throw -2; }
    }
    catch (...) {
      t.reset();
      lev::debug_print("error on timer class creation");
    }
    return t;
  }

  long timer::get_count() const
  {
    return al_get_timer_count(cast_timer(_obj)->t);
  }

  long timer::get_id() const
  {
    return (long)cast_timer(_obj)->t;
  }

  double timer::get_freq() const
  {
    return 1 / get_interval();
  }

  double timer::get_interval() const
  {
    return al_get_timer_speed(cast_timer(_obj)->t);
  }

  bool timer::is_one_shot() const
  {
    return cast_timer(_obj)->one_shot;
  }

  luabind::object timer::get_on_tick()
  {
    return cast_timer(_obj)->on_tick;
  }

  bool timer::is_running() const
  {
    return cast_timer(_obj)->running;
  }

  bool timer::set_count(long count)
  {
    al_set_timer_count(cast_timer(_obj)->t, count);
    return true;
  }

  bool timer::set_freq(double freq)
  {
    return set_interval(1 / freq);
  }

  bool timer::set_interval(double new_interval)
  {
//    if (new_interval < 0) { return false; }
    al_set_timer_speed(cast_timer(_obj)->t, new_interval);
    return true;
  }

  bool timer::set_on_tick(luabind::object func)
  {
    cast_timer(_obj)->on_tick = func;
    return true;
  }

  bool timer::start(double interval_seconds, bool one_shot)
  {
    return cast_timer(_obj)->Start(interval_seconds, one_shot);
  }

  bool timer::stop()
  {
    cast_timer(_obj)->Stop();
    return true;
  }

}

