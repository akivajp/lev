/////////////////////////////////////////////////////////////////////////////
// Name:        src/window.cpp
// Purpose:     source for window managing class
// Author:      Akiva Miura <akiva.miura@gmail.com>
// Modified by:
// Created:     01/14/2012
// Copyright:   (C) 2010-2012 Akiva Miura
// Licence:     MIT License
/////////////////////////////////////////////////////////////////////////////

// pre-compiled header
#include "prec.h"

// declarations
#include "lev/window.hpp"

// dependencies
#include "lev/system.hpp"
#include "lev/util.hpp"
#include "register.hpp"

// libraries
#include <luabind/luabind.hpp>
#include <SDL.h>

int luaopen_lev_window(lua_State *L)
{
  using namespace luabind;
  using namespace lev;

  open(L);
  // beginning of loading
  globals(L)["package"]["loaded"]["lev.window"] = true;
  // pre-requirement
  globals(L)["require"]("lev.system");

  module(L, "lev")
  [
    namespace_("classes")
    [
      class_<window, base>("window")
        .def("close", &window::close)
        .def("create_context", &window::create_context, adopt(result))
        .def("create_screen", &window::create_context, adopt(result))
        .property("h", &window::get_h, &window::set_h)
        .property("height", &window::get_h, &window::set_h)
        .def("screen", &window::create_context, adopt(result))
        .property("w", &window::get_w, &window::set_w)
        .property("width", &window::get_w, &window::set_w)
    ]
  ];

  globals(L)["package"]["loaded"]["lev.window"] = true;
  return 0;
}


namespace lev
{

  window::window() : _obj(NULL) { }

  window::~window()
  {
    if (_obj)
    {
      SDL_DestroyWindow((SDL_Window *)_obj);
    }
  }

  bool window::close()
  {
    if (_obj)
    {
      SDL_DestroyWindow((SDL_Window *)_obj);
      _obj = NULL;
      return true;
    }
    return false;
  }

  window* window::create(const char *title, int x, int y, int w, int h, unsigned long flags)
  {
    window* win = NULL;
    try {
      SDL_GL_SetAttribute(SDL_GL_RED_SIZE,    8);
      SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,  8);
      SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,   8);
      SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,  8);
      SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
      SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

      win = new window;
      win->_obj = SDL_CreateWindow(title, x, y, w, h, flags);
      if (! win->_obj) { throw -1; }
      return win;
    }
    catch (...) {
      delete win;
      return NULL;
    }
  }

  int window::create_l(lua_State *L)
  {
    using namespace luabind;
    const char *title = "Seraf Application";
    int x = SDL_WINDOWPOS_UNDEFINED, y = SDL_WINDOWPOS_UNDEFINED;
    int w = 640, h = 480;
    const char *flags_str = NULL;
    unsigned long flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;

    object t = util::get_merged(L, 1, -1);

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

    if (t["flags"]) { flags_str = object_cast<const char *>(t["flags"]); }
    else if (t["lua.string2"]) { flags_str = object_cast<const char *>(t["lua.string2"]); }
    if (flags_str)
    {
      if (strstr(flags_str, "borderless"))    { flags |= SDL_WINDOW_BORDERLESS; }
      if (strstr(flags_str, "full"))          { flags |= SDL_WINDOW_FULLSCREEN; }
      if (strstr(flags_str, "hidden"))        { flags &= ~SDL_WINDOW_SHOWN; }
      if (strstr(flags_str, "input_grabbed")) { flags |= SDL_WINDOW_INPUT_GRABBED; }
      if (strstr(flags_str, "max"))           { flags |= SDL_WINDOW_MAXIMIZED; }
      if (strstr(flags_str, "min"))           { flags |= SDL_WINDOW_MINIMIZED; }
      if (strstr(flags_str, "resizable"))     { flags |= SDL_WINDOW_RESIZABLE; }
      if (strstr(flags_str, "shown"))         { flags |= SDL_WINDOW_SHOWN; }
    }

    object o = globals(L)["lev"]["classes"]["system"]["create_window_c"](title, x, y, w, h, flags);
    o.push(L);
    return 1;
  }

  screen* window::create_context()
  {
    return screen::create(this);
  }

  int window::get_h()
  {
    int h;
    SDL_GetWindowSize((SDL_Window *)_obj, NULL, &h);
    return h;
  }

  int window::get_w()
  {
    int w;
    SDL_GetWindowSize((SDL_Window *)_obj, &w, NULL);
    return w;
  }

  bool window::set_h(int h)
  {
    int w = get_w();
    SDL_SetWindowSize((SDL_Window *)_obj, w, h);
  }

  bool window::set_w(int w)
  {
    int h = get_h();
    SDL_SetWindowSize((SDL_Window *)_obj, w, h);
  }

}

