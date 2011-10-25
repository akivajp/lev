/////////////////////////////////////////////////////////////////////////////
// Name:        src/system.cpp
// Purpose:     source for system managing class
// Author:      Akiva Miura <akiva.miura@gmail.com>
// Modified by:
// Created:     13/10/2011
// Copyright:   (C) 2010-2011 Akiva Miura
// Licence:     MIT License
/////////////////////////////////////////////////////////////////////////////

#include "prec.h"

#include "lev/system.hpp"
#include "register.hpp"

#include <map>

#include <luabind/luabind.hpp>
#include <SDL/SDL.h>

namespace lev
{
  class finalizer : public base
  {
    protected:
      finalizer() : base() { }
    public:
      virtual ~finalizer() { system::get()->done(); }
      static finalizer* create()
      {
        finalizer *f = NULL;
        try {
          f = new finalizer;
          return f;
        }
        catch (...) {
          delete f;
          return NULL;
        }
      }

      virtual type_id get_type_id() const { return LEV_TFINALIZER; }
      virtual const char *get_type_name() const { return "lev.system.finalizer"; }
  };
}

int luaopen_lev_system(lua_State *L)
{
  using namespace luabind;
  using namespace lev;

  open(L);
  globals(L)["require"]("lev.base");
  globals(L)["require"]("lev.image");

  module(L, "lev")
  [
    namespace_("system"),
    namespace_("classes")
    [
      class_<event, base>("event")
        .property("button", &event::get_button)
        .property("device", &event::get_device)
        .property("dx", &event::get_dx)
        .property("dy", &event::get_dy)
        .property("is_pressed", &event::is_pressed)
        .property("is_released", &event::is_released)
        .property("key", &event::get_key)
        .property("keycode", &event::get_key_code)
        .property("key_code", &event::get_key_code)
        .property("left", &event::get_left)
        .property("middle", &event::get_middle)
        .property("pressed", &event::is_pressed)
        .property("released", &event::is_released)
        .property("right", &event::get_right)
        .property("x", &event::get_x)
        .property("y", &event::get_y),
      class_<lev::system, base>("system")
        .def("close", &system::done)
        .def("delay", &system::delay)
        .def("do_event", &system::do_event)
        .def("do_events", &system::do_events)
        .def("done", &system::done)
        .property("is_running", &system::is_running, &system::set_running)
        .property("on_button_down", &system::get_on_button_down, &system::set_on_button_down)
        .property("on_button_up", &system::get_on_button_up, &system::set_on_button_up)
        .property("on_key_down", &system::get_on_key_down, &system::set_on_key_down)
        .property("on_key_up", &system::get_on_key_up, &system::set_on_key_up)
        .property("on_left_down", &system::get_on_left_down, &system::set_on_left_down)
        .property("on_left_up", &system::get_on_left_up, &system::set_on_left_up)
        .property("on_middle_down", &system::get_on_middle_down, &system::set_on_middle_down)
        .property("on_middle_up", &system::get_on_middle_up, &system::set_on_middle_up)
        .property("on_motion", &system::get_on_motion, &system::set_on_motion)
        .property("on_quit", &system::get_on_quit, &system::set_on_quit)
        .property("on_right_down", &system::get_on_right_down, &system::set_on_right_down)
        .property("on_right_up", &system::get_on_right_up, &system::set_on_right_up)
        .property("on_tick", &system::get_on_tick, &system::set_on_tick)
        .def("quit", &system::quit)
        .def("quit", &system::quit0)
        .def("run", &system::run)
        .property("screen", &system::get_screen)
        .def("set_running", &system::set_running)
        .def("set_video_mode", &system::set_video_mode)
        .def("set_video_mode", &system::set_video_mode2)
        .property("ticks", &system::get_ticks)
        .def("toggle_full_screen", &system::toggle_full_screen)
        .scope
        [
          def("get_c", &system::get),
          def("init", &system::init)
        ],
      class_<lev::finalizer, base>("finalizer")
        .scope
        [
          def("create", &finalizer::create, adopt(result))
        ]
    ]
  ];
  object lev = globals(L)["lev"];
  object classes = lev["classes"];
  register_to(classes["system"], "get", &system::get_l);
  lev["system"] = classes["system"]["get"];
  lev["finalizer"] = classes["finalizer"]["create"];

  globals(L)["package"]["loaded"]["lev.system"] = true;
  return 0;
}

