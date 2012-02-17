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
#include "lev/font.hpp"
#include "lev/image.hpp"
#include "lev/window.hpp"

// dependencies
#include "lev/debug.hpp"
#include "lev/system.hpp"
#include "lev/util.hpp"
#include "register.hpp"

// libraries
#include <boost/shared_ptr.hpp>
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
      class_<window, base, boost::shared_ptr<base> >("window")
        .def("close", &window::close)
        .def("create_context", &screen::create)
        .def("create_screen", &screen::create)
        .property("h", &window::get_h, &window::set_h)
        .property("height", &window::get_h, &window::set_h)
        .property("id", &window::get_id)
        .property("is_full_screen", &window::is_fullscreen, &window::set_fullscreen)
        .property("is_fullscreen", &window::is_fullscreen, &window::set_fullscreen)
        .def("hide", &window::hide)
        .property("on_close", &window::get_on_close, &window::set_on_close)
        .property("on_key_down", &window::get_on_key_down, &window::set_on_key_down)
        .property("on_key_up", &window::get_on_key_down, &window::set_on_key_up)
        .property("on_left_down", &window::get_on_left_down, &window::set_on_left_down)
        .property("on_left_up", &window::get_on_left_up, &window::set_on_left_up)
        .property("on_middle_down", &window::get_on_middle_down, &window::set_on_middle_down)
        .property("on_middle_up", &window::get_on_middle_up, &window::set_on_middle_up)
        .property("on_motion", &window::get_on_motion, &window::set_on_motion)
        .property("on_right_down", &window::get_on_right_down, &window::set_on_right_down)
        .property("on_right_up", &window::get_on_right_up, &window::set_on_right_up)
        .property("on_wheel", &window::get_on_wheel, &window::set_on_wheel)
        .property("on_wheel_down", &window::get_on_wheel_down, &window::set_on_wheel_down)
        .property("on_wheel_up", &window::get_on_wheel_up, &window::set_on_wheel_up)
        .def("screen", &screen::create)
        .def("set_full_screen", &window::set_fullscreen)
        .def("set_fullscreen", &window::set_fullscreen)
        .def("show", &window::show)
        .def("show", &window::show0)
        .property("w", &window::get_w, &window::set_w)
        .property("width", &window::get_w, &window::set_w),
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
        .def("get_screen_shot", &screen::get_screenshot)
        .def("get_screenshot", &screen::get_screenshot)
        .def("map2d", &screen::map2d)
        .def("map2d", &screen::map2d_auto)
        .property("screen_shot", &screen::get_screenshot)
        .property("screenshot", &screen::get_screenshot)
        .def("set_current", &screen::set_current)
        .def("swap", &screen::swap)
    ]
  ];
  object lev = globals(L)["lev"];
  object classes = lev["classes"];
  register_to(classes["screen"], "draw", &screen::draw_l);

  globals(L)["package"]["loaded"]["lev.window"] = true;
  return 0;
}


namespace lev
{

  class myWindow
  {
    protected:
      myWindow() :
        win(NULL), event_funcs(), win_funcs(),
        on_left_down(), on_left_up(),
        on_middle_down(), on_middle_up(),
        on_right_down(), on_right_up(),
        on_wheel(), on_wheel_down(), on_wheel_up()
      { }

    public:
      virtual ~myWindow()
      {
        if (win)
        {
          SDL_DestroyWindow(win);
          win = NULL;
        }
      }

      static myWindow* Create(const char *title, int x, int y, int w, int h, unsigned long flags)
      {
        myWindow *win = NULL;
        try {
          win = new myWindow;
          if (! win) { throw -1; }
          win->win = SDL_CreateWindow(title, x, y, w, h, flags);
          if (! win->win) { throw -2; }
          return win;
        }
        catch (...) {
          delete win;
          return NULL;
        }
      }

      std::map<Uint8, luabind::object> win_funcs;
      std::map<Uint32, luabind::object> event_funcs;
      luabind::object on_left_down, on_left_up;
      luabind::object on_middle_down, on_middle_up;
      luabind::object on_right_down, on_right_up;
      luabind::object on_wheel, on_wheel_down, on_wheel_up;
      SDL_Window *win;
  };
  static myWindow* cast_win(void *obj) { return (myWindow*)obj; }

