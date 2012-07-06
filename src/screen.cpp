/////////////////////////////////////////////////////////////////////////////
// Name:        src/screen.cpp
// Purpose:     source for screen managing class
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
#include "lev/screen.hpp"

// dependencies
#include "lev/debug.hpp"
#include "lev/system.hpp"
#include "lev/util.hpp"
#include "register.hpp"

// libraries
#include <boost/shared_ptr.hpp>
#include <luabind/luabind.hpp>
#include <SDL2/SDL.h>


namespace lev
{

  class impl_screen : public screen
  {
    public:
      typedef boost::shared_ptr<impl_screen> ptr;
    protected:
      impl_screen() :
        wptr(),
        screen(),
        win(NULL),
        context(NULL),
        event_funcs(), win_funcs(),
        on_left_down(), on_left_up(),
        on_middle_down(), on_middle_up(),
        on_right_down(), on_right_up(),
        on_wheel(), on_wheel_down(), on_wheel_up()
        { }
    public:
      virtual ~impl_screen()
      {
        if (context)
        {
          SDL_GL_DeleteContext(context);
          context = NULL;
        }
        if (win)
        {
          SDL_DestroyWindow(win);
          win = NULL;
        }
      }

      virtual bool blit(int dst_x, int dst_y, bitmap::ptr src,
                        int src_x, int src_y, int w, int h, unsigned char alpha)
      {
        if (src == NULL) { return false; }
        if (src->is_texturized())
        {
          return blit(dst_x, dst_y, src->get_texture(), src_x, src_y, w, h, alpha);
        }

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
              color::ptr pixel(src->get_pixel(real_src_x, real_src_y));
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

      virtual bool blit(int dst_x, int dst_y, texture::ptr src,
                        int src_x, int src_y, int w, int h, unsigned char alpha)
      {
        return src->blit_on(to_screen(), dst_x, dst_y, src_x, src_y, w, h, alpha);
      }

      virtual bool clear(unsigned char r, unsigned char g,
                         unsigned char b, unsigned char a)
      {
        set_current();
        glClearColor(r / 255.0, g / 255.0, b / 255.0, a / 255.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //    glClear(GL_COLOR_BUFFER_BIT);
        return true;
      }

      virtual bool close()
      {
        if (context)
        {
          SDL_GL_DeleteContext(context);
          context = NULL;
        }
        if (win)
        {
          SDL_DestroyWindow(win);
          win = NULL;
        }
        return false;
      }

      static impl_screen::ptr create(const char *title, int x, int y, int w, int h,
                                     const char *style)
      {
        impl_screen::ptr s;
        system::ptr sys = system::get();
        if (! sys) { return s; }

        unsigned long flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
        if (style)
        {
          if (strstr(style, "borderless"))    { flags |= SDL_WINDOW_BORDERLESS; }
          if (strstr(style, "full"))          { flags |= SDL_WINDOW_FULLSCREEN; }
          if (strstr(style, "hidden"))        { flags |= SDL_WINDOW_HIDDEN; }
    //      if (strstr(flags_str, "input"))         { flags |= SDL_WINDOW_INPUT_GRABBED; }
          if (strstr(style, "input_grabbed")) { flags |= SDL_WINDOW_INPUT_GRABBED; }
          if (strstr(style, "max"))           { flags |= SDL_WINDOW_MAXIMIZED; }
          if (strstr(style, "min"))           { flags |= SDL_WINDOW_MINIMIZED; }
          if (strstr(style, "resizable"))     { flags |= SDL_WINDOW_RESIZABLE; }
          if (strstr(style, "shown"))         { flags |= SDL_WINDOW_SHOWN; }
        }

        try {
          s.reset(new impl_screen);
          if (! s) { throw -1; }
          s->wptr = s;
          s->win = SDL_CreateWindow(title, x, y, w, h, flags);
          if (! s->win) { throw -2; }
          s->context = SDL_GL_CreateContext(s->win);
          if (! s->context) { throw -3; }

          SDL_GL_SetSwapInterval(0);
    //      glMatrixMode(GL_PROJECTION);
    //      glLoadIdentity();
    //      glOrtho(0.0f, 640, 480, 0.0f, 0.0f, 1000.0f);
    //      glMatrixMode(GL_MODELVIEW);

          glOrtho(0, s->get_w(), s->get_h(), 0, -1, 1);
          glEnable(GL_BLEND);
          glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
          if (! sys->attach(s->to_screen())) { throw -4; }
        }
        catch (...) {
          s.reset();
          lev::debug_print("error on screen instance creation");
        }
        return s;
      }

      static int create_l(lua_State *L)
      {
        using namespace luabind;
        const char *title = "Lev Application";
        int x = SDL_WINDOWPOS_UNDEFINED, y = SDL_WINDOWPOS_UNDEFINED;
        int w = 640, h = 480;
        const char *flags_str = NULL;

        try {
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
          else if (t["f"]) { flags_str = object_cast<const char *>(t["f"]); }
          else if (t["lua.string2"]) { flags_str = object_cast<const char *>(t["lua.string2"]); }
          object o = globals(L)["lev"]["classes"]["screen"]["create_c"](title, x, y, w, h, flags_str);
          o.push(L);
          return 1;
        }
        catch (...) {
          lev::debug_print(lua_tostring(L, -1));
          lev::debug_print("error on wrapped screen creation");
          lua_pushnil(L);
          return 1;
        }
      }

      virtual bool draw(drawable::ptr src, int x, int y, unsigned char alpha)
      {
printf("SCREEN DRAW!\n");
        if (! src) { return false; }
        set_current();
printf("SCREEN DRAW ON!\n");
printf("TO SCREEN: %p\n", to_screen().get());
        return src->draw_on(to_screen(), x, y, alpha);
      }

      virtual bool draw_bitmap(bitmap::ptr src, int offset_x, int offset_y, unsigned char alpha)
      {
        if (! src) { return false; }
        if (src->is_texturized())
        {
          texture::ptr tex = src->get_texture();
          if (! tex) { return false; }
          return tex->blit_on(to_screen(), offset_x, offset_y, 0, 0, -1, -1, alpha);
//          return tex->draw_on_screen(to_screen(), offset_x, offset_y, alpha);
        }

        int w = get_w();
        int h = get_h();
        const unsigned char *pixel = src->get_buffer();
        set_current();
        glBegin(GL_POINTS);
          for (int y = 0; y < h; y++)
          {
            for (int x = 0; x < w; x++)
            {
              unsigned char a = (unsigned short)pixel[3] * alpha / 255;
              if (a > 0)
              {
                glColor4ub(pixel[0], pixel[1], pixel[2], a);
                glVertex2i(offset_x + x, offset_y + y);
              }
              pixel += 4;
            }
          }
        glEnd();
        return true;
      }

      virtual bool draw_pixel(int x, int y, const color &c)
      {
        glBegin(GL_POINTS);
          glColor4ub(c.get_r(), c.get_g(), c.get_b(), c.get_a());
          glVertex2i(x, y);
        glEnd();
        return true;
      }

//      virtual bool draw_raster(const raster *r, int offset_x, int offset_y, boost::shared_ptr<color> c)
//      {
//        if (! r) { return false; }
//
//        if (! c) { c = color::white(); }
//        if (! c) { return false; }
//        glBegin(GL_POINTS);
//          for (int y = 0; y < r->get_h(); y++)
//          {
//            for (int x = 0; x < r->get_w(); x++)
//            {
//              unsigned char a = (unsigned short)c->get_a() * r->get_pixel(x, y) / 255;
//              if (a > 0)
//              {
//                glColor4ub(c->get_r(), c->get_g(), c->get_b(), a);
//                glVertex2i(offset_x + x, offset_y + y);
//              }
//            }
//          }
//        glEnd();
//        return true;
//      }

      virtual bool enable_alpha_blending(bool enable)
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

      virtual int get_h() const
      {
        if (! win) { return 0; }
        int h;
        SDL_GetWindowSize(win, NULL, &h);
        return h;
      }

      virtual long get_id() const
      {
        if (! win) { return -1; }
        return SDL_GetWindowID(win);
      }

      virtual luabind::object get_on_close()
      {
        if (! win) { return luabind::object(); }
        return win_funcs[SDL_WINDOWEVENT_CLOSE];
      }

      virtual luabind::object get_on_key_down()
      {
        if (! win) { return luabind::object(); }
        return event_funcs[SDL_KEYDOWN];
      }

      virtual luabind::object get_on_key_up()
      {
        if (! win) { return luabind::object(); }
        return event_funcs[SDL_KEYUP];
      }

      virtual luabind::object get_on_left_down()
      {
        if (! win) { return luabind::object(); }
        return on_left_down;
      }

      virtual luabind::object get_on_left_up()
      {
        if (! win) { return luabind::object(); }
        return on_left_up;
      }

      virtual luabind::object get_on_middle_down()
      {
        if (! win) { return luabind::object(); }
        return on_middle_down;
      }

      virtual luabind::object get_on_middle_up()
      {
        if (! win) { return luabind::object(); }
        return on_middle_up;
      }

      virtual luabind::object get_on_motion()
      {
        if (! win) { return luabind::object(); }
        return event_funcs[SDL_MOUSEMOTION];
      }

      virtual luabind::object get_on_right_down()
      {
        if (! win) { return luabind::object(); }
        return on_right_down;
      }

      virtual luabind::object get_on_right_up()
      {
        if (! win) { return luabind::object(); }
        return on_right_up;
      }

      virtual luabind::object get_on_wheel()
      {
        if (! win) { return luabind::object(); }
        return event_funcs[SDL_MOUSEWHEEL];
      }

      virtual luabind::object get_on_wheel_down()
      {
        if (! win) { return luabind::object(); }
        return on_wheel_down;
      }

      virtual luabind::object get_on_wheel_up()
      {
        if (! win) { return luabind::object(); }
        return on_wheel_up;
      }

      virtual bitmap::ptr get_screenshot()
      {
        bitmap::ptr img;
        if (! win) { return img; }
        set_current();
        try {
          if (get_id() > 0)
          {
            const int w = get_w();
            const int h = get_h();
            img = bitmap::create(w, h);
            if (! img) { throw -1; }
            glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, img->get_buffer());
            Uint32 *buf = (Uint32 *)img->get_buffer();
            for (int y = 0; y < h / 2; y++)
            {
              for (int x = 0; x < w; x++)
              {
                Uint32 tmp = buf[y * w + x];
                buf[y * w + x] = buf[(h - 1 - y) * w + x];
                buf[(h - 1 - y) * w + x] = tmp;
              }
            }
          }
          else { throw -1; }
        }
        catch (...) {
          img.reset();
          lev::debug_print("error on screen shot bitmap creation");
        }
        return img;
      }

      virtual int get_w() const
      {
        if (! win) { return 0; }
        int w;
        SDL_GetWindowSize(win, &w, NULL);
        return w;
      }

      virtual bool hide()
      {
        if (! win) { return false; }
        SDL_HideWindow(win);
        return true;
      }

      virtual bool is_fullscreen() const
      {
        if (! win) { return false; }
        if (SDL_GetWindowFlags(win) & SDL_WINDOW_FULLSCREEN) { return true; }
        return false;
      }

      virtual bool is_shown() const
      {
        if (! win) { return false; }
        if (SDL_GetWindowFlags(win) & SDL_WINDOW_SHOWN) { return true; }
        return false;
      }

      virtual bool map2d_auto()
      {
        if (win)
        {
          int w = get_w();
          int h = get_h();
          set_current();
          glLoadIdentity();
          glOrtho(0, w, h, 0, -1, 1);
          return true;
        }
        return false;
      }

      virtual bool map2d(int left, int right, int top, int bottom)
      {
        if (win)
        {
          set_current();
          glLoadIdentity();
          glOrtho(left, right, bottom, top, -1, 1);
          return true;
        }
        return false;
      }

      virtual bool set_current()
      {
        if (win)
        {
          if (get_id() < 0) { return false; }
          SDL_GL_MakeCurrent(win, context);
          return true;
        }
        return false;
      }

      virtual bool set_fullscreen(bool enable)
      {
        if (! win) { return false; }
        if (SDL_SetWindowFullscreen(win, (SDL_bool)enable) == 0)
        {
          return true;
        }
        return false;
      }

      virtual bool set_h(int h)
      {
        if (! win) { return false; }
        int w = get_w();
        SDL_SetWindowSize(win, w, h);
        return true;
      }

      virtual bool set_on_close(luabind::object func)
      {
        if (! win) { return false; }
        win_funcs[SDL_WINDOWEVENT_CLOSE] = func;
        return true;
      }

      virtual bool set_on_key_down(luabind::object func)
      {
        if (! win) { return false; }
        event_funcs[SDL_KEYDOWN] = func;
        return true;
      }

      virtual bool set_on_key_up(luabind::object func)
      {
        if (! win) { return false; }
        event_funcs[SDL_KEYUP] = func;
        return true;
      }

      virtual bool set_on_left_down(luabind::object func)
      {
        if (! win) { return false; }
        on_left_down = func;
        return true;
      }

      virtual bool set_on_left_up(luabind::object func)
      {
        if (! win) { return false; }
        on_left_up = func;
        return true;
      }

      virtual bool set_on_middle_down(luabind::object func)
      {
        if (! win) { return false; }
        on_middle_down = func;
        return true;
      }

      virtual bool set_on_middle_up(luabind::object func)
      {
        if (! win) { return false; }
        on_middle_up = func;
        return true;
      }

      virtual bool set_on_motion(luabind::object func)
      {
        if (! win) { return false; }
        event_funcs[SDL_MOUSEMOTION] = func;
        return true;
      }

      virtual bool set_on_right_down(luabind::object func)
      {
        if (! win) { return false; }
        on_right_down = func;
        return true;
      }

      virtual bool set_on_right_up(luabind::object func)
      {
        if (! win) { return false; }
        on_right_up = func;
        return true;
      }

      virtual bool set_on_wheel(luabind::object func)
      {
        if (! win) { return false; }
        event_funcs[SDL_MOUSEWHEEL] = func;
        return true;
      }

      virtual bool set_on_wheel_down(luabind::object func)
      {
        if (! win) { return false; }
        on_wheel_down = func;
        return true;
      }

      virtual bool set_on_wheel_up(luabind::object func)
      {
        if (! win) { return false; }
        on_wheel_up = func;
        return true;
      }

      virtual bool set_w(int w)
      {
        if (! win) { return false; }
        int h = get_h();
        SDL_SetWindowSize(win, w, h);
        return true;
      }

      virtual bool show(bool showing)
      {
        if (! win) { return false; }
        if (showing)
        {
          SDL_ShowWindow(win);
          return true;
        }
        return hide();
      }

      virtual bool swap()
      {
        if (win)
        {
          if (get_id() < 0) { return false; }
          SDL_GL_SwapWindow(win);
          return true;
        }
        return false;
      }

      virtual canvas::ptr to_canvas()
      {
        return canvas::ptr(wptr);
      }

      virtual screen::ptr to_screen()
      {
        return screen::ptr(wptr);
      }

      boost::weak_ptr<impl_screen> wptr;
      SDL_GLContext context;
      SDL_Window *win;
      std::map<Uint8, luabind::object> win_funcs;
      std::map<Uint32, luabind::object> event_funcs;
      luabind::object on_left_down, on_left_up;
      luabind::object on_middle_down, on_middle_up;
      luabind::object on_right_down, on_right_up;
      luabind::object on_wheel, on_wheel_down, on_wheel_up;
  };

