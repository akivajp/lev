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
#include "lev/entry.hpp"
#include "lev/font.hpp"
#include "lev/image.hpp"
#include "lev/map.hpp"
#include "lev/util.hpp"

// libraries

namespace lev
{

  spacer::ptr spacer::create(int w, int h, int descent)
  {
    spacer::ptr s;
    try {
      s.reset(new spacer(w, h, descent));
      if (! s) { throw -1; }
    }
    catch (...) {
      s.reset();
      lev::debug_print("error on spacer instance creation");
    }
    return s;
  }

  bool canvas::clear_color(color::ptr c)
  {
    if (! c) { return false; }
    return clear(c->get_r(), c->get_g(), c->get_b(), c->get_a());
  }

  int canvas::draw_l(lua_State *L)
  {
    using namespace luabind;

    try {
      luaL_checktype(L, 1, LUA_TUSERDATA);
      canvas *cv = object_cast<canvas *>(object(from_stack(L, 1)));
      object t = util::get_merged(L, 2, -1);
      int x = 0, y = 0;
      unsigned char a = 255;

      if (t["x"]) { x = object_cast<int>(t["x"]); }
      else if (t["lua.number1"]) { x = object_cast<int>(t["lua.number1"]); }

      if (t["y"]) { y = object_cast<int>(t["y"]); }
      else if (t["lua.number2"]) { y = object_cast<int>(t["lua.number2"]); }

      if (t["lua.number3"]) { a = object_cast<int>(t["lua.number3"]); }
      else if (t["alpha"]) { a = object_cast<int>(t["alpha"]); }
      else if (t["a"]) { a = object_cast<int>(t["a"]); }

      if (t["lev.drawable1"])
      {
//printf("IMAGE CAST!\n");
        object obj = t["lev.drawable1"];
//printf("OBJECT: %s\n", object_cast<const char *>(obj["type_name"]));
        boost::shared_ptr<drawable> src = object_cast<boost::shared_ptr<drawable> >(obj["to_drawable"](obj));
        cv->draw(src, x, y, a);
//printf("IMAGE END!\n");
      }
      else if (t["lev.font1"])
      {
        int spacing = 1;
        boost::shared_ptr<color> c;
        font *f = object_cast<font *>(t["lev.font1"]);
        const char *str = NULL;

        if (t["spacing"]) { spacing = object_cast<int>(t["spacing"]); }
        else if (t["space"]) { spacing = object_cast<int>(t["space"]); }
        else if (t["s"]) { spacing = object_cast<int>(t["s"]); }
        else if (t["lua.number3"]) { spacing = object_cast<int>(t["lua.number3"]); }

        if (t["color"]) { c = object_cast<boost::shared_ptr<color> >(t["color"]); }
        else if (t["c"]) { c = object_cast<boost::shared_ptr<color> >(t["c"]); }
        else if (t["lev.color1"]) { c = object_cast<boost::shared_ptr<color> >(t["lev.color1"]); }

        if (t["lua.string1"]) { str = object_cast<const char *>(t["lua.string1"]); }

        if (! str)
        {
          lua_pushboolean(L, false);
          return 1;
        }

        boost::shared_ptr<bitmap> r = f->rasterize_utf8(str, c);
        if (! r.get()) { throw -2; }
        cv->draw(r, x, y, 255);
//        cv->draw_bitmap(r, x, y, 255);
      }
      else
      {
        lua_pushboolean(L, false);
        return 1;
      }
      lua_pushboolean(L, true);
    }
    catch (std::exception &e)
    {
      lev::debug_print(e.what());
      lev::debug_print("error on drawing");
    }
    catch (...) {
      lev::debug_print("error on drawing");
    }
    return 1;
  }

  bool canvas::fill_circle(int cx, int cy, int radius, color::ptr filling)
  {
    if (! filling) { return false; }
    color::ptr half = filling->clone();
    half->set_a(half->get_a() / 2);
    for (int y = -radius; y <= radius; y++)
    {
      for (int x = -radius; x <= radius; x++)
      {
        int dist2 = x * x + y * y;
        int rad2 = radius * radius;
        if (dist2 < rad2)
        {
          draw_pixel(cx + x, cy + y, *filling);
        }
        else if (dist2 <= rad2)
        {
          draw_pixel(cx + x, cy + y, *half);
        }
      }
    }
    return true;
  }

