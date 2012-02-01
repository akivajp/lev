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
#include "lev/font.hpp"
#include "lev/image.hpp"
#include "lev/map.hpp"
#include "lev/util.hpp"
#include "lev/window.hpp"
#include "register.hpp"

// libraries
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
        .property("width", &drawable::get_w),
      class_<screen, base, boost::shared_ptr<base> >("screen")
        .def("blit", &screen::blit)
        .def("blit", &screen::blit1)
        .def("blit", &screen::blit2)
        .def("blit", &screen::blit3)
        .def("blit", &screen::blit4)
        .def("clear", &screen::clear)
        .def("clear", &screen::clear_color)
        .def("clear", &screen::clear_color1)
        .def("clear", &screen::clear_color3)
        .def("draw_pixel", &screen::draw_pixel)
        .def("draw_raster", &screen::draw_raster)
        .def("enable_alpha_blending", &screen::enable_alpha_blending0)
        .def("enable_alpha_blending", &screen::enable_alpha_blending)
        .def("flip", &screen::flip)
        .def("map2d", &screen::map2d)
        .def("map2d", &screen::map2d_auto)
        .def("set_current", &screen::set_current)
        .def("swap", &screen::swap)
//      class_<canvas, control>("canvas")
//        .def("clear", &canvas::clear)
//        .def("clear", &canvas::clear_color)
//        .def("clear", &canvas::clear_color1)
//        .def("clear", &canvas::clear_color3)
//        .def("compile", &canvas::compile)
//        .def("compile", &canvas::compile1)
//        .def("draw_image", &canvas::draw_image)
//        .def("draw_image", &canvas::draw_image1)
//        .def("draw_image", &canvas::draw_image3)
//        .def("draw_point", &canvas::draw_point)
//        .def("enable_alpha_blending", &canvas::enable_alpha_blending0)
//        .def("enable_alpha_blending", &canvas::enable_alpha_blending)
//        .def("fill_rect", &canvas::fill_rect)
//        .def("flush", &canvas::flush)
//        .def("line",  &canvas::line)
//        .def("map2d", &canvas::map2d)
//        .def("map2d", &canvas::map2d_auto)
//        .def("redraw", &canvas::redraw)
//        .def("print", &canvas::print)
//        .def("set_current", &canvas::set_current)
//        .def("swap", &canvas::swap)
//        .def("texturize", &canvas::texturize)
//        .def("texturize", &canvas::texturize1)
//        .scope
//        [
//          def("create_c", &canvas::create, adopt(result))
//        ]
    ]
  ];
  object lev = globals(L)["lev"];
  object classes = lev["classes"];
  register_to(classes["screen"], "draw", &screen::draw_l);
//
//  register_to(classes["canvas"], "create", &canvas::create_l);
//  register_to(classes["canvas"], "draw", &canvas::draw_l);
//  gui["canvas"] = classes["canvas"]["create"];

  globals(L)["package"]["loaded"]["lev.draw"] = true;
  return 0;
}

namespace lev
{

  static SDL_GLContext cast_ctx(void *obj) { return (SDL_GLContext)obj; }

  screen::screen() : base(), _obj(NULL) { }

  screen::~screen()
  {
    if (_obj)
    {
      SDL_GL_DeleteContext((SDL_GLContext)_obj);
    }
  }

  bool screen::blit(int dst_x, int dst_y, image *src,
                    int src_x, int src_y, int w, int h, unsigned char alpha)
  {
    if (src == NULL) { return false; }

    int src_h = src->get_h();
    int src_w = src->get_w();
    if (w < 0) { w = src_w; }
    if (h < 0) { h = src_h; }
    glBegin(GL_POINTS);
      for (int y = 0; y < h; y++)
      {
        for (int x = 0; x < w; x++)
        {
          int real_src_x = src_x + x;
          int real_src_y = src_y + y;
          if (real_src_x < 0 || real_src_x >= src_w || real_src_y < 0 || real_src_y >= src_h)
          {
            continue;
          }
          boost::shared_ptr<color> pixel(src->get_pixel(real_src_x, real_src_y));
          if (! pixel) { continue; }
          unsigned char a = (unsigned short)pixel->get_a() * alpha / 255;
          if (a > 0)
          {
            glColor4ub(pixel->get_r(), pixel->get_g(), pixel->get_b(), a);
            glVertex2i(dst_x + x, dst_y + y);
          }
        }
      }
    glEnd();
    return true;
  }

