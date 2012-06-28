/////////////////////////////////////////////////////////////////////////////
// Name:        src/draw.cpp
// Purpose:     source for drawing features
// Author:      Akiva Miura <akiva.miura@gmail.com>
// Modified by:
// Created:     12/23/2010
// Copyright:   (C) 2010-2011 Akiva Miura
// Licence:     MIT License
/////////////////////////////////////////////////////////////////////////////

// pre-compiled header
#include "prec.h"

// declarations
#include "lev/draw.hpp"

// dependencies
#include "lev/debug.hpp"
#include "lev/font.hpp"
#include "lev/image.hpp"
#include "lev/map.hpp"
#include "lev/util.hpp"
#include "lev/screen.hpp"

// libraries

int luaopen_lev_draw(lua_State *L)
{
  using namespace lev;
  using namespace luabind;

  open(L);
  globals(L)["require"]("lev");

  module(L, "lev")
  [
    namespace_("classes")
    [
      class_<drawable, base, boost::shared_ptr<base> >("drawable")
        .def("as_drawable", &drawable::as_drawable_from_animation)
        .def("as_drawable", &drawable::as_drawable_from_bitmap)
        .def("as_drawable", &drawable::as_drawable_from_layout)
        .def("as_drawable", &drawable::as_drawable_from_map)
        .def("as_drawable", &drawable::as_drawable_from_screen)
        .def("as_drawable", &drawable::as_drawable_from_transition)
        .def("draw_on", &drawable::draw_on)
        .def("draw_on", &drawable::draw_on1)
        .def("draw_on", &drawable::draw_on3)
        .property("ascent", &drawable::get_ascent)
        .property("descent", &drawable::get_descent, &drawable::set_descent)
        .property("h", &drawable::get_h)
        .property("height", &drawable::get_h)
        .property("w", &drawable::get_w)
        .property("width", &drawable::get_w)
    ]
  ];
//  object lev = globals(L)["lev"];
//  object classes = lev["classes"];

  globals(L)["package"]["loaded"]["lev.draw"] = true;
  return 0;
}

namespace lev
{

  boost::shared_ptr<drawable>
    drawable::as_drawable_from_animation(boost::shared_ptr<animation> d)
  {
    return d;
  }

  boost::shared_ptr<drawable>
    drawable::as_drawable_from_bitmap(boost::shared_ptr<bitmap> d)
  {
    return d;
  }

  boost::shared_ptr<drawable>
    drawable::as_drawable_from_layout(boost::shared_ptr<layout> d)
  {
    return d;
  }

  boost::shared_ptr<drawable>
    drawable::as_drawable_from_map(boost::shared_ptr<map> d)
  {
    return d;
  }

  boost::shared_ptr<drawable>
    drawable::as_drawable_from_screen(boost::shared_ptr<screen> d)
  {
    return d;
  }

  boost::shared_ptr<drawable>
    drawable::as_drawable_from_transition(boost::shared_ptr<transition> d)
  {
    return d;
  }

  bool drawable::set_descent(int d)
  {
    if (d < 0) { return false; }
    descent = d;
    return true;
  }


  boost::shared_ptr<spacer> spacer::create(int w, int h)
  {
    boost::shared_ptr<spacer> s;
    try {
      s.reset(new spacer(w, h));
      if (! s) { throw -1; }
    }
    catch (...) {
      s.reset();
      lev::debug_print("error on spacer instance creation");
    }
    return s;
  }

}

