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
// libraries
#include <SDL/SDL.h>

int luaopen_lev_timer(lua_State *L)
{
  using namespace lev;
  using namespace luabind;

  open(L);
  // beginning to load
  globals(L)["package"]["loaded"]["lev.timer"] = true;
  globals(L)["require"]("lev.base");

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
        .def("start", &timer::start)
        .def("start", &timer::start0)
        .def("start", &timer::start1)
        .def("stop", &timer::stop)
        .property("is_one_shot", &timer::is_one_shot)
        .property("is_running", &timer::is_running)
        .property("interval", &timer::get_interval)
        .property("notify", &timer::get_notify, &timer::set_notify)
        .property("on_notify", &timer::get_notify, &timer::set_notify)
        .property("on_tick", &timer::get_notify, &timer::set_notify)
        .scope
        [
          def("create", &timer::create, adopt(result))
        ]
    ]
  ];
  object lev = globals(L)["lev"];

  lev["stop_watch"] = lev["classes"]["stop_watch"]["create"];
  lev["timer"] = lev["classes"]["timer"]["create"];

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


  // prototype of timer callback function
  static Uint32 timer_callback(Uint32 interval, void *param);

  class myTimer
  {

    public:

      myTimer() : id((SDL_TimerID)NULL), interval(1000), notify(), one_shot(false), running(false) { }

      ~myTimer()
      {
        if (id)
        {
          SDL_RemoveTimer(id);
        }
      }

      long GetInterval() { return interval; }

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

      void Notify()
      {
        if (notify && luabind::type(notify) == LUA_TFUNCTION) { notify(); }
      }

      bool SetNotify(luabind::object func)
      {
        notify = func;
        return true;
      }

      bool Start(long new_interval = -1, bool one_shot = false)
      {
        if (new_interval > 0) { interval = new_interval; }

        if (! id)
        {
          id = SDL_AddTimer(interval, timer_callback, this);
          if (! id) { return false; }
        }

        this->interval = interval;
        this->one_shot = one_shot;
        running = true;
        return true;
      }

      bool Stop()
      {
        running = false;
        return true;
      }

      SDL_TimerID id;
      long interval;
      bool one_shot;
      bool running;
      luabind::object notify;
  };

  static Uint32 timer_callback(Uint32 interval, void *param)
  {
    myTimer *t= (myTimer *)param;

//printf("CALLBACK!\n");
    if (t->running)
    {
      if (t->notify && luabind::type(t->notify) == LUA_TFUNCTION)
      {
        try {
          luabind::object result = t->notify();
          if (luabind::type(result) == LUA_TBOOLEAN && result == false)
          {
            t->running = false;
          }
        }
        catch (...) {
          fprintf(stderr, "%s\n", lua_tostring(t->notify.interpreter(), -1));
        }
        if (t->one_shot)
        {
          t->running = false;
        }
      }
    }
    return t->interval;
  }

  static myTimer* cast_timer(void *obj) { return (myTimer *)obj; }

  timer::timer() : _obj(NULL) { }

  timer::~timer()
  {
    if (_obj)
    {
      delete cast_timer(_obj);
    }
  }

  timer* timer::create()
  {
    timer* t = NULL;
    myTimer *obj = NULL;
    try {
      t = new timer;
      t->_obj = obj = new myTimer;
      return t;
    }
    catch (...) {
      delete t;
      return NULL;
    }
  }

  int timer::get_interval()
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

}

