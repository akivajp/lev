/////////////////////////////////////////////////////////////////////////////
// Name:        src/system.cpp
// Purpose:     source for system managing class
// Author:      Akiva Miura <akiva.miura@gmail.com>
// Modified by:
// Created:     13/10/2011
// Copyright:   (C) 2010-2011 Akiva Miura
// Licence:     MIT License
/////////////////////////////////////////////////////////////////////////////

// pre-compiled header
#include "prec.h"

// declarations
#include "lev/system.hpp"

// dependencies
#include "lev/debug.hpp"
#include "lev/font.hpp"
#include "lev/entry.hpp"
#include "lev/draw.hpp"
#include "lev/sound.hpp"
#include "lev/timer.hpp"
#include "lev/screen.hpp"
#include "lev/util.hpp"

// libraries
#include <allegro5/allegro.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_physfs.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_ttf.h>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <luabind/luabind.hpp>
#include <luabind/raw_policy.hpp>
#include <map>
#include <physfs.h>
#include <vector>

int luaopen_lev_system(lua_State *L)
{
  using namespace luabind;
  using namespace lev;

  open(L);
  // beginning to load
  globals(L)["package"]["loaded"]["lev.system"] = true;
  // pre-requirement
  globals(L)["require"]("lev.base");
  globals(L)["require"]("lev.debug");
  globals(L)["require"]("lev.image");
  globals(L)["require"]("lev.input");
  globals(L)["require"]("lev.screen");
  globals(L)["require"]("lev.sound");
  globals(L)["require"]("lev.timer");

  module(L, "lev")
  [
    namespace_("classes")
    [
      class_<event, base, boost::shared_ptr<base> >("event")
        .property("button", &event::get_button)
        .property("count", &event::get_count)
        .property("dw", &event::get_dw)
        .property("dx", &event::get_dx)
        .property("dy", &event::get_dy)
        .property("dz", &event::get_dz)
        .property("id", &event::get_id)
        .property("is_pressed", &event::is_pressed)
        .property("is_released", &event::is_released)
        .property("key", &event::get_key)
        .property("keycode", &event::get_key_code)
        .property("key_code", &event::get_key_code)
        .property("left_is_down", &event::left_is_down)
        .property("middle_is_down", &event::middle_is_down)
        .property("pressed", &event::is_pressed)
        .property("released", &event::is_released)
        .property("right_is_down", &event::right_is_down)
        .property("w", &event::get_w)
        .property("x", &event::get_x)
        .property("y", &event::get_y)
        .property("z", &event::get_z),
      class_<lev::system, base, boost::shared_ptr<base> >("system")
        .def("close", &system::done)
        .def("create_mixer", &system::create_mixer)
        .def("create_timer", &system::create_timer)
        .def("create_timer", &system::create_timer0)
        .def("create_timer", &system::create_timer1)
        .def("debug", &system::start_debug)
        .def("delay", &system::delay)
        .def("do_event", &system::do_event)
        .def("do_events", &system::do_events)
        .def("done", &system::done)
        .property("elapsed", &system::get_elapsed_time)
        .property("elapsed_time", &system::get_elapsed_time)
        .property("input", &system::get_input)
        .property("is_debugging", &system::is_debugging)
        .property("is_running", &system::is_running, &system::set_running)
        .def("mixer", &system::create_mixer)
        .property("name", &system::get_name, &system::set_name)
        .property("on_quit", &system::get_on_quit, &system::set_on_quit)
        .property("on_tick", &system::get_on_tick, &system::set_on_tick)
        .def("quit", &system::quit)
        .def("quit", &system::quit0)
        .def("rest", &system::delay)
        .def("run", &system::run)
        .def("run", &system::run0)
        .def("set_running", &system::set_running)
        .def("start_debug", &system::start_debug)
        .def("stop_debug", &system::stop_debug)
        .property("ticks", &system::get_elapsed_time)
        .property("time", &system::get_elapsed_time)
        .def("timer", &system::create_timer)
        .def("timer", &system::create_timer0)
        .def("timer", &system::create_timer1)
        .scope
        [
          def("create_screen_c", &system::create_screen),
//          def("get_c", &system::get),
//          def("get", &system::get),
          def("init", &system::init)
        ]
    ]
  ];
  object lev = globals(L)["lev"];
  object classes = lev["classes"];
  lev::register_to(classes["system"], "create_screen", &system::create_screen_l);
  lev::register_to(classes["system"], "screen", &system::create_screen_l);
  lev::register_to(classes["system"], "window", &system::create_screen_l);
  lev["system"] = classes["system"]["init"];

  // end of loading
  globals(L)["package"]["loaded"]["lev.system"] = true;
  return 0;
}

