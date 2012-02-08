/////////////////////////////////////////////////////////////////////////////
// Name:        src/debug.cpp
// Purpose:     source for debugging features
// Author:      Akiva Miura <akiva.miura@gmail.com>
// Modified by:
// Created:     02/08/2012
// Copyright:   (C) 2010-2012 Akiva Miura
// Licence:     MIT License
/////////////////////////////////////////////////////////////////////////////

// pre-compiled header
#include "prec.h"

// declarations
#include "lev/debug.hpp"

// dependencies
#include "lev/system.hpp"

// static member variable initialization
boost::shared_ptr<lev::debug_window> lev::debug_window::singleton;

int luaopen_lev_debug(lua_State *L)
{
  using namespace luabind;
  using namespace lev;

  open(L);
  // beginning of loading
  globals(L)["package"]["loaded"]["lev.debug"] = true;
  // pre-requirement
  globals(L)["require"]("lev.system");

  module(L, "lev")
  [
    namespace_("classes")
    [
      class_<debug_window, window, boost::shared_ptr<base> >("debug_window")
        .scope
        [
          def("get", &debug_window::get),
          def("init", &debug_window::init)
        ]
    ]
  ];

  globals(L)["package"]["loaded"]["debug"] = true;
  return 0;
}

namespace lev
{

  debug_window::debug_window() : window(), ptr_screen() { }

  debug_window::~debug_window() { }

  boost::shared_ptr<debug_window> debug_window::init()
  {
    // singleton already exists
    if (singleton) { return singleton; }
    // system is not yet initialized
    if (! system::get()) { return singleton; }
    try {
      singleton.reset(new debug_window);
      if (! singleton) { throw -1; }
      singleton->ptr_screen = screen::create(singleton);
      if (! singleton->ptr_screen) { throw -2; }
    }
    catch (...) {
      singleton.reset();
      fprintf(stderr, "error on debug window singleton initialization\n");
    }
    return singleton;
  }
}