  bool screen::clear_color(unsigned char r, unsigned char g,
                   unsigned char b, unsigned char a)
  {
    glClearColor(r / 255.0, g / 255.0, b / 255.0, a / 255.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//    glClear(GL_COLOR_BUFFER_BIT);
    return true;
  }

  bool screen::clear_color1(const color &c)
  {
    return clear_color(c.get_r(), c.get_g(), c.get_b(), c.get_a());
  }

//  screen* screen::create(window *holder)
  boost::shared_ptr<screen> screen::create(boost::shared_ptr<window> holder)
  {
    boost::shared_ptr<screen> screen;

    if (! holder) { return screen; }
    try {
      screen.reset(new lev::screen);
      if (! screen) { throw -1; }
      screen->_obj = SDL_GL_CreateContext((SDL_Window *)holder->get_rawobj());
      if (! screen->_obj) { throw -2; }
      screen->holder = holder;

      SDL_GL_SetSwapInterval(0);
//      glMatrixMode(GL_PROJECTION);
//      glLoadIdentity();
//      glOrtho(0.0f, 640, 480, 0.0f, 0.0f, 1000.0f);
//      glMatrixMode(GL_MODELVIEW);

      glOrtho(0, holder->get_w(), holder->get_h(), 0, -1, 1);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    catch (...) {
      screen.reset();
      fprintf(stderr, "error on screen instance creation\n");
    }
    return screen;
  }

  bool screen::draw(drawable *src, int x, int y, unsigned char alpha)
  {
    if (! src) { return false; }
    return src->draw_on_screen(this, x, y, alpha);
  }

  int screen::draw_l(lua_State *L)
  {
    using namespace luabind;

    try {
      luaL_checktype(L, 1, LUA_TUSERDATA);
      screen *s = object_cast<screen *>(object(from_stack(L, 1)));
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

      if (t["lev.image1"])
      {
        drawable *src = object_cast<drawable *>(t["lev.image1"]);
        s->draw(src, x, y, a);
      }
      else if (t["lev.image.map1"])
      {
        drawable *src = object_cast<drawable *>(t["lev.image.map1"]);
        s->draw(src, x, y, a);
      }
      else if (t["lev.image.layout1"])
      {
        drawable *src = object_cast<drawable *>(t["lev.image.layout1"]);
        s->draw(src, x, y, a);
      }
      else if (t["lev.image.texture1"])
      {
        drawable *src = object_cast<drawable *>(t["lev.image.texture1"]);
        s->draw(src, x, y, a);
      }
      else if (t["lev.image.transition1"])
      {
        drawable *src = object_cast<drawable *>(t["lev.image.transition1"]);
        s->draw(src, x, y, a);
      }
      else if (t["lev.raster1"])
      {
        color *c = NULL;
        raster *r = object_cast<raster *>(t["lev.raster1"]);

        if (t["color"]) { c = object_cast<color *>(t["color"]); }
        else if (t["c"]) { c = object_cast<color *>(t["c"]); }
        else if (t["lev.prim.color1"]) { c = object_cast<color *>(t["lev.prim.color1"]); }

        s->draw_raster(r, x, y, c);
      }
      else if (t["lev.font1"])
      {
        int spacing = 1;
        color *c = NULL;
        font *f = object_cast<font *>(t["lev.font1"]);
        const char *str = NULL;

        if (t["spacing"]) { spacing = object_cast<int>(t["spacing"]); }
        else if (t["space"]) { spacing = object_cast<int>(t["space"]); }
        else if (t["s"]) { spacing = object_cast<int>(t["s"]); }
        else if (t["lua.number3"]) { spacing = object_cast<int>(t["lua.number3"]); }

        if (t["color"]) { c = object_cast<color *>(t["color"]); }
        else if (t["c"]) { c = object_cast<color *>(t["c"]); }
        else if (t["lev.prim.color1"]) { c = object_cast<color *>(t["lev.prim.color1"]); }

        if (t["lua.string1"]) { str = object_cast<const char *>(t["lua.string1"]); }

        if (! str)
        {
          lua_pushboolean(L, false);
          return 1;
        }

        boost::shared_ptr<raster> r = f->rasterize_utf8(str, spacing);
        if (! r.get()) { throw -2; }
        s->draw_raster(r.get(), x, y, c);
      }
      else
      {
        lua_pushboolean(L, false);
        return 1;
      }
      lua_pushboolean(L, true);
      return 1;
    }
    catch (...) {
      fprintf(stderr, "error on drawing\n");
      lua_pushboolean(L, false);
      return 1;
    }
  }

  bool screen::draw_pixel(int x, int y, const color &c)
  {
    glBegin(GL_POINTS);
      glColor4ub(c.get_r(), c.get_g(), c.get_b(), c.get_a());
      glVertex2i(x, y);
    glEnd();
    return true;
  }

  bool screen::draw_raster(const raster *r, int offset_x, int offset_y, const color *c)
  {
    if (! r) { return false; }

    color orig = color::white();
    if (c) { orig = *c; }
    glBegin(GL_POINTS);
      for (int y = 0; y < r->get_h(); y++)
      {
        for (int x = 0; x < r->get_w(); x++)
        {
          unsigned char a = (unsigned short)orig.get_a() * r->get_pixel(x, y) / 255;
          if (a > 0)
          {
            glColor4ub(orig.get_r(), orig.get_g(), orig.get_b(), a);
            glVertex2i(offset_x + x, offset_y + y);
          }
        }
      }
    glEnd();
    return true;
  }

  bool screen::enable_alpha_blending(bool enable)
  {
    set_current();
    if (enable)
    {
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else
    {
      glDisable(GL_BLEND);
    }
    return true;
  }

  bool screen::flip()
  {
    SDL_GL_SwapBuffers();
    return true;
//    SDL_Surface *screen = SDL_GetVideoSurface();
//    if (SDL_Flip(screen) == 0) { return true; }
//    else { return false; }
  }

  bool screen::map2d_auto()
  {
    if (boost::shared_ptr<window> win = holder.lock())
    {
      int w = win->get_w();
      int h = win->get_h();
      set_current();
      glLoadIdentity();
      glOrtho(0, w, h, 0, -1, 1);
      return true;
    }
    return false;
  }

  bool screen::map2d(int left, int right, int top, int bottom)
  {
    set_current();
    glLoadIdentity();
    glOrtho(left, right, bottom, top, -1, 1);
    return true;
  }

  bool screen::set_current()
  {
    if (boost::shared_ptr<window> win = holder.lock())
    {
      SDL_GL_MakeCurrent((SDL_Window *)win->get_rawobj(), (SDL_GLContext)_obj);
      return true;
    }
    return false;
  }

  bool screen::swap()
  {
    if (boost::shared_ptr<window> win = holder.lock())
    {
      SDL_GL_SwapWindow((SDL_Window *)win->get_rawobj());
      return true;
    }
    return false;
  }

//  bool canvas::call_compiled(drawable *img)
//  {
//    return img->call_compiled(this);
//  }
//
//  bool canvas::call_texture(drawable *img)
//  {
//    return img->call_texture(this);
//  }
//
//  bool canvas::clear()
//  {
//    set_current();
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//    return true;
//  }
//
//  bool canvas::clear_color(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
//  {
//    set_current();
//    glClearColor(r / 255.0, g / 255.0, b / 255.0, a / 255.0);
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//    return true;
//  }
//
//  bool canvas::clear_color1(const color &c)
//  {
//    return clear_color(c.get_r(), c.get_g(), c.get_b(), c.get_a());
//  }
//
//  bool canvas::compile(drawable *img, bool force)
//  {
//    if (! this->is_valid()) { return false; }
//    return img->compile(this, force);
//  }
//
//  canvas* canvas::create(control *parent, int width, int height)
//  {
//    int attribs[] = { WX_GL_RGBA, WX_GL_DOUBLEBUFFER, 0 };
//    canvas *cv = NULL;
//    wxWindow *p = NULL;
//    myCanvas *obj = NULL;
//    try
//    {
//      cv = new canvas();
//      if (parent) { p = (wxWindow *)parent->get_rawobj(); }
//      cv->_obj = obj = new myCanvas(p, attribs, width, height);
//      cv->_id = obj->GetId();
//      cv->connector = obj->GetConnector();
//      cv->func_getter = obj->GetFuncGetter();
//
//      cv->system_managed = true;
//      cv->set_current();
//      glViewport(0, 0, width, height);
//      return cv;
//    }
//    catch (...)
//    {
//      delete cv;
//      return NULL;
//    }
//  }
//
//  int canvas::create_l(lua_State *L)
//  {
//    using namespace luabind;
//    object p;
//    int w = -1, h = -1;
//
//    int n = lua_gettop(L);
//    lua_pushcfunction(L, &util::merge);
//    newtable(L).push(L);
//    for (int i = 1; i <= n; i++)
//    {
//      object(from_stack(L, i)).push(L);
//    }
//    lua_call(L, n + 1, 1);
//    object t(from_stack(L, -1));
//    lua_remove(L, -1);
//
//    if (t["parent"]) { p = t["parent"]; }
//    else if (t["p"]) { p = t["p"]; }
//    else if (t["lua.userdata1"]) { p = t["lua.userdata1"]; }
//
//    if (t["width"]) { w = object_cast<int>(t["width"]); }
//    else if (t["w"]) { w = object_cast<int>(t["w"]); }
//    else if (t["lua.number1"]) { w = object_cast<int>(t["lua.number1"]); }
//
//    if (t["height"]) { h = object_cast<int>(t["height"]); }
//    else if (t["h"]) { h = object_cast<int>(t["h"]); }
//    else if (t["lua.number2"]) { h = object_cast<int>(t["lua.number2"]); }
//
//    object func = globals(L)["lev"]["classes"]["canvas"]["create_c"];
//    object result = func(p, w, h);
//    result.push(L);
//    return 1;
//  }
//
//  bool canvas::draw_image(drawable *img, int x, int y, unsigned char alpha)
//  {
//    if (! this->is_valid()) { return false; }
//    if (! img) { return false; }
//    return img->draw_on(this, x, y, alpha);
//  }
//
//
//  bool canvas::draw_point(point *pt)
//  {
//    set_current();
//
//    glPushMatrix();
//    glBegin(GL_POINTS);
//    {
//      color *c = pt->get_color();
//      vector *v = pt->get_vertex();
//      if (c)
//      {
//        glColor4ub(c->get_r(), c->get_g(), c->get_b(), c->get_a());
//      }
//      glVertex3i(v->get_x(), v->get_y(), v->get_z());
//    }
//    glEnd();
//    glPopMatrix();
//
//    return true;
//  }
//
//
//  int canvas::draw_points(lua_State *L)
//  {
//    using namespace luabind;
//
//    int n = lua_gettop(L);
//    luaL_checktype(L, 1, LUA_TUSERDATA);
//    glBegin(GL_POINTS);
//    for (int i = 2; i <= n; i++)
//    {
//      object o(from_stack(L, i));
//      if (type(o) == LUA_TUSERDATA && o["type_id"] == LEV_TVECTOR)
//      {
//      }
//    }
//    glEnd();
//  }
//
//
//  bool canvas::fill_rect(int x, int y, int w, int h, color *fill)
//  {
//    set_current();
//    glBegin(GL_QUADS);
//      glColor4ub(fill->get_r(), fill->get_g(), fill->get_b(), fill->get_a());
//      glVertex2i(x    , y);
//      glVertex2i(x    , y + h);
//      glVertex2i(x + w, y + h);
//      glVertex2i(x + w, y);
//    glEnd();
//  }
//
//
//  bool canvas::enable_alpha_blending(bool enable)
//  {
//    set_current();
//    if (enable)
//    {
//      glEnable(GL_BLEND);
//      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//    }
//    else
//    {
//      glDisable(GL_BLEND);
//    }
//    return true;
//  }
//
//
//  void canvas::flush()
//  {
//    glFlush();
//  }
//
//  void canvas::line(int x1, int y1, int x2, int y2)
//  {
//    glBegin(GL_LINES);
//      glVertex2i(x1, y1);
//      glVertex2i(x2, y2);
//    glEnd();
//  }
//
//  bool canvas::map2d_auto()
//  {
//    int w, h;
//    ((wxGLCanvas *)_obj)->GetSize(&w, &h);
//    set_current();
//    glLoadIdentity();
//    glOrtho(0, w, h, 0, -1, 1);
//    return true;
//  }
//
//  bool canvas::map2d(int left, int right, int top, int bottom)
//  {
//    set_current();
//    glLoadIdentity();
//    glOrtho(left, right, bottom, top, -1, 256);
//    return true;
//  }
//
//
//  bool canvas::print(const char *text)
//  {
//    try {
//      boost::shared_ptr<image> img(image::string(text, NULL, NULL, NULL));
//      draw_image(img.get(), 200, 80);
//    }
//    catch (...) {
//      return false;
//    }
//  }
//
//  bool canvas::redraw()
//  {
//    wxPaintEvent e;
//    cast_draw(_obj)->AddPendingEvent(e);
//    return true;
//  }
//
//  bool canvas::set_current()
//  {
//    cast_draw(_obj)->SetCurrent();
//    return true;
//  }
//
//  bool canvas::swap()
//  {
//    if (! this->is_valid()) { return false; }
//    cast_draw(_obj)->SwapBuffers();
//    return true;
//  }
//
//  bool canvas::texturize(drawable *img, bool force)
//  {
//    if (! this->is_valid()) { return false; }
//    return img->texturize(this, force);
//  }

}