  window::window() : _obj(NULL) { }

  window::~window()
  {
    if (_obj)
    {
      delete cast_win(_obj);
      _obj = NULL;
    }
  }

  bool window::close()
  {
    if (_obj)
    {
      delete cast_win(_obj);
      _obj = NULL;
      return true;
    }
    return false;
  }

  boost::shared_ptr<window> window::create(const char *title, int x, int y, int w, int h, unsigned long flags)
  {
    boost::shared_ptr<window> win;
    if (! system::get()) { return win; }
    try {
      win.reset(new window);
      if (! win) { throw -1; }
      win->_obj = myWindow::Create(title, x, y, w, h, flags);
      if (! win->_obj) { throw -2; }
    }
    catch (...) {
      win.reset();
      lev::debug_print("error on window instance creation");
    }
    return win;
  }

//  screen* window::create_context()
//  boost::shared_ptr<screen> window::create_context()
//  {
//    return screen::create(this);
//  }

  int window::get_h()
  {
    if (! _obj) { return 0; }
    int h;
    SDL_GetWindowSize(cast_win(_obj)->win, NULL, &h);
    return h;
  }

  long window::get_id()
  {
    if (! _obj) { return -1; }
    return SDL_GetWindowID(cast_win(_obj)->win);
  }

  luabind::object window::get_on_close()
  {
    if (! _obj) { return luabind::object(); }
    return cast_win(_obj)->win_funcs[SDL_WINDOWEVENT_CLOSE];
  }

  luabind::object window::get_on_key_down()
  {
    if (! _obj) { return luabind::object(); }
    return cast_win(_obj)->event_funcs[SDL_KEYDOWN];
  }

  luabind::object window::get_on_key_up()
  {
    if (! _obj) { return luabind::object(); }
    return cast_win(_obj)->event_funcs[SDL_KEYUP];
  }

  luabind::object window::get_on_left_down()
  {
    if (! _obj) { return luabind::object(); }
    return cast_win(_obj)->on_left_down;
  }

  luabind::object window::get_on_left_up()
  {
    if (! _obj) { return luabind::object(); }
    return cast_win(_obj)->on_left_up;
  }

  luabind::object window::get_on_middle_down()
  {
    if (! _obj) { return luabind::object(); }
    return cast_win(_obj)->on_middle_down;
  }

  luabind::object window::get_on_middle_up()
  {
    if (! _obj) { return luabind::object(); }
    return cast_win(_obj)->on_middle_up;
  }

  luabind::object window::get_on_motion()
  {
    if (! _obj) { return luabind::object(); }
    return cast_win(_obj)->event_funcs[SDL_MOUSEMOTION];
  }

  luabind::object window::get_on_right_down()
  {
    if (! _obj) { return luabind::object(); }
    return cast_win(_obj)->on_right_down;
  }

  luabind::object window::get_on_right_up()
  {
    if (! _obj) { return luabind::object(); }
    return cast_win(_obj)->on_right_up;
  }

  luabind::object window::get_on_wheel()
  {
    if (! _obj) { return luabind::object(); }
    return cast_win(_obj)->event_funcs[SDL_MOUSEWHEEL];
  }

  luabind::object window::get_on_wheel_down()
  {
    if (! _obj) { return luabind::object(); }
    return cast_win(_obj)->on_wheel_down;
  }

  luabind::object window::get_on_wheel_up()
  {
    if (! _obj) { return luabind::object(); }
    return cast_win(_obj)->on_wheel_up;
  }

  int window::get_w()
  {
    if (! _obj) { return 0; }
    int w;
    SDL_GetWindowSize(cast_win(_obj)->win, &w, NULL);
    return w;
  }

  bool window::hide()
  {
    if (! _obj) { return 0; }
    SDL_HideWindow(cast_win(_obj)->win);
  }

  bool window::is_fullscreen()
  {
    if (! _obj) { return false; }
    if (SDL_GetWindowFlags(cast_win(_obj)->win) & SDL_WINDOW_FULLSCREEN) { return true; }
    return false;
  }