namespace lev
{

  class myEvent
  {
    public:
      myEvent() : evt() { }

      ~myEvent() { }

      ALLEGRO_EVENT evt;
  };
  static myEvent* cast_evt(void *obj) { return (myEvent *)obj; }

  static luabind::object safe_call(luabind::object func)
  {
    lua_State *L = func.interpreter();
    func.push(L);
    if (lua_pcall(L, 0 /* nargs */, 1 /* nreturns */, 0 /* std errror */))
    {
      lev::debug_print(lua_tostring(L, -1));
    }
    luabind::object o(luabind::from_stack(L, -1));
    lua_pop(L, 1);
    return o;
  }
  static luabind::object safe_call(luabind::object func, luabind::object arg1)
  {
    lua_State *L = func.interpreter();
    func.push(L);
    arg1.push(L);
    if (lua_pcall(L, 1 /* nargs */, 1 /* nreturns */, 0 /* std errror */))
    {
      lev::debug_print(lua_tostring(L, -1));
    }
    luabind::object o(luabind::from_stack(L, -1));
    lua_pop(L, 1);
    return o;
  }

  event::event() : base(), _obj(NULL)
  {
    _obj = new myEvent();
  }

  event::~event()
  {
    if (_obj) { delete cast_evt(_obj); }
  }

  int event::get_button() const
  {
    return cast_evt(_obj)->evt.mouse.button;
  }

  long event::get_count() const
  {
    return cast_evt(_obj)->evt.timer.count;
  }

  int event::get_dw() const
  {
    return cast_evt(_obj)->evt.mouse.dw;
  }

  int event::get_dx() const
  {
    return cast_evt(_obj)->evt.mouse.dx;
  }

  int event::get_dy() const
  {
    return cast_evt(_obj)->evt.mouse.dy;
  }

  int event::get_dz() const
  {
    return cast_evt(_obj)->evt.mouse.dz;
  }

  long event::get_id() const
  {
    ALLEGRO_EVENT &evt = cast_evt(_obj)->evt;
    switch (evt.type)
    {
      case ALLEGRO_EVENT_MOUSE_AXES:
      case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
      case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
        return (long)evt.mouse.display;
      case ALLEGRO_EVENT_KEY_CHAR:
      case ALLEGRO_EVENT_KEY_DOWN:
      case ALLEGRO_EVENT_KEY_UP:
        return (long)evt.keyboard.display;
      case ALLEGRO_EVENT_TIMER:
        return (long)evt.timer.source;
      default:
        return -1;
    }
  }

  std::string event::get_key() const
  {
    return input::to_keyname(get_key_code());
  }

  long event::get_key_code() const
  {
    ALLEGRO_EVENT &evt = cast_evt(_obj)->evt;
    return evt.keyboard.keycode;
  }

  int event::get_w() const
  {
    return cast_evt(_obj)->evt.mouse.w;
  }

  int event::get_x() const
  {
    return cast_evt(_obj)->evt.mouse.x;
  }

  int event::get_y() const
  {
    return cast_evt(_obj)->evt.mouse.y;
  }

  int event::get_z() const
  {
    return cast_evt(_obj)->evt.mouse.z;
  }

