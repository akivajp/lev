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
#include "lev/font.hpp"
#include "lev/system.hpp"
#include "lev/string.hpp"
#include "lev/util.hpp"

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
  globals(L)["require"]("lev.window");

  module(L, "lev")
  [
    namespace_("debug")
    [
      def("print", &debug_print_lua)
    ],
    namespace_("classes")
    [
      class_<debug_window, window, boost::shared_ptr<base> >("debug_window")
        .def("clear", &debug_window::clear)
        .def("get_log", &debug_window::get_log)
        .property("log", &debug_window::get_log)
        .def("print", &debug_window::print)
        .def("print", &debug_window::print1)
//        .scope
//        [
//          def("get", &debug_window::get),
//          def("init", &debug_window::init)
//        ]
    ]
  ];
  object lev = globals(L)["lev"];
  object classes = lev["classes"];
  object debug = lev["debug"];

//  debug["window"] = classes["debug_window"]["init"];

  globals(L)["package"]["loaded"]["lev.debug"] = true;
  return 0;
}

namespace lev
{

  bool debug_print(const std::string &message_utf8)
  {
    time_t t;
    struct tm *t_st;
    time(&t);
    t_st = localtime(&t);
    char buf[9];
    strftime(buf, 9, "%H:%M:%S", t_st);

    if (debug_window::get())
    {
      debug_window::get()->show();
      return debug_window::get()->print1(std::string("[") + buf + "]: " + message_utf8 + "\n");
    }
    printf("Debug Message (%s): %s\n", buf, message_utf8.c_str());
  }

  bool debug_print_lua(luabind::object obj)
  {
    return debug_print(util::tostring(obj));
  }

}