  bool window::set_fullscreen(bool enable)
  {
    if (! _obj) { return false; }
    if (SDL_SetWindowFullscreen(cast_win(_obj)->win, (SDL_bool)enable) == 0)
    {
      return true;
    }
    return false;
  }

  bool window::set_h(int h)
  {
    if (! _obj) { return false; }
    int w = get_w();
    SDL_SetWindowSize(cast_win(_obj)->win, w, h);
  }

  bool window::set_on_close(luabind::object func)
  {
    if (! _obj) { return false; }
    cast_win(_obj)->win_funcs[SDL_WINDOWEVENT_CLOSE] = func;
    return true;
  }

  bool window::set_on_key_down(luabind::object func)
  {
    if (! _obj) { return false; }
    cast_win(_obj)->event_funcs[SDL_KEYDOWN] = func;
    return true;
  }

  bool window::set_on_key_up(luabind::object func)
  {
    if (! _obj) { return false; }
    cast_win(_obj)->event_funcs[SDL_KEYUP] = func;
    return true;
  }

  bool window::set_on_left_down(luabind::object func)
  {
    if (! _obj) { return false; }
    cast_win(_obj)->on_left_down = func;
    return true;
  }

  bool window::set_on_left_up(luabind::object func)
  {
    if (! _obj) { return false; }
    cast_win(_obj)->on_left_up = func;
    return true;
  }

  bool window::set_on_middle_down(luabind::object func)
  {
    if (! _obj) { return false; }
    cast_win(_obj)->on_middle_down = func;
    return true;
  }

  bool window::set_on_middle_up(luabind::object func)
  {
    if (! _obj) { return false; }
    cast_win(_obj)->on_middle_up = func;
    return true;
  }

  bool window::set_on_motion(luabind::object func)
  {
    if (! _obj) { return false; }
    cast_win(_obj)->event_funcs[SDL_MOUSEMOTION] = func;
    return true;
  }

  bool window::set_on_right_down(luabind::object func)
  {
    if (! _obj) { return false; }
    cast_win(_obj)->on_right_down = func;
    return true;
  }

  bool window::set_on_right_up(luabind::object func)
  {
    if (! _obj) { return false; }
    cast_win(_obj)->on_right_up = func;
    return true;
  }

  bool window::set_on_wheel(luabind::object func)
  {
    if (! _obj) { return false; }
    cast_win(_obj)->event_funcs[SDL_MOUSEWHEEL] = func;
    return true;
  }

  bool window::set_on_wheel_down(luabind::object func)
  {
    if (! _obj) { return false; }
    cast_win(_obj)->on_wheel_down = func;
    return true;
  }

  bool window::set_on_wheel_up(luabind::object func)
  {
    if (! _obj) { return false; }
    cast_win(_obj)->on_wheel_up = func;
    return true;
  }

  bool window::set_w(int w)
  {
    if (! _obj) { return false; }
    int h = get_h();
    SDL_SetWindowSize(cast_win(_obj)->win, w, h);
  }

  bool window::show(bool showing)
  {
    if (! _obj) { return false; }
    if (showing)
    {
      SDL_ShowWindow(cast_win(_obj)->win);
      return true;
    }
    return hide();
  }

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
    set_current();
    glClearColor(r / 255.0, g / 255.0, b / 255.0, a / 255.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//    glClear(GL_COLOR_BUFFER_BIT);
    return true;
  }

  bool screen::clear_color1(const color &c)
  {
    return clear_color(c.get_r(), c.get_g(), c.get_b(), c.get_a());
  }

  boost::shared_ptr<screen> screen::create(boost::shared_ptr<window> holder)
  {
    boost::shared_ptr<screen> screen;

    if (! holder) { return screen; }
    try {
      screen.reset(new lev::screen);
      if (! screen) { throw -1; }
      screen->_obj = SDL_GL_CreateContext(cast_win(holder->get_rawobj())->win);
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
      lev::debug_print("error on screen instance creation");
    }
    return screen;
  }

  bool screen::draw(drawable *src, int x, int y, unsigned char alpha)
  {
    if (! src) { return false; }
    set_current();
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
    }
    catch (...) {
      lev::debug_print("error on drawing");
    }
    return 1;
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