  bool event::is_pressed() const
  {
//    SDL_Event &evt = cast_evt(_obj)->evt;
//    if (evt.type == SDL_MOUSEBUTTONDOWN || evt.type == SDL_MOUSEBUTTONUP)
//    {
//      SDL_MouseButtonEvent &mouse = (SDL_MouseButtonEvent &)evt;
//      return mouse.state == SDL_PRESSED;
//    }
    return false;
  }

  bool event::is_released() const
  {
//    ALLEGRO_EVENT &evt = cast_evt(_obj)->evt;
//    if (evt.type == SDL_MOUSEBUTTONDOWN || evt.type == SDL_MOUSEBUTTONUP)
//    {
//      SDL_MouseButtonEvent &mouse = (SDL_MouseButtonEvent &)evt;
//      return mouse.state == SDL_RELEASED;
//    }
    return false;
  }

  bool event::left_is_down() const
  {
    ALLEGRO_EVENT &evt = cast_evt(_obj)->evt;
    return evt.mouse.button == 1;
//    if (evt.type == SDL_MOUSEMOTION)
//    {
//      SDL_MouseMotionEvent &motion = (SDL_MouseMotionEvent &)evt;
//      return motion.state & SDL_BUTTON_LMASK;
//    }
//    return false;
  }

  bool event::middle_is_down() const
  {
//    SDL_Event &evt = cast_evt(_obj)->evt;
//    if (evt.type == SDL_MOUSEMOTION)
//    {
//      SDL_MouseMotionEvent &motion = (SDL_MouseMotionEvent &)evt;
//      return motion.state & SDL_BUTTON_MMASK;
//    }
    return false;
  }

  bool event::right_is_down() const
  {
//    SDL_Event &evt = cast_evt(_obj)->evt;
//    if (evt.type == SDL_MOUSEMOTION)
//    {
//      SDL_MouseMotionEvent &motion = (SDL_MouseMotionEvent &)evt;
//      return motion.state & SDL_BUTTON_RMASK;
//    }
    return false;
  }

  class mySystem
  {
    private:
      mySystem(lua_State *L)
        : funcs(), name("lev"), running(true),
          on_tick(),
          screens(),
          timers(),
          L(L),
          evt_queue(NULL),
          instance_count(0)
      { }

    public:
      ~mySystem()
      {
//printf("QUITING1\n");
        font::destroy_cache();
        if (evt_queue)
        {
//printf("QUITING2\n");
          al_destroy_event_queue(evt_queue);
//printf("QUITING3\n");
          evt_queue = NULL;
        }
//printf("QUITING4\n");
        PHYSFS_deinit();
        al_uninstall_system();
//printf("QUITTED\n");
      }

      static mySystem* Create(lua_State *L)
      {
        mySystem *sys = NULL;
        try {
//          al_set_new_display_flags(ALLEGRO_OPENGL);
          sys = new mySystem(L);
//printf("INIT1!\n");
          if (! al_init()) { throw -1; }
//printf("INIT2!\n");
          if (! PHYSFS_init(NULL)) { throw -2; }
          PHYSFS_permitSymbolicLinks(true);
          PHYSFS_setWriteDir("./");
          PHYSFS_addToSearchPath("./", 1);
//          al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_INVERSE_ALPHA);
          al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);
          al_install_audio();
          al_init_acodec_addon();
          al_init_font_addon();
          al_init_ttf_addon();
          al_init_image_addon();
          al_init_primitives_addon();
          al_install_mouse();
          al_install_keyboard();
          al_set_physfs_file_interface();
          sys->evt_queue = al_create_event_queue();
          if (! sys->evt_queue) { throw -3; }
          al_register_event_source(sys->evt_queue, al_get_mouse_event_source());
          al_register_event_source(sys->evt_queue, al_get_keyboard_event_source());
//printf("INITTED!\n");
          return sys;
        }
        catch (...) {
          delete sys;
          return NULL;
        }
      }

      bool SetRunning(bool run)
      {
        running = run;
        return true;
      }