  bool canvas::fill_rect(int offset_x, int offset_y, int w, int h, color::ptr filling)
  {
    if (! filling) { return false; }
    for (int y = 0; y < h; y++)
    {
      for (int x = 0; x < w; x++)
      {
        draw_pixel(offset_x + x, offset_y + y, *filling);
      }
    }
    return true;
  }

//
//  bool bitmap::stroke_circle(int x, int y, int radius, color *border, int width)
//  {
//    wxBitmap tmp(get_w(), get_h(), 32);
//    try {
//      wxMemoryDC mdc(tmp);
//      mdc.SetPen(wxPen(wxColour(255, 255, 255, 255), width));
//      mdc.SetBrush(wxColour(0, 0, 0, 255));
//      mdc.SetBackground(wxColour(0, 0, 0, 255));
//      mdc.Clear();
//      mdc.DrawCircle(x, y, radius);
//    }
//    catch (...) {
//      return false;
//    }
//    return bitmap_draw_mask(this, &tmp, border);
//  }

  bool canvas::stroke_line(int x1, int y1, int x2, int y2, boost::shared_ptr<color> c, int width,
                           const std::string &style)
  {
    if (! c) { return false; }
    if (x2 < x1)
    {
      int tmp = x1;
      x2 = x1;
      x1 = tmp;
      tmp = y1;
      y2 = y1;
      y1 = tmp;
    }
    try {
      if (style == "dot")
      {
        for (int x = x1; x < x2; x+=2)
        {
          int y = y1;
          if (x > x1)
          {
            y = (y2 - y1) * (x2 - x1) / (x - x1) + y1;
          }
          draw_pixel(x, y, *c);
        }
      }
      else
      {
        for (int x = x1; x < x2; x++)
        {
          int y = y1;
          if (x > x1)
          {
            y = (y2 - y1) * (x2 - x1) / (x - x1) + y1;
          }
          draw_pixel(x, y, *c);
        }
      }
    }
    catch (...) {
      return false;
    }
    return true;
  }

  bool canvas::stroke_rect(int offset_x, int offset_y, int w, int h, color::ptr border, int width)
  {
    if (! border) { return false; }
    for (int y = 0; y < h; y++)
    {
      for (int x = 0; x < w; x++)
      {
        if (0 <= x && x < width ||
            0 <= y && y < width ||
            w - width <= x && x < w ||
            h - width <= y && y < h)
        {
          draw_pixel(offset_x + x, offset_y + y, *border);
        }
      }
    }
    return true;
  }

}

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
        .property("ascent", &drawable::get_ascent)
        .def("compile", &drawable::compile)
        .def("compile", &drawable::compile0)
        .property("descent", &drawable::get_descent, &drawable::set_descent)
        .def("draw_on", &drawable::draw_on)
        .def("draw_on", &drawable::draw_on1)
        .def("draw_on", &drawable::draw_on3)
        .property("h", &drawable::get_h)
        .property("height", &drawable::get_h)
        .property("is_compiled", &drawable::is_compiled)
        .property("is_texturized", &drawable::is_texturized)
        .def("texturize", &drawable::texturize)
        .def("texturize", &drawable::texturize0)
        .def("to_drawable", &drawable::to_drawable)
        .property("w", &drawable::get_w)
        .property("width", &drawable::get_w),
      class_<canvas, drawable, boost::shared_ptr<drawable> >("canvas")
        .def("blit", &canvas::blit)
        .def("blit", &canvas::blit1)
        .def("blit", &canvas::blit2)
        .def("blit", &canvas::blit3)
        .def("blit", &canvas::blit4)
        .def("clear", &canvas::clear)
        .def("clear", &canvas::clear0)
        .def("clear", &canvas::clear3)
        .def("clear", &canvas::clear_color)
        .def("draw_pixel", &canvas::draw_pixel)
        .def("fill_circle", &canvas::fill_circle)
        .def("fill_rect", &canvas::fill_rect)
        .def("fill_rectangle", &canvas::fill_rect)
        .def("get_color", &canvas::get_pixel)
        .def("get_pixel", &canvas::get_pixel)
//        .def("stroke_circle", &canvas::stroke_circle)
        .def("stroke_line", &canvas::stroke_line)
        .def("stroke_line", &canvas::stroke_line6)
        .def("stroke_rect", &canvas::stroke_rect)
        .def("stroke_rectangle", &canvas::stroke_rect)
    ]
  ];
  object lev = globals(L)["lev"];
  object classes = lev["classes"];

  register_to(classes["canvas"], "draw", &canvas::draw_l);

  globals(L)["package"]["loaded"]["lev.draw"] = true;
  return 0;
}