  boost::shared_ptr<image> screen::get_screenshot()
  {
    set_current();
    return image::take_screenshot(holder.lock());
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
      if (win->get_id() < 0) { return false; }
      SDL_GL_MakeCurrent(cast_win(win->get_rawobj())->win, (SDL_GLContext)_obj);
      return true;
    }
    return false;
  }

  bool screen::swap()
  {
    if (boost::shared_ptr<window> win = holder.lock())
    {
      if (win->get_id() < 0) { return false; }
      SDL_GL_SwapWindow(cast_win(win->get_rawobj())->win);
      return true;
    }
    return false;
  }

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

//  void canvas::flush()
//  {
//    glFlush();
//  }

//  void canvas::line(int x1, int y1, int x2, int y2)
//  {
//    glBegin(GL_LINES);
//      glVertex2i(x1, y1);
//      glVertex2i(x2, y2);
//    glEnd();
//  }
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

  debug_window::debug_window() : window(), ptr_screen() { }

  debug_window::~debug_window()
  {
    if (_obj)
    {
      delete cast_win(_obj);
      _obj = NULL;
    }
  }

  bool debug_window::clear()
  {
    ptr_screen->clear();
    ptr_layout->clear();
    ptr_screen->swap();
    log = "";
    buffer = "";
    return true;
  }

  boost::shared_ptr<debug_window> debug_window::get()
  {
    if (singleton && singleton->_obj != NULL) { return singleton; }
    return boost::shared_ptr<debug_window>();
  }

  boost::shared_ptr<debug_window> debug_window::init()
  {
    const int w = 640;
    const int h = 480;
    // singleton already exists
    if (singleton) { return singleton; }
    // system is not yet initialized
    if (! system::get()) { return singleton; }
    try {
      singleton.reset(new debug_window);
      if (! singleton) { throw -1; }
//      long flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
      long flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
      singleton->_obj = myWindow::Create("Lev Debug Window", 0, 0, w, h, flags);
      if (! singleton->_obj) { throw -2; }
      singleton->ptr_screen = screen::create(singleton);
      if (! singleton->ptr_screen) { throw -3; }
      singleton->ptr_layout = layout::create(w);
      if (! singleton->ptr_layout) { throw -4; }
    }
    catch (...) {
      singleton.reset();
      lev::debug_print("error on debug window singleton initialization");
    }
    return singleton;
  }

  bool debug_window::print(const std::string &message_utf8, int font_size)
  {
    if (! _obj) { return false; }
    if (font_size <= 0) { return false; }
    try {
      ptr_screen->set_current();
      boost::shared_ptr<unicode_string> str = unicode_string::from_utf8(message_utf8);
      if (! str) { throw -1; }
      ptr_layout->get_font()->set_pixel_size(font_size);
      for (int i = 0; i < str->length(); i++)
      {
        std::string unit = str->index_str(i)->to_utf8();
        if (unit == "\r") { continue; }
        else if (unit == "\n") { ptr_layout->reserve_new_line(); }
        else { ptr_layout->reserve_word(unit); }
      }
      ptr_layout->complete();
      ptr_screen->clear();
      if (ptr_layout->get_h() > get_h())
      {
        ptr_screen->draw(ptr_layout.get(), 0, - (ptr_layout->get_h() - get_h()), 255);
      }
      else
      {
        ptr_screen->draw(ptr_layout.get(), 0, 0, 255);
      }
      ptr_screen->swap();
      log += message_utf8;
      buffer += message_utf8;

      if (ptr_layout->get_h() > get_h() * 2)
      {
        // messages are to long, purging the oldest part
        str = unicode_string::from_utf8(buffer);
        str = str->sub_string1(str->length() / 4);
        buffer = str->to_utf8();
        ptr_layout->clear();

        for (int i = 0; i < str->length(); i++)
        {
          std::string unit = str->index_str(i)->to_utf8();
          if (unit == "\r") { continue; }
          else if (unit == "\n") { ptr_layout->reserve_new_line(); }
          else { ptr_layout->reserve_word(unit); }
        }
      }
    }
    catch (...) {
      fprintf(stderr, "error on debug printing\n");
      return false;
    }
    return true;
  }

  bool debug_window::stop()
  {
    if (singleton and singleton->_obj)
    {
      singleton->close();
    }
    singleton.reset();
    return true;
  }

}

