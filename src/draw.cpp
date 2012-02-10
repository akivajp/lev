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
#include "lev/window.hpp"
#include "register.hpp"

// libraries
#include <boost/scoped_array.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

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
        .def("compile", &drawable::compile)
        .def("compile", &drawable::compile0)
        .def("draw_on", &drawable::draw_on_image)
        .def("draw_on", &drawable::draw_on_image1)
        .def("draw_on", &drawable::draw_on_image3)
        .def("draw_on", &drawable::draw_on_screen)
        .def("draw_on", &drawable::draw_on_screen1)
        .def("draw_on", &drawable::draw_on_screen3)
        .property("h", &drawable::get_h)
        .property("height", &drawable::get_h)
        .property("is_compiled", &drawable::is_compiled)
        .property("is_texturized", &drawable::is_texturized)
        .def("texturize", &drawable::texturize)
        .def("texturize", &drawable::texturize0)
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

}