namespace lev
{

  class myEvent
  {
    public:
      myEvent()
        : x(0), y(0), dx(0), dy(0), device(-1), button(0),
          left(false), middle(false), right(false),
          pressed(false), released(false),
          key_code(-1)
      { }

      ~myEvent() { }

      int x, y, dx, dy, button, device;
      bool left, middle, right;
      bool pressed, released;
      long key_code;
  };
  static myEvent* cast_evt(void *obj) { return (myEvent *)obj; }

  event::event() : base(), _obj(NULL)
  {
    _obj = new myEvent();
  }

  event::~event()
  {
    if (_obj) { delete cast_evt(_obj); }
  }

  std::string event::get_button() const
  {
    switch (cast_evt(_obj)->button)
    {
      case SDL_BUTTON_LEFT:
        return "left";
      case SDL_BUTTON_MIDDLE:
        return "middle";
      case SDL_BUTTON_RIGHT:
        return "right";
      case SDL_BUTTON_WHEELUP:
        return "up";
      case SDL_BUTTON_WHEELDOWN:
        return "down";
      default:
        return "";
    }
  }

  int event::get_device() const
  {
    return cast_evt(_obj)->device;
  }

  int event::get_dx() const
  {
    return cast_evt(_obj)->dx;
  }

  int event::get_dy() const
  {
    return cast_evt(_obj)->dy;
  }

  std::string event::get_key() const
  {
    return "";
  }

  long event::get_key_code() const
  {
    return cast_evt(_obj)->key_code;
  }

  bool event::get_left() const
  {
    return cast_evt(_obj)->left;
  }

  bool event::get_middle() const
  {
    return cast_evt(_obj)->middle;
  }

  bool event::get_right() const
  {
    return cast_evt(_obj)->right;
  }

  int event::get_x() const
  {
    return cast_evt(_obj)->x;
  }

  int event::get_y() const
  {
    return cast_evt(_obj)->y;
  }

  bool event::is_pressed() const
  {
    return cast_evt(_obj)->pressed;
  }

  bool event::is_released() const
  {
    return cast_evt(_obj)->released;
  }

  bool event::set_button_code(int button)
  {
    cast_evt(_obj)->button = button;
    return true;
  }

  bool event::set_device(int device)
  {
    cast_evt(_obj)->device = device;
    return true;
  }

  bool event::set_dx(int dx)
  {
    cast_evt(_obj)->dx = dx;
    return true;
  }

  bool event::set_dy(int dy)
  {
    cast_evt(_obj)->dy = dy;
    return true;
  }

  bool event::set_key_code(long key)
  {
    cast_evt(_obj)->key_code = key;
    return true;
  }

  bool event::set_left(bool pressed)
  {
    cast_evt(_obj)->left = pressed;
    return true;
  }

  bool event::set_middle(bool pressed)
  {
    cast_evt(_obj)->middle = pressed;
    return true;
  }

  bool event::set_pressed(bool pressed)
  {
    return cast_evt(_obj)->pressed = pressed;
  }

  bool event::set_released(bool released)
  {
    return cast_evt(_obj)->released = released;
  }

  bool event::set_right(bool pressed)
  {
    cast_evt(_obj)->right = pressed;
    return true;
  }

  bool event::set_x(int x)
  {
    cast_evt(_obj)->x = x;
    return true;
  }

  bool event::set_y(int y)
  {
    cast_evt(_obj)->y = y;
    return true;
  }


  class mySystem
  {
    private:
      mySystem()
        : funcs(), running(true),
          on_tick(),
          on_left_down(),   on_left_up(),
          on_middle_down(), on_middle_up(),
          on_right_down(),  on_right_up()
      { }
    public:
      ~mySystem() { }

