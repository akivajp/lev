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
#include "lev/system.hpp"

// libraries
#include <SDL/SDL.h>

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
      class_<stop_watch, base>("stop_watch")
        .def("pause", &stop_watch::pause)
        .def("resume", &stop_watch::resume)
        .def("start", &stop_watch::start)
        .def("start", &stop_watch::start0)
        .property("time", &stop_watch::get_time, &stop_watch::set_time)
        .scope
        [
          def("create", &stop_watch::create, adopt(result))
        ],
      class_<timer, base>("timer")
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
        .def("stop", &timer::stop),
      class_<lev::clock, timer>("clock")
        .property("fps", &clock::get_freq, &clock::set_freq)
        .property("freq", &clock::get_freq, &clock::set_freq)
        .property("frequency", &clock::get_freq, &clock::set_freq)
        .def("start", &clock::start)
        .def("start", &clock::start0)
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
      myStopWatch(long initial_msec = 0) :
        last_time(SDL_GetTicks()), running(true), ticks(initial_msec)
      { }

      bool Pause()
      {
        running = false;
        return true;
      }

      bool Resume()
      {
        running = true;
        last_time = SDL_GetTicks();
        return true;
      }

      bool SetTime(long ticks)
      {
        this->ticks = ticks;
        return true;
      }

      bool Start(long initial_msec)
      {
        running = true;
        ticks = initial_msec;
        last_time = SDL_GetTicks();
        return true;
      }

      long Time()
      {
        if (running)
        {
          long new_time = SDL_GetTicks();
          ticks += (new_time - last_time);
          last_time = new_time;
        }
        return ticks;
      }

      long last_time;
      bool running;
      long ticks;
  };
  static myStopWatch *cast_watch(void *obj) { return (myStopWatch *)obj; }

  stop_watch::stop_watch() : base(), _obj(NULL) { }

  stop_watch::~stop_watch()
  {
    if (_obj) { delete cast_watch(_obj); }
  }

  stop_watch* stop_watch::create()
  {
    stop_watch *sw = NULL;
    try {
      sw = new stop_watch;
      sw->_obj = new myStopWatch;
      return sw;
    }
    catch(...) {
      delete sw;
      return NULL;
    }
  }

  long stop_watch::get_time()
  {
    return cast_watch(_obj)->Time();
  }

  bool stop_watch::pause()
  {
    return cast_watch(_obj)->Pause();
  }

  bool stop_watch::resume()
  {
    return cast_watch(_obj)->Resume();
  }

  bool stop_watch::set_time(long ticks)
  {
    return cast_watch(_obj)->SetTime(ticks);
  }

  bool stop_watch::start(long initial_msec)
  {
    return cast_watch(_obj)->Start(initial_msec);
  }


//  // prototype of timer callback function
//  static Uint32 timer_callback(Uint32 interval, void *param);

  class myTimer
  {

    public:

      myTimer(system *sys) :
        base_time(0), interval(1000), notify(), one_shot(false),
        running(false), sys(sys) { }

      ~myTimer() { }

      double GetInterval() { return interval; }

      luabind::object GetNotify()
      {
        return notify;
      }

      bool IsOneShot()
      {
        return one_shot;
      }

      bool IsRunning()
      {
        return running;
      }

      bool Notify()
      {
        if (notify && luabind::type(notify) == LUA_TFUNCTION)
        {
          notify();
          return true;
        }
        return false;
      }

      virtual bool Probe()
      {
        if (running)
        {
          if (sys->get_ticks() - base_time > interval)
          {
            Notify();
            base_time = sys->get_ticks();
            if (one_shot) { running = false; }
            return true;
          }
        }
        return false;
      }

      bool SetNotify(luabind::object func)
      {
        notify = func;
        return true;
      }

      bool Start(long new_interval = -1, bool one_shot = false)
      {
        if (new_interval >= 0) { interval = new_interval; }

        base_time = sys->get_ticks();
        this->one_shot = one_shot;
        running = true;
        return true;
      }

      bool Stop()
      {
        running = false;
        return true;
      }

      double base_time, interval;
      bool one_shot;
      bool running;
      luabind::object notify;
      system *sys;
  };