  static SDL_GLContext cast_ctx(void *obj) { return (SDL_GLContext)obj; }

  screen::ptr screen::create(const char *title, int x, int y, int w, int h, const char *style)
  {
    return impl_screen::create(title, x, y, w, h, style);
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

}

int luaopen_lev_screen(lua_State *L)
{
  using namespace luabind;
  using namespace lev;

  open(L);
  // beginning of loading
  globals(L)["package"]["loaded"]["lev.screen"] = true;
  // pre-requirement
  globals(L)["require"]("lev.draw");
  globals(L)["require"]("lev.system");

  module(L, "lev")
  [
    namespace_("classes")
    [
      class_<screen, canvas, boost::shared_ptr<canvas> >("screen")
        .def("close", &screen::close)
        .def("enable_alpha_blending", &screen::enable_alpha_blending0)
        .def("enable_alpha_blending", &screen::enable_alpha_blending)
        .def("flip", &screen::swap)
        .def("get_screen_shot", &screen::get_screenshot)
        .def("get_screenshot", &screen::get_screenshot)
        .property("h", &screen::get_h, &screen::set_h)
        .property("height", &screen::get_h, &screen::set_h)
        .def("hide", &screen::hide)
        .property("id", &screen::get_id)
        .property("is_full_screen", &screen::is_fullscreen, &screen::set_fullscreen)
        .property("is_fullscreen", &screen::is_fullscreen, &screen::set_fullscreen)
        .property("is_shown", &screen::is_shown, &screen::show)
        .def("map2d", &screen::map2d)
        .def("map2d", &screen::map2d_auto)
        .property("on_close", &screen::get_on_close, &screen::set_on_close)
        .property("on_key_down", &screen::get_on_key_down, &screen::set_on_key_down)
        .property("on_key_up", &screen::get_on_key_down, &screen::set_on_key_up)
        .property("on_left_down", &screen::get_on_left_down, &screen::set_on_left_down)
        .property("on_left_up", &screen::get_on_left_up, &screen::set_on_left_up)
        .property("on_middle_down", &screen::get_on_middle_down, &screen::set_on_middle_down)
        .property("on_middle_up", &screen::get_on_middle_up, &screen::set_on_middle_up)
        .property("on_motion", &screen::get_on_motion, &screen::set_on_motion)
        .property("on_right_down", &screen::get_on_right_down, &screen::set_on_right_down)
        .property("on_right_up", &screen::get_on_right_up, &screen::set_on_right_up)
        .property("on_wheel", &screen::get_on_wheel, &screen::set_on_wheel)
        .property("on_wheel_down", &screen::get_on_wheel_down, &screen::set_on_wheel_down)
        .property("on_wheel_up", &screen::get_on_wheel_up, &screen::set_on_wheel_up)
        .property("screen_shot", &screen::get_screenshot)
        .property("screenshot", &screen::get_screenshot)
        .def("set_current", &screen::set_current)
        .def("set_full_screen", &screen::set_fullscreen)
        .def("set_fullscreen", &screen::set_fullscreen)
        .def("show", &screen::show)
        .def("show", &screen::show0)
        .def("swap", &screen::swap)
        .property("w", &screen::get_w, &screen::set_w)
        .property("width", &screen::get_w, &screen::set_w)
        .scope
        [
          def("create_c", &screen::create)
        ]
    ]
  ];
  object lev = globals(L)["lev"];
  object classes = lev["classes"];
  register_to(classes["screen"], "draw", &screen::draw_l);
  register_to(classes["screen"], "create", &impl_screen::create_l);

  lev["screen"] = classes["screen"]["create"];

  globals(L)["package"]["loaded"]["lev.screen"] = true;
  return 0;
}

