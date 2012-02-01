/////////////////////////////////////////////////////////////////////////////
// Name:        src/prim.cpp
// Purpose:     source for primitive expressions
// Author:      Akiva Miura <akiva.miura@gmail.com>
// Created:     01/05/2011
// Copyright:   (C) 2010-2011 Akiva Miura
// Licence:     MIT License
/////////////////////////////////////////////////////////////////////////////

#include "prec.h"

#include "lev/prim.hpp"
#include "lev/util.hpp"
#include "register.hpp"

#include <boost/format.hpp>

int luaopen_lev_prim(lua_State *L)
{
  using namespace lev;
  using namespace luabind;

  open(L);
  globals(L)["require"]("lev.base");

  // primitives
  module(L, "lev")
  [
    namespace_("prim"),
    namespace_("classes")
    [
      class_<color, base, boost::shared_ptr<base> >("color")
        .property("a", &color::get_a, &color::set_a)
        .property("alpha", &color::get_a, &color::set_a)
        .property("b", &color::get_b, &color::set_b)
        .property("blue", &color::get_b, &color::set_b)
        .property("code", &color::get_code32)
        .property("code32", &color::get_code32)
        .property("codestr", &color::get_codestr)
        .property("g", &color::get_g, &color::set_g)
        .property("green", &color::get_g, &color::set_g)
        .property("r", &color::get_r, &color::set_r)
        .property("red", &color::get_r, &color::set_r)
        .property("str", &color::get_codestr)
        .property("string", &color::get_codestr)
        .scope
        [
          def("create_c", &color::create)
        ],
      class_<size, base, boost::shared_ptr<base> >("size")
        .def("assign", &size::assign)
        .def("assign", &size::assign_size)
        .property("d", &size::get_d, &size::set_d)
        .property("depth", &size::get_h, &size::set_d)
        .property("h", &size::get_h, &size::set_h)
        .property("height", &size::get_h, &size::set_h)
        .property("w", &size::get_w, &size::set_w)
        .property("width", &size::get_w, &size::set_w)
        .scope
        [
          def("create_c", &size::create)
        ],
      class_<vector, base, boost::shared_ptr<base> >("vector")
        .def("assign", &vector::assign)
        .def("assign", &vector::assign_vector)
//        .def(self + vector())
        .property("x", &vector::get_x, &vector::set_x)
        .property("y", &vector::get_y, &vector::set_y)
        .property("z", &vector::get_z, &vector::set_z)
        .scope
        [
          def("create_c", &vector::create)
        ],
      class_<point, base, boost::shared_ptr<base> >("point")
        .property("c", &point::get_color, &point::set_color)
        .property("col", &point::get_color, &point::set_color)
        .property("color", &point::get_color, &point::set_color)
        .property("colour", &point::get_color, &point::set_color)
        .property("p", &point::get_vertex, &point::set_vertex)
        .property("pos", &point::get_vertex, &point::set_vertex)
        .property("position", &point::get_vertex, &point::set_vertex)
        .property("vec", &point::get_vertex, &point::set_vertex)
        .property("vector", &point::get_vertex, &point::set_vertex)
        .property("vert", &point::get_vertex, &point::set_vertex)
        .property("vertex", &point::get_vertex, &point::set_vertex)
        .scope
        [
          def("create_c", &point::create)
        ],
      class_<rect, base, boost::shared_ptr<base> >("rect")
        .def("assign", &rect::assign)
        .def("assign", &rect::assign_position_size)
        .def("assign", &rect::assign_rect)
        .property("b", &rect::get_bottom, &rect::set_bottom)
        .property("bottom", &rect::get_bottom, &rect::set_bottom)
        .property("h", &rect::get_h, &rect::set_h)
        .property("height", &rect::get_h, &rect::set_h)
        .def("include", &rect::include)
        .def("include", &rect::include1)
        .property("l", &rect::get_left, &rect::set_left)
        .property("left", &rect::get_left, &rect::set_left)
        .property("p", &rect::get_position, &rect::set_position)
        .property("pos", &rect::get_position, &rect::set_position)
        .property("position", &rect::get_position, &rect::set_position)
        .property("r", &rect::get_right, &rect::set_right)
        .property("right", &rect::get_right, &rect::set_right)
        .property("s", &rect::get_size, &rect::set_size)
        .property("size", &rect::get_size, &rect::set_size)
        .property("sz", &rect::get_size, &rect::set_size)
        .property("t", &rect::get_top, &rect::set_top)
        .property("top", &rect::get_top, &rect::set_top)
        .property("v", &rect::get_position, &rect::set_position)
        .property("vec", &rect::get_position, &rect::set_position)
        .property("vector", &rect::get_position, &rect::set_position)
        .property("x", &rect::get_x, &rect::set_x)
        .property("y", &rect::get_y, &rect::set_y)
        .property("w", &rect::get_w, &rect::set_w)
        .property("width", &rect::get_w, &rect::set_w)
        .scope
        [
          def("create_c", &rect::create)
        ]
    ]
  ];
  object lev = globals(L)["lev"];
  object classes = lev["classes"];
  object prim = lev["prim"];

  register_to(classes["color"],  "create", &color::create_l);
  register_to(classes["point"],  "create", &point::create_l);
  register_to(classes["rect"],   "create", &rect::create_l);
  register_to(classes["size"],   "create", &size::create_l);
  register_to(classes["vector"], "create", &vector::create_l);

  prim["color"]    = classes["color"]["create"];
  prim["colour"]   = classes["color"]["create"];
  prim["point"]    = classes["point"]["create"];
  prim["position"] = classes["vector"]["create"];
  prim["rect"]     = classes["rect"]["create"];
  prim["size"]     = classes["size"]["create"];
  prim["vertex"]   = classes["vector"]["create"];
  prim["vector"]   = classes["vector"]["create"];

  prim["black"] = classes["color"]["create"](  0,   0,   0);
  prim["blue"]  = classes["color"]["create"](  0,   0, 255);
  prim["green"] = classes["color"]["create"](  0, 255,   0);
  prim["red"]   = classes["color"]["create"](255,   0,   0);
  prim["trans"] = classes["color"]["create"](  0,   0,   0,   0);
  prim["transparent"] = classes["color"]["create"](0, 0, 0, 0);
  prim["white"] = classes["color"]["create"](255, 255, 255);

  globals(L)["package"]["loaded"]["lev.prim"] = prim;
  return 0;
}