//  static Uint32 timer_callback(Uint32 interval, void *param)
//  {
//    myTimer *t= (myTimer *)param;
//
////printf("CALLBACK!\n");
//    if (t->running)
//    {
//      if (t->notify && luabind::type(t->notify) == LUA_TFUNCTION)
//      {
//        try {
//          luabind::object result = t->notify();
//          if (luabind::type(result) == LUA_TBOOLEAN && result == false)
//          {
//            t->running = false;
//          }
//        }
//        catch (...) {
//          fprintf(stderr, "%s\n", lua_tostring(t->notify.interpreter(), -1));
//        }
//        if (t->one_shot)
//        {
//          t->running = false;
//        }
//      }
//    }
//    return t->interval;
//  }

  static myTimer* cast_timer(void *obj) { return (myTimer *)obj; }

  timer::timer() : _obj(NULL) { }

  timer::~timer()
  {
    if (_obj)
    {
      cast_timer(_obj)->sys->detach_timer(this);
      delete cast_timer(_obj);
    }
  }

  timer* timer::create(system *sys, double interval)
  {
    timer* t = NULL;
    try {
      t = new timer;
      t->_obj = new myTimer(sys);
      t->start(interval, false);
      return t;
    }
    catch (...) {
      delete t;
      return NULL;
    }
  }

  double timer::get_interval() const
  {
    return cast_timer(_obj)->GetInterval();
  }

  luabind::object timer::get_notify()
  {
    return cast_timer(_obj)->GetNotify();
  }

  bool timer::is_one_shot()
  {
    return cast_timer(_obj)->IsOneShot();
  }

  bool timer::is_running()
  {
    return cast_timer(_obj)->IsRunning();
  }

  bool timer::probe()
  {
    return cast_timer(_obj)->Probe();
  }

  bool timer::set_interval(double new_interval)
  {
    if (new_interval < 0) { return false; }
    cast_timer(_obj)->interval = new_interval;
    return true;
  }

  bool timer::set_notify(luabind::object func)
  {
    return cast_timer(_obj)->SetNotify(func);
  }

  bool timer::start(int milliseconds, bool one_shot)
  {
    return cast_timer(_obj)->Start(milliseconds, one_shot);
  }

  bool timer::stop()
  {
    cast_timer(_obj)->Stop();
    return true;
  }


  class myClock : public myTimer
  {
    public:

      myClock(system *sys) : myTimer(sys) { }

      virtual ~myClock() { }

      virtual bool Probe()
      {
        if (running)
        {
          if (sys->get_ticks() - base_time > interval)
          {
            base_time = base_time + interval;
            Notify();
            if (one_shot) { running = false; }
            return true;
          }
        }
        return false;
      }
  };
  static myClock *cast_clk(void *obj) { return (myClock *)obj; }

  clock::clock() : timer() { }

  clock::~clock()
  {
    if (_obj)
    {
      cast_clk(_obj)->sys->detach_timer(this);
      delete cast_clk(_obj);
      _obj = NULL;
    }
  }

  clock* clock::create(system *sys, double freq)
  {
    clock *c = NULL;
    try {
      c = new clock;
      c->_obj = new myClock(sys);
      c->start(freq);
      return c;
    }
    catch (...) {
      delete c;
      return NULL;
    }
  }

  double clock::get_freq() const
  {
    return 1000 / get_interval();
  }

  bool clock::probe()
  {
    return cast_clk(_obj)->Probe();
  }

  bool clock::set_freq(double freq)
  {
    return set_interval(1000 / freq);
  }

  bool clock::start(double freq)
  {
    if (freq <= 0)
    {
      return cast_clk(_obj)->Start(-1, false);
    }
    return cast_clk(_obj)->Start(1000 / freq, false);
  }

}