      lua_State *L;
      ALLEGRO_EVENT_QUEUE *evt_queue;
      std::map<unsigned int, luabind::object> funcs;
      std::string name;
      luabind::object on_tick;
      std::map<ALLEGRO_TIMER*, boost::weak_ptr<timer> > timers;
      std::map<ALLEGRO_DISPLAY*, boost::weak_ptr<screen> > screens;
      bool running;
      int instance_count;
  };
  static mySystem *singleton;
//  static mySystem *cast_sys(void *obj) { return (mySystem *)obj; }

  system::system() : base(), valid(true)
  {
  }

  system::~system()
  {
    done();
  }

  boost::shared_ptr<screen>
    system::create_screen(const char *title, int w, int h, int x, int y)
  {
    boost::shared_ptr<screen> scr = screen::create(title, w, h, x, y);
    if (! scr) { return scr; }
    ALLEGRO_DISPLAY *disp = (ALLEGRO_DISPLAY *)scr->get_id();
//printf("ATTACHING SCREEN: %p\n", disp);
    singleton->screens[disp] = scr;
    al_register_event_source(singleton->evt_queue, al_get_display_event_source(disp));
    return scr;
  }

  int system::create_screen_l(lua_State *L)
  {
    using namespace luabind;
    const char *title = "Lev Application";
    int x = -1, y = -1;
//    int x = SDL_WINDOWPOS_UNDEFINED, y = SDL_WINDOWPOS_UNDEFINED;
    int w = 640, h = 480;

    try {
      luaL_checktype(L, 1, LUA_TUSERDATA);
      boost::shared_ptr<system> sys =
        object_cast<boost::shared_ptr<system> >(object(from_stack(L, 1)));
      object t = util::get_merged(L, 2, -1);

      if (t["title"]) { title = object_cast<const char *>(t["title"]); }
      else if (t["t"]) { title = object_cast<const char *>(t["t"]); }
      else if (t["name"]) { title = object_cast<const char *>(t["name"]); }
      else if (t["n"]) { title = object_cast<const char *>(t["n"]); }
      else if (t["caption"]) { title = object_cast<const char *>(t["caption"]); }
      else if (t["c"]) { title = object_cast<const char *>(t["c"]); }
      else if (t["lua.string1"]) { title = object_cast<const char *>(t["lua.string1"]); }

      if (t["w"]) { w = object_cast<int>(t["w"]); }
      else if (t["lua.number1"]) { w = object_cast<int>(t["lua.number1"]); }

      if (t["h"]) { h = object_cast<int>(t["h"]); }
      else if (t["lua.number2"]) { h = object_cast<int>(t["lua.number2"]); }

      if (t["x"]) { x = object_cast<int>(t["x"]); }
      else if (t["lua.number3"]) { x = object_cast<int>(t["lua.number3"]); }

      if (t["y"]) { y = object_cast<int>(t["y"]); }
      else if (t["lua.number4"]) { y = object_cast<int>(t["lua.number4"]); }

      object o = globals(L)["lev"]["classes"]["system"]["create_screen_c"](sys, title, w, h, x, y);
      o.push(L);
      return 1;
    }
    catch (...) {
      lev::debug_print(lua_tostring(L, -1));
      lev::debug_print("error on wrapped window creation");
      lua_pushnil(L);
      return 1;
    }
  }

  boost::shared_ptr<mixer> system::create_mixer()
  {
    return mixer::create();
  }

  boost::shared_ptr<timer> system::create_timer(double interval, bool one_shot)
  {
    boost::shared_ptr<timer> t;
    t = timer::create(interval, one_shot);
    if (! t) { return t; }
    ALLEGRO_TIMER *id = (ALLEGRO_TIMER *)t->get_id();
    singleton->timers[id] = t;
    al_register_event_source(singleton->evt_queue, al_get_timer_event_source(id));
    return t;
  }

  bool system::delay(unsigned long msec)
  {
    al_rest(msec);
    return true;
  }

  bool system::done()
  {
    if (valid && singleton)
    {
      singleton->instance_count--;
//printf("INST: %d\n", singleton->instance_count);
      if (singleton->instance_count == 0)
      {
        delete singleton;
        singleton = NULL;
      }
      valid = false;
      return true;
    }
    return false;
  }

  bool system::do_event()
  {
    ALLEGRO_EVENT allegro_evt;
    event e;
    mySystem *sys = singleton;

//printf("DO EVENT\n");
    if (al_get_next_event(sys->evt_queue, &allegro_evt))
    {
//printf("EVENT: %d\n", allegro_evt.type);
      luabind::object f;
      cast_evt(e.get_rawobj())->evt = allegro_evt;

      if (allegro_evt.type == ALLEGRO_EVENT_KEY_DOWN || allegro_evt.type == ALLEGRO_EVENT_KEY_UP)
      {
        ALLEGRO_DISPLAY *disp = allegro_evt.keyboard.display;
        if (boost::shared_ptr<screen> scr = sys->screens[disp].lock())
        {
          if (allegro_evt.type == ALLEGRO_EVENT_KEY_DOWN) { f = scr->get_on_key_down(); }
          else { f = scr->get_on_key_up(); }
        }
      }
      else if (allegro_evt.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN ||
               allegro_evt.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP)
      {
        ALLEGRO_DISPLAY *disp = allegro_evt.mouse.display;
        if (boost::shared_ptr<screen> scr = sys->screens[disp].lock())
        {
          if (allegro_evt.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
          {
            if (allegro_evt.mouse.button == 1)      { f = scr->get_on_left_down(); }
            else if (allegro_evt.mouse.button == 2) { f = scr->get_on_right_down(); }
            else if (allegro_evt.mouse.button == 3) { f = scr->get_on_middle_down(); }
            if (! f.is_valid()) { f = scr->get_on_button_down(); }
          }
          else
          {
            if (allegro_evt.mouse.button == 1)      { f = scr->get_on_left_up(); }
            else if (allegro_evt.mouse.button == 2) { f = scr->get_on_right_up(); }
            else if (allegro_evt.mouse.button == 3) { f = scr->get_on_middle_up(); }
            if (! f.is_valid()) { f = scr->get_on_button_up(); }
          }
        }
      }
      else if (allegro_evt.type == ALLEGRO_EVENT_MOUSE_AXES)
      {
        ALLEGRO_DISPLAY *disp = allegro_evt.mouse.display;
        if (boost::shared_ptr<screen> scr = sys->screens[disp].lock())
        {
          if (allegro_evt.mouse.dz < 0)      { f = scr->get_on_wheel_down(); }
          else if (allegro_evt.mouse.dz > 0) { f = scr->get_on_wheel_up(); }

          if (! f.is_valid() && allegro_evt.mouse.dz != 0) { f = scr->get_on_wheel(); }
          if (! f.is_valid()) { f = scr->get_on_motion(); }
        }
      }
      else if (allegro_evt.type == ALLEGRO_EVENT_TIMER)
      {
        ALLEGRO_TIMER *source = allegro_evt.timer.source;
        if (boost::shared_ptr<timer> t = sys->timers[source].lock())
        {
          f = t->get_on_tick();
          if (t->is_one_shot()) { t->stop(); }
        }
      }
      else if (allegro_evt.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
      {
        ALLEGRO_DISPLAY *source = allegro_evt.display.source;
        if (boost::shared_ptr<screen> scr = sys->screens[source].lock())
        {
          f = scr->get_on_close();
          if (! f.is_valid()) { scr->close(); }
        }
      }
      else
      {
        // OTHERS
      }

      if (! f.is_valid())
      {
        f = sys->funcs[allegro_evt.type];
      }
      if (f.is_valid() && luabind::type(f) == LUA_TFUNCTION)
      {
        try {
          f(&e);
        }
        catch (...) {
          lev::debug_print(lua_tostring(singleton->L, -1));
          lev::debug_print("error on event processing\n");
        }
      }
//printf("RETURN EVENT: %d\n", allegro_evt.type);
      return true;
    }
    return false;
  }

  bool system::do_events()
  {
    while (is_running() && do_event()) { }
    return true;
  }

//  boost::shared_ptr<system> system::get()
//  {
//    boost::shared_ptr<system> sys;
//    if (! singleton) { return sys; }
//    try {
//      sys.reset(new system);
//    }
//    catch (...) {
//      sys.reset();
//    }
//    return sys;
//  }

  double system::get_elapsed_time() const
  {
    return al_get_time();
  }

  lua_State *system::get_interpreter()
  {
    if (! singleton) { return NULL; }
    return singleton->L;
  }

  std::string system::get_name()
  {
    return singleton->name;
  }

  luabind::object system::get_on_quit()
  {
//    return cast_sys(_obj)->funcs[SDL_QUIT];
  }

  luabind::object system::get_on_tick()
  {
    return singleton->on_tick;
  }

  boost::shared_ptr<system> system::init(lua_State *L)
  {
    boost::shared_ptr<system> sys;
    try {
      sys.reset(new system);
      if (! sys) { throw -1; }
      if (! singleton)
      {
        singleton = mySystem::Create(L);
        if (! singleton) { throw -2; }
      }
      singleton->instance_count++;
    }
    catch (...) {
      sys.reset();
      lev::debug_print("error on system instance creation");
    }
    return sys;
  }

  bool system::is_debugging()
  {
    if (! singleton) { return false; }
    return false;
  }

  bool system::is_running()
  {
    if (! singleton) { return false; }
    return singleton->running;
  }

  bool system::quit(bool force)
  {
    return set_running(false);
    if (force)
    {
      set_running(false);
    }
    else
    {
//      SDL_Event q;
//      q.type = SDL_QUIT;
//      SDL_PushEvent(&q);
    }
    return true;
  }

  bool system::run(boost::shared_ptr<screen> main)
  {
    if (! main || ! main->is_valid() ) { return false; }

    mySystem *sys = singleton;
    sys->running = true;
    while (main->is_valid() && is_running())
    {
      try {
        if (sys->on_tick && luabind::type(sys->on_tick) == LUA_TFUNCTION)
        {
          sys->on_tick();
//          safe_call(sys->on_tick);
        }
        do_events();
      }
      catch (...) {
        lev::debug_print(lua_tostring(sys->L, -1));
        lev::debug_print("error on system::run");
        return false;
      }
    }
    set_running(false);
    return true;
  }

  bool system::run0()
  {
    mySystem *sys = singleton;
    sys->running = true;
    while (is_running())
    {
      try {
        if (sys->on_tick && luabind::type(sys->on_tick) == LUA_TFUNCTION)
        {
          sys->on_tick();
//          safe_call(sys->on_tick);
        }
        do_events();
      }
      catch (...) {
        lev::debug_print(lua_tostring(sys->L, -1));
        lev::debug_print("error on system::run0");
        return false;
      }
    }
    return true;
  }

  bool system::set_name(const std::string &name)
  {
    singleton->name = name;
    return true;
  }

  bool system::set_on_quit(luabind::object func)
  {
//    cast_sys(_obj)->funcs[SDL_QUIT] = func;
    return true;
  }

  bool system::set_on_tick(luabind::object func)
  {
    singleton->on_tick = func;
    return true;
  }

  bool system::set_running(bool run)
  {
    singleton->SetRunning(run);
  }

  bool system::start_debug()
  {
//    boost::shared_ptr<debug_window> win = debug_window::init();
//    if (! win) { return win; }
////printf("ATTACHING DEBUG WINDOW: %d\n", (int)win->get_id());
//    cast_sys(_obj)->windows[win->get_id()] = win;
//    return win;
    return false;
  }

  bool system::stop_debug()
  {
//    if (debug_window::get())
//    {
//      debug_window::get()->close();
//      return true;
//    }
    return false;
  }

}