namespace lev
{

  color::color(const color &orig)
    : r(orig.r), g(orig.g), b(orig.b), a(orig.a) { }

  color::color(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
    : r(r), g(g), b(b), a(a) { }

  color::color(unsigned long argb_code)
  {
    a = (argb_code & 0xFF000000) >> 24;
    r = (argb_code & 0x00FF0000) >> 16;
    g = (argb_code & 0x0000FF00) >>  8;
    b = (argb_code & 0x000000FF) >>  0;
  }

  boost::shared_ptr<color> color::create(unsigned char r, unsigned char g,
                                         unsigned char b, unsigned char a)
  {
    boost::shared_ptr<color> c;
    try {
      c.reset(new color(r, g, b, a));
      if (! c) { throw -1; }
    }
    catch (...) {
      c.reset();
      fprintf(stderr, "error on color instance creation\n");
    }
    return c;
  }

  int color::create_l(lua_State *L)
  {
    using namespace luabind;
    unsigned char r = 0, g = 0, b = 0, a = 255;
    const char *name = NULL;

    object t = util::get_merged(L, 1, -1);

    if (t["name"]) { name = object_cast<const char *>(t["name"]); }
    else if (t["n"]) { name = object_cast<const char *>(t["n"]); }
    else if (t["lua.string1"]) { name = object_cast<const char *>(t["lua.string1"]); }

    if (name)
    {
      object o;
      object func = globals(L)["lev"]["classes"]["color"]["create_c"];

      if (t["alpha"]) { a = object_cast<unsigned char>(t["alpha"]); }
      else if (t["a"]) { a = object_cast<unsigned char>(t["a"]); }
      else if (t["lua.number1"]) { a = object_cast<unsigned char>(t["lua.number1"]); }

      if      (strstr(name, "black")) { o = func(  0,   0,   0, a); }
      else if (strstr(name, "blue"))  { o = func(  0,   0, 255, a); }
      else if (strstr(name, "green")) { o = func(  0, 255,   0, a); }
      else if (strstr(name, "red"))   { o = func(255,   0,   0, a); }
      else if (strstr(name, "trans")) { o = func(  0,   0,   0, 0); }
      else if (strstr(name, "white")) { o = func(255, 255, 255, a); }
      o.push(L);
      return 1;
    }

    if (t["red"]) { r = object_cast<unsigned char>(t["red"]); }
    else if (t["r"]) { r = object_cast<unsigned char>(t["r"]); }
    else if (t["lua.number1"]) { r = object_cast<unsigned char>(t["lua.number1"]); }

    if (t["green"]) { g = object_cast<unsigned char>(t["green"]); }
    else if (t["g"]) { g = object_cast<unsigned char>(t["g"]); }
    else if (t["lua.number2"]) { g = object_cast<unsigned char>(t["lua.number2"]); }

    if (t["blue"]) { b = object_cast<unsigned char>(t["blue"]); }
    else if (t["b"]) { b = object_cast<unsigned char>(t["b"]); }
    else if (t["lua.number3"]) { b = object_cast<unsigned char>(t["lua.number3"]); }

    if (t["alpha"]) { a = object_cast<unsigned char>(t["alpha"]); }
    else if (t["a"]) { a = object_cast<unsigned char>(t["a"]); }
    else if (t["lua.number4"]) { a = object_cast<unsigned char>(t["lua.number4"]); }

    object o = globals(L)["lev"]["classes"]["color"]["create_c"](r, g, b, a);
    o.push(L);
    return 1;
  }

  unsigned long color::get_code32() const
  {
    return (a << 24) | (r << 16) | (g << 8) | (b << 0);
  }

  const std::string color::get_codestr() const
  {
    char str[9];
    sprintf(str, "%02x%02x%02x%02x", a, r, g, b);
    return str;
  }

  bool size::assign(int new_w, int new_h, int new_d)
  {
    this->w = new_w;
    this->h = new_h;
    this->d = new_d;
    return true;
  }

  boost::shared_ptr<size> size::create(int w, int h, int d)
  {
    boost::shared_ptr<size> s;
    try {
      s.reset(new size(w, h, d));
      if (! s) { throw -1; }
    }
    catch (...) {
      s.reset();
      fprintf(stderr, "error on size instance creation\n");
    }
    return s;
  }


  int size::create_l(lua_State *L)
  {
    using namespace luabind;
    int w = 0, h = 0, d = 0;

    object t = util::get_merged(L, 1, -1);

    if (t["width"]) { w = object_cast<int>(t["width"]); }
    else if (t["w"]) { w = object_cast<int>(t["w"]); }
    else if (t["lua.number1"]) { w = object_cast<int>(t["lua.number1"]); }

    if (t["height"]) { h = object_cast<int>(t["height"]); }
    else if (t["h"]) { h = object_cast<int>(t["h"]); }
    else if (t["lua.number2"]) { h = object_cast<int>(t["lua.number2"]); }

    if (t["depth"]) { d = object_cast<int>(t["depth"]); }
    else if (t["d"]) { d = object_cast<int>(t["d"]); }
    else if (t["lua.number3"]) { d = object_cast<int>(t["lua.number3"]); }

    object o = globals(L)["lev"]["classes"]["size"]["create_c"](w, h, d);
    o.push(L);
    return 1;
  }


  vector::vector(const vector &orig)
    : x(orig.x), y(orig.y), z(orig.z) { }

  bool vector::assign(int new_x, int new_y, int new_z)
  {
    this->x = new_x;
    this->y = new_y;
    this->z = new_z;
    return true;
  }

  boost::shared_ptr<vector> vector::create(int x, int y, int z)
  {
    boost::shared_ptr<vector> vec;
    try {
      vec.reset(new vector(x, y, z));
      if (! vec) { throw -1; }
    }
    catch (...) {
      vec.reset();
      fprintf(stderr, "error on vector instance creation\n");
    }
    return vec;
  }

  int vector::create_l(lua_State *L)
  {
    using namespace luabind;
    int x = 0, y = 0, z = 0;

    object t = util::get_merged(L, 1, -1);

    if (t["x"]) { x = object_cast<int>(t["x"]); }
    else if (t["lua.number1"]) { x = object_cast<int>(t["lua.number1"]); }

    if (t["y"]) { y = object_cast<int>(t["y"]); }
    else if (t["lua.number2"]) { y = object_cast<int>(t["lua.number2"]); }

    if (t["z"]) { z = object_cast<int>(t["z"]); }
    else if (t["lua.number3"]) { z = object_cast<int>(t["lua.number3"]); }

    object o = globals(L)["lev"]["classes"]["vector"]["create_c"](x, y, z);
    o.push(L);
    return 1;
  }

  point::point() : vertex(), col() { }

  point::~point()
  { }

  bool point::clear_color()
  {
    col.reset();
    return true;
  }

  boost::shared_ptr<point> point::create(vector *vec, color *col)
  {
    boost::shared_ptr<point> pt;
    try {
      pt.reset(new point);

      if (vec) { pt->vertex.reset(new vector(*vec)); }
      else { pt->vertex.reset(new vector); }

      if (col) { pt->col.reset(new color(*col)); }
//      else { pt->col.reset(new color); }
    }
    catch (...) {
      pt.reset();
      fprintf(stderr, "error on point instance creation\n");
    }
    return pt;
  }

  int point::create_l(lua_State *L)
  {
    using namespace luabind;
    object ver, col;
    unsigned char r = 0, g = 0, b = 0, a = 255;

    try {
      object t = util::get_merged(L, 1, -1);

      if (t["lua.userdata1"])
      {
        switch ( object_cast<int>(t["lua.userdata1"]["type_id"]) )
        {
          case LEV_TCOLOR:
            col = t["lua.userdata1"];
            break;
          case LEV_TVECTOR:
            ver = t["lua.userdata1"];
            break;
        }
      }
      if (t["lua.userdata2"])
      {
        switch ( object_cast<int>(t["lua.userdata2"]["type_id"]) )
        {
          case LEV_TCOLOR:
            col = t["lua.userdata2"];
            break;
          case LEV_TVECTOR:
            ver = t["lua.userdata2"];
            break;
        }
      }

      if (t["vertex"]) { ver = t["vertex"]; }
      else if (t["ver"]) { ver = t["ver"]; }
      else if (t["v"]) { ver = t["v"]; }

      if (! ver) { ver = globals(L)["lev"]["prim"]["vector"](t); }

      if (t["color"]) { col = t["color"]; }
      else if (t["col"]) { col = t["col"]; }
      else if (t["c"]) { col = t["c"]; }

      if (! col)
      {
        if (t["red"] || t["r"] || t["green"] || t["g"] || t["blue"] || t["b"])
        {
          col = globals(L)["lev"]["prim"]["color"](t);
        }
      }

      object o = globals(L)["lev"]["classes"]["point"]["create_c"](ver, col);
      o.push(L);
      return 1;
    }
    catch (...) {
      fprintf(stderr, "error on point instance creation\n");
      fprintf(stderr, "error message: %s\n", lua_tostring(L, -1));
    }
  }

  bool point::set_color(color *c)
  {
    try {
      if (c) { col.reset(new color(*c)); }
      else { col.reset(); }
    }
    catch (...) {
      fprintf(stderr, "error on point color setting\n");
      return false;
    }
    return true;
  }

  bool point::set_vertex(vector *v)
  {
    if (! v) { return false; }
    try {
      vertex.reset(new vector(*v));
    }
    catch (...) {
      fprintf(stderr, "error on point vertex setting\n");
      return false;
    }
    return true;
  }

  rect::rect(int x, int y, int w, int h)
    : pos(x, y), sz(w, h)
  { }

  rect::rect(const vector &p, const size &s)
    : pos(p), sz(s)
  { }

  rect::rect(const rect &r)
    : pos(r.pos), sz(r.sz)
  { }

  rect::~rect()
  {
  }

  bool rect::assign(int x, int y, int w, int h)
  {
    pos.assign(x, y);
    sz.assign(w, h);
    return true;
  }

  bool rect::assign_position_size(const vector &v, const size &s)
  {
    pos.assign_vector(v);
    sz.assign_size(s);
    return true;
  }

  bool rect::assign_rect(const rect &r)
  {
    return assign_position_size(r.pos, r.sz);
  }

  boost::shared_ptr<rect> rect::create(int x, int y, int w, int h)
  {
    boost::shared_ptr<rect> r;
    try {
      r.reset(new rect(x, y, w, h));
      if (! r) { throw -1; }
    }
    catch (...) {
      r.reset();
      fprintf(stderr, "error on rect instance creation\n");
    }
    return r;
  }

  int rect::create_l(lua_State *L)
  {
    using namespace luabind;
    int x = 0, y = 0, w = 0, h = 0;

    object t = util::get_merged(L, 1, -1);

    if (t["x"]) { x = object_cast<int>(t["x"]); }
    else if (t["lua.number1"]) { x = object_cast<int>(t["lua.number1"]); }

    if (t["y"]) { y = object_cast<int>(t["y"]); }
    else if (t["lua.number2"]) { y = object_cast<int>(t["lua.number2"]); }

    if (t["width"]) { w = object_cast<int>(t["width"]); }
    else if (t["w"]) { w = object_cast<int>(t["w"]); }
    else if (t["lua.number3"]) { w = object_cast<int>(t["lua.number3"]); }

    if (t["height"]) { h = object_cast<int>(t["height"]); }
    else if (t["h"]) { h = object_cast<int>(t["h"]); }
    else if (t["lua.number4"]) { h = object_cast<int>(t["lua.number4"]); }

    if (t["lev.prim.vector1"])
    {
      object vec1 = t["lev.prim.vector1"];
      if (t["lev.prim.vector2"])
      {
        object vec2 = t["lev.prim.vector2"];
        int x1 = object_cast<int>(vec1["x"]);
        int y1 = object_cast<int>(vec1["y"]);
        int x2 = object_cast<int>(vec2["x"]);
        int y2 = object_cast<int>(vec2["y"]);
        int max_x = x1 > x2 ? x1 : x2;
        int min_x = x1 < x2 ? x1 : x2;
        int max_y = y1 > y2 ? y1 : y2;
        int min_y = y1 < y2 ? y1 : y2;
        x = min_x;
        y = min_y;
        w = max_x - min_x;
        h = max_y - min_y;
      }
      else
      {
        x = object_cast<int>(vec1["x"]);
        y = object_cast<int>(vec1["y"]);
      }
    }

    if (t["lev.prim.size1"])
    {
      object sz = t["lev.prim.size1"];
      w = object_cast<int>(sz["w"]);
      h = object_cast<int>(sz["h"]);
    }

    object o = globals(L)["lev"]["classes"]["rect"]["create_c"](x, y, w, h);
    o.push(L);
    return 1;
  }

  bool rect::include(int x, int y) const
  {
    if (x < get_left())   { return false; }
    if (x > get_right())  { return false; }
    if (y < get_top())    { return false; }
    if (y > get_bottom()) { return false; }
    return true;
  }

  bool rect::set_position(const vector &vec)
  {
    return pos.assign_vector(vec);
  }

  bool rect::set_size(const size &new_size)
  {
    return sz.assign_size(new_size);
  }

  const rect& rect::operator=(const rect &rhs)
  {
    assign_rect(rhs);
    return *this;
  }

}

