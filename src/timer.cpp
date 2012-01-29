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
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/shared_ptr.hpp>
//#include <SDL/SDL.h>

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
      class_<lev::clock, timer, boost::shared_ptr<base> >("clock")
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
      fprintf(stderr, "error on stop watch instance creation\n");
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

      myTimer(boost::shared_ptr<system> sys) :
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
          if (boost::shared_ptr<system> s = sys.lock())
          {
            if (s->get_ticks() - base_time > interval)
            {
              Notify();
              base_time = s->get_ticks();
              if (one_shot) { running = false; }
              return true;
            }
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

        if (boost::shared_ptr<system> s = sys.lock())
        {
          base_time = s->get_ticks();
          this->one_shot = one_shot;
          running = true;
          return true;
        }
        return false;
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
      boost::weak_ptr<system> sys;
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
      if (boost::shared_ptr<system> s = cast_timer(_obj)->sys.lock())
      {
        s->detach_timer(this);
      }
      delete cast_timer(_obj);
    }
  }

  boost::shared_ptr<timer> timer::create(boost::shared_ptr<system> sys, double interval)
  {
    boost::shared_ptr<timer> t;
    if (! sys) { return t; }
    try {
      t.reset(new timer);
      if (! t) { throw -1; }
      t->_obj = new myTimer(sys);
      if (! t->_obj) { throw -2; }
      t->start(interval, false);
    }
    catch (...) {
      t.reset();
      fprintf(stderr, "error on timer class creation\n");
    }
    return t;
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

      myClock(boost::shared_ptr<system> sys) : myTimer(sys) { }

      virtual ~myClock() { }

      virtual bool Probe()
      {
        if (running)
        {
          if (boost::shared_ptr<system> s = sys.lock())
          {
            if (s->get_ticks() - base_time > interval)
            {
              base_time = base_time + interval;
              Notify();
              if (one_shot) { running = false; }
              return true;
            }
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
      if (boost::shared_ptr<system> s = cast_clk(_obj)->sys.lock())
      {
        s->detach_timer(this);
      }
      delete cast_clk(_obj);
      _obj = NULL;
    }
  }

  boost::shared_ptr<clock> clock::create(boost::shared_ptr<system> sys, double freq)
  {
    boost::shared_ptr<clock> c;
    if (! sys) { return c; }
    try {
      c.reset(new clock);
      if (! c) { throw -1; }
      c->_obj = new myClock(sys);
      if (! c->_obj) { throw -2; }
      c->start(freq);
    }
    catch (...) {
      c.reset();
      fprintf(stderr, "error on clock instance creation\n");
    }
    return c;
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