      static mySystem* Create()
      {
        mySystem *sys = NULL;
        try {
          sys = new mySystem;
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

      std::map<Uint8, luabind::object> funcs;
      luabind::object on_tick;
      luabind::object on_left_down,   on_left_up;
      luabind::object on_middle_down, on_middle_up;
      luabind::object on_right_down,  on_right_up;
      bool running;
  };
  static mySystem *cast_sys(void *obj) { return (mySystem *)obj; }

  system::system() : base(), _obj(NULL)
  {
  }

  system::~system()
  {
    done();
  }

  bool system::delay(unsigned long msec)
  {
    SDL_Delay(msec);
    return true;
  }

  bool system::done()
  {
//printf("QUITING1\n");
    if (_obj)
    {
//printf("QUITING2\n");
      SDL_Quit();
      delete cast_sys(_obj);
      _obj = NULL;
      return true;
    }
//printf("QUITING3\n");
    return false;
  }

  bool system::do_event()
  {
    SDL_Event sdl_evt;
    event e;

    if (SDL_PollEvent(&sdl_evt))
    {
      mySystem *sys = cast_sys(_obj);
      luabind::object f = sys->funcs[sdl_evt.type];

      if (sdl_evt.type == SDL_KEYDOWN || sdl_evt.type == SDL_KEYUP)
      {
        SDL_KeyboardEvent &keyboard = (SDL_KeyboardEvent &)sdl_evt;
        e.set_key_code(keyboard.keysym.unicode);
      }
      else if (sdl_evt.type == SDL_MOUSEBUTTONDOWN || sdl_evt.type == SDL_MOUSEBUTTONUP)
      {
        SDL_MouseButtonEvent &mouse = (SDL_MouseButtonEvent &)sdl_evt;
        e.set_button_code(mouse.button);
        e.set_device(mouse.which);
        e.set_x(mouse.x);
        e.set_y(mouse.y);
        e.set_pressed(mouse.state == SDL_PRESSED);
        e.set_released(mouse.state == SDL_RELEASED);
        if (mouse.button == SDL_BUTTON_LEFT)
        {
          if (mouse.state == SDL_PRESSED && sys->on_left_down)
          { f = sys->on_left_down; }
          if (mouse.state == SDL_RELEASED && sys->on_left_up)
          { f = sys->on_left_up; }
        }
        if (mouse.button == SDL_BUTTON_MIDDLE)
        {
          if (mouse.state == SDL_PRESSED && sys->on_middle_down)
          { f = sys->on_middle_down; }
          if (mouse.state == SDL_RELEASED && sys->on_middle_up)
          { f = sys->on_middle_up; }
        }
        if (mouse.button == SDL_BUTTON_RIGHT)
        {
          if (mouse.state == SDL_PRESSED && sys->on_right_down)
          { f = sys->on_right_down; }
          if (mouse.state == SDL_RELEASED && sys->on_right_up)
          { f = sys->on_right_up; }
        }
      }
      else if (sdl_evt.type == SDL_MOUSEMOTION)
      {
        SDL_MouseMotionEvent &motion = (SDL_MouseMotionEvent &)sdl_evt;
        e.set_x(motion.x);
        e.set_y(motion.y);
        e.set_dx(motion.xrel);
        e.set_dy(motion.yrel);
        e.set_left(motion.state & SDL_BUTTON_LMASK);
        e.set_middle(motion.state & SDL_BUTTON_MMASK);
        e.set_right(motion.state & SDL_BUTTON_RMASK);
      }
      else if (sdl_evt.type == SDL_QUIT)
      {
        if (! f) { set_running(false); }
      }
      else
      {
        // OTHERS
      }

      if (f && luabind::type(f) == LUA_TFUNCTION)
      {
        f(&e);
      }
      return true;
    }
    return false;
  }

  bool system::do_events()
  {
    while (do_event()) { }
    return true;
  }

  int system::get_l(lua_State *L)
  {
    using namespace luabind;
    object o = globals(L)["lev"]["classes"]["system"]["get_c"]();
    o["finalizer"] = globals(L)["lev"]["classes"]["finalizer"]["create"]();
    o.push(L);
    return 1;
  }

  luabind::object system::get_on_button_down()
  {
    return cast_sys(_obj)->funcs[SDL_MOUSEBUTTONDOWN];
  }

  luabind::object system::get_on_button_up()
  {
    return cast_sys(_obj)->funcs[SDL_MOUSEBUTTONUP];
  }

  luabind::object system::get_on_key_down()
  {
    return cast_sys(_obj)->funcs[SDL_KEYDOWN];
  }

  luabind::object system::get_on_key_up()
  {
    return cast_sys(_obj)->funcs[SDL_KEYUP];
  }

  luabind::object system::get_on_left_down()
  {
    return cast_sys(_obj)->on_left_down;
  }

  luabind::object system::get_on_left_up()
  {
    return cast_sys(_obj)->on_left_up;
  }

  luabind::object system::get_on_middle_down()
  {
    return cast_sys(_obj)->on_middle_down;
  }

  luabind::object system::get_on_middle_up()
  {
    return cast_sys(_obj)->on_middle_up;
  }

  luabind::object system::get_on_motion()
  {
    return cast_sys(_obj)->funcs[SDL_MOUSEMOTION];
  }

  luabind::object system::get_on_quit()
  {
    return cast_sys(_obj)->funcs[SDL_QUIT];
  }

  luabind::object system::get_on_right_down()
  {
    return cast_sys(_obj)->on_right_down;
  }

  luabind::object system::get_on_right_up()
  {
    return cast_sys(_obj)->on_right_up;
  }

  luabind::object system::get_on_tick()
  {
    return cast_sys(_obj)->on_tick;
  }

  unsigned long system::get_ticks()
  {
    return SDL_GetTicks();
  }

  system* system::init()
  {
    static system sys;
    if (sys._obj) { return &sys; }
//printf("INITTING!\n");
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) { return NULL; }
    SDL_EnableUNICODE(1);
//printf("INITTED!\n");
    sys._obj = mySystem::Create();
    if (! sys._obj) { return NULL; }
    return &sys;
  }

