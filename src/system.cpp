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

#include <map>

#include <luabind/luabind.hpp>
#include <SDL/SDL.h>

int luaopen_lev_system(lua_State *L)
{
  using namespace luabind;
  using namespace lev;

  open(L);
  globals(L)["require"]("lev.base");

  module(L, "lev")
  [
    namespace_("system"),
    namespace_("classes")
    [
      class_<lev::system, base>("system")
        .def("delay", &system::delay)
        .def("do_event", &system::do_event)
        .def("do_events", &system::do_events)
        .property("on_quit", &system::get_on_quit, &system::set_on_quit)
        .property("screen", &system::get_screen)
        .def("set_video_mode", &system::set_video_mode)
        .def("set_video_mode", &system::set_video_mode2)
        .property("ticks", &system::get_ticks)
        .def("toggle_full_screen", &system::toggle_full_screen)
        .scope
        [
          def("get", &system::get),
          def("init", &system::init)
        ]
    ]
  ];
  object lev = globals(L)["lev"];
  object classes = lev["classes"];
  lev["system"] = classes["system"]["get"];

  globals(L)["package"]["loaded"]["lev.system"] = true;
  return 0;
}

namespace lev
{

  class mySystem
  {
    private:
      mySystem() : funcs() { }
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

      std::map<Uint8, luabind::object> funcs;
  };
  static mySystem *cast_sys(void *obj) { return (mySystem *)obj; }

  system::system() : base(), _obj(NULL)
  {
  }

  system::~system()
  {
//printf("QUITING!\n");
    if (_obj)
    {
//printf("QUITING!1\n");
      delete cast_sys(_obj);
//printf("QUITING!2\n");
      SDL_QuitSubSystem(SDL_INIT_VIDEO);
//      SDL_Quit();
    }
//printf("QUITING!3\n");
  }

  bool system::delay(unsigned long msec)
  {
    SDL_Delay(msec);
    return true;
  }

  bool system::do_event()
  {
    SDL_Event event;

    if (SDL_PollEvent(&event))
    {
      luabind::object f = cast_sys(_obj)->funcs[event.type];
      if (f && luabind::type(f) == LUA_TFUNCTION)
      {
        f();
      }

      switch (event.type)
      {
        case SDL_QUIT:
//          printf("QUIT!\n");
          return true;
        default:
//          printf("OTHERS!\n");
          return true;
      }
    }
    else { return false; }
  }

  bool system::do_events()
  {
    while (do_event()) { }
    return true;
  }

  luabind::object system::get_on_quit()
  {
    return cast_sys(_obj)->funcs[SDL_QUIT];
  }

  unsigned long system::get_ticks()
  {
    return SDL_GetTicks();
  }

  system* system::init()
  {
    static system sys;
    if (sys._obj) { return &sys; }
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) { return NULL; }
//    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) { return NULL; }
//printf("INITTED!\n");
    sys._obj = mySystem::Create();
    if (! sys._obj) { return NULL; }
    return &sys;
  }

  bool system::set_on_quit(luabind::object func)
  {
    cast_sys(_obj)->funcs[SDL_QUIT] = func;
    return true;
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