  bool system::is_running()
  {
    return cast_sys(_obj)->running;
  }

  bool system::quit(bool force)
  {
    if (force)
    {
      set_running(false);
    }
    else
    {
      SDL_Event q;
      q.type = SDL_QUIT;
      SDL_PushEvent(&q);
    }
    return true;
  }

  bool system::run()
  {
    mySystem *sys = cast_sys(_obj);
    while (is_running())
    {
      if (sys->on_tick && luabind::type(sys->on_tick) == LUA_TFUNCTION)
      {
        sys->on_tick();
      }
      do_events();
    }
    return true;
  }

  bool system::set_on_button_down(luabind::object func)
  {
    cast_sys(_obj)->funcs[SDL_MOUSEBUTTONDOWN] = func;
    return true;
  }

  bool system::set_on_button_up(luabind::object func)
  {
    cast_sys(_obj)->funcs[SDL_MOUSEBUTTONUP] = func;
    return true;
  }

  bool system::set_on_key_down(luabind::object func)
  {
    cast_sys(_obj)->funcs[SDL_KEYDOWN] = func;
    return true;
  }

  bool system::set_on_key_up(luabind::object func)
  {
    cast_sys(_obj)->funcs[SDL_KEYUP] = func;
    return true;
  }

  bool system::set_on_left_down(luabind::object func)
  {
    cast_sys(_obj)->on_left_down = func;
    return true;
  }

  bool system::set_on_left_up(luabind::object func)
  {
    cast_sys(_obj)->on_left_up = func;
    return true;
  }

  bool system::set_on_middle_down(luabind::object func)
  {
    cast_sys(_obj)->on_middle_down = func;
    return true;
  }

  bool system::set_on_middle_up(luabind::object func)
  {
    cast_sys(_obj)->on_middle_up = func;
    return true;
  }

  bool system::set_on_motion(luabind::object func)
  {
    cast_sys(_obj)->funcs[SDL_MOUSEMOTION] = func;
    return true;
  }

  bool system::set_on_quit(luabind::object func)
  {
    cast_sys(_obj)->funcs[SDL_QUIT] = func;
    return true;
  }

  bool system::set_on_right_down(luabind::object func)
  {
    cast_sys(_obj)->on_right_down = func;
    return true;
  }

  bool system::set_on_right_up(luabind::object func)
  {
    cast_sys(_obj)->on_right_up = func;
    return true;
  }

  bool system::set_on_tick(luabind::object func)
  {
    cast_sys(_obj)->on_tick = func;
    return true;
  }

  bool system::set_running(bool run)
  {
    return cast_sys(_obj)->SetRunning(run);
  }

  screen* system::set_video_mode(int width, int height, int depth)
  {
    return screen::set_mode(width, height, depth);
  }

  bool system::toggle_full_screen()
  {
    screen* s = system::get_screen();
    if (! s) { return false; }
    return SDL_WM_ToggleFullScreen((SDL_Surface *)s->get_rawobj());
  }

}

