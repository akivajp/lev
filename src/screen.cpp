/////////////////////////////////////////////////////////////////////////////
// Name:        src/screen.cpp
// Purpose:     source for screen managing class
// Author:      Akiva Miura <akiva.miura@gmail.com>
// Modified by:
// Created:     06/19/2012
// Copyright:   (C) 2010-2012 Akiva Miura
// Licence:     MIT License
/////////////////////////////////////////////////////////////////////////////

// pre-compiled header
#include "prec.h"

// declarations
#include "lev/screen.hpp"

// dependencies
#include "lev/debug.hpp"
#include "lev/entry.hpp"
#include "lev/font.hpp"
#include "lev/image.hpp"
#include "lev/system.hpp"
#include "lev/util.hpp"

// libraries
#include <allegro5/allegro.h>
#include <boost/shared_ptr.hpp>
#include <luabind/luabind.hpp>

int luaopen_lev_screen(lua_State *L)
{
  using namespace luabind;
  using namespace lev;

  open(L);
  // beginning of loading
  globals(L)["package"]["loaded"]["lev.screen"] = true;
  // pre-requirement
  globals(L)["require"]("lev.system");

  module(L, "lev")
  [
    namespace_("classes")
    [
      class_<screen, bitmap, boost::shared_ptr<base> >("screen")
        .def("close", &screen::close)
        .def("create_context", &screen::create)
        .def("create_screen", &screen::create)
        .def("flip", &screen::flip)
        .property("id", &screen::get_id)
        .property("is_full_screen", &screen::is_fullscreen, &screen::set_fullscreen)
        .property("is_fullscreen", &screen::is_fullscreen, &screen::set_fullscreen)
        .property("is_valid", &screen::is_valid)
        .def("hide", &screen::hide)
        .property("on_button_down", &screen::get_on_button_down, &screen::set_on_button_down)
        .property("on_button_up", &screen::get_on_button_down, &screen::set_on_button_up)
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
        .property("screen_shot", &screen::take_screen_shot)
        .property("screenshot", &screen::take_screen_shot)
        .def("screen", &screen::create)
        .def("set_full_screen", &screen::set_fullscreen)
        .def("set_full_screen", &screen::set_fullscreen0)
        .def("set_fullscreen", &screen::set_fullscreen)
        .def("set_fullscreen", &screen::set_fullscreen0)
        .def("show", &screen::show)
        .def("show", &screen::show0)
        .def("swap", &screen::flip)
        .def("take_screenshot", &screen::take_screen_shot)
        .def("take_screen_shot", &screen::take_screen_shot)
//      class_<screen, base, boost::shared_ptr<base> >("screen")
//        .def("blit", &screen::blit)
//        .def("blit", &screen::blit1)
//        .def("blit", &screen::blit2)
//        .def("blit", &screen::blit3)
//        .def("blit", &screen::blit4)
//        .def("draw_pixel", &screen::draw_pixel)
//        .def("draw_raster", &screen::draw_raster)
//        .def("enable_alpha_blending", &screen::enable_alpha_blending0)
//        .def("enable_alpha_blending", &screen::enable_alpha_blending)
//        .def("map2d", &screen::map2d)
//        .def("map2d", &screen::map2d_auto)
//        .def("swap", &screen::swap)
    ]
  ];
  object lev = globals(L)["lev"];
  object classes = lev["classes"];
//  register_to(classes["screen"], "draw", &screen::draw_l);

  globals(L)["package"]["loaded"]["lev.screen"] = true;
  return 0;
}


namespace lev
{

  class myScreen
  {
    protected:
      myScreen() :
        disp(NULL), event_funcs(),
        on_left_down(), on_left_up(),
        on_middle_down(), on_middle_up(),
        on_right_down(), on_right_up(),
        on_wheel(), on_wheel_down(), on_wheel_up()
      { }

    public:
      virtual ~myScreen()
      {
        if (disp && system::get_interpreter())
        {
          al_destroy_display(disp);
          disp = NULL;
        }
      }

      static myScreen* Create(const char *title, int w, int h, int x, int y)
      {
        myScreen *scr = NULL;
        ALLEGRO_DISPLAY *disp = NULL;
        try {
          scr = new myScreen;
          if (! scr) { throw -1; }
          scr->disp = disp = al_create_display(w, h);
          if (! disp) { throw -2; }
          if (title) { al_set_window_title(disp, title); }
          if (x >= 0 || y >= 0)
          {
            int current_x, current_y;
            al_get_window_position(disp, &current_x, &current_y);
            if (x < 0) { x = current_x; }
            if (y < 0) { y = current_y; }
            al_set_window_position(disp, x, y);
          }
          return scr;
        }
        catch (...) {
          delete scr;
          return NULL;
        }
      }

//      std::map<Uint8, luabind::object> win_funcs;
      std::map<unsigned int, luabind::object> event_funcs;
      luabind::object on_left_down, on_left_up;
      luabind::object on_middle_down, on_middle_up;
      luabind::object on_right_down, on_right_up;
      luabind::object on_wheel, on_wheel_down, on_wheel_up;
      ALLEGRO_DISPLAY *disp;
  };
//  static myWindow* cast_win(void *obj) { return (myWindow*)obj; }
  static myScreen* cast_screen(void *obj) { return (myScreen*)obj; }

  screen::screen() : _obj(NULL), bitmap() { }

  screen::~screen()
  {
    close();
  }

  bool screen::clear(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
  {
    if (! _obj) { throw false; }
    set_as_target();
    al_clear_to_color(al_map_rgba(r, g, b, a));
    return true;
  }

  bool screen::close()
  {
    if (_obj)
    {
      delete cast_screen(_obj);
      _obj = NULL;
      return true;
    }
    return false;
  }

  boost::shared_ptr<screen> screen::create(const char *title, int w, int h, int x, int y)
  {
    boost::shared_ptr<screen> scr;
    try {
      scr.reset(new screen);
      if (! scr) { throw -1; }
      scr->_obj = myScreen::Create(title, w, h, x, y);
      if (! scr->_obj) { throw -2; }
    }
    catch (...) {
      scr.reset();
      lev::debug_print("error on screen instance creation");
    }
    return scr;
  }

//  screen* screen::create_context()
//  boost::shared_ptr<screen> screen::create_context()
//  {
//    return screen::create(this);
//  }

  bool screen::flip()
  {
    al_flip_display();
    return true;
  }

  int screen::get_h() const
  {
    if (! _obj) { return 0; }
    return al_get_display_height(cast_screen(_obj)->disp);
  }

  long screen::get_id()
  {
    if (! _obj) { return -1; }
    return (long)cast_screen(_obj)->disp;
  }

  luabind::object screen::get_on_button_down()
  {
    if (! _obj) { return luabind::object(); }
    return cast_screen(_obj)->event_funcs[ALLEGRO_EVENT_MOUSE_BUTTON_DOWN];
  }

  luabind::object screen::get_on_button_up()
  {
    if (! _obj) { return luabind::object(); }
    return cast_screen(_obj)->event_funcs[ALLEGRO_EVENT_MOUSE_BUTTON_UP];
  }

  luabind::object screen::get_on_close()
  {
    if (! _obj) { return luabind::object(); }
    return cast_screen(_obj)->event_funcs[ALLEGRO_EVENT_DISPLAY_CLOSE];
  }

  luabind::object screen::get_on_key_down()
  {
    if (! _obj) { return luabind::object(); }
    return cast_screen(_obj)->event_funcs[ALLEGRO_EVENT_KEY_DOWN];
  }

  luabind::object screen::get_on_key_up()
  {
    if (! _obj) { return luabind::object(); }
    return cast_screen(_obj)->event_funcs[ALLEGRO_EVENT_KEY_UP];
  }

  luabind::object screen::get_on_left_down()
  {
    if (! _obj) { return luabind::object(); }
    return cast_screen(_obj)->on_left_down;
  }

  luabind::object screen::get_on_left_up()
  {
    if (! _obj) { return luabind::object(); }
    return cast_screen(_obj)->on_left_up;
  }

  luabind::object screen::get_on_middle_down()
  {
    if (! _obj) { return luabind::object(); }
    return cast_screen(_obj)->on_middle_down;
  }

  luabind::object screen::get_on_middle_up()
  {
    if (! _obj) { return luabind::object(); }
    return cast_screen(_obj)->on_middle_up;
  }

  luabind::object screen::get_on_motion()
  {
    if (! _obj) { return luabind::object(); }
    return cast_screen(_obj)->event_funcs[ALLEGRO_EVENT_MOUSE_AXES];
  }

  luabind::object screen::get_on_right_down()
  {
    if (! _obj) { return luabind::object(); }
    return cast_screen(_obj)->on_right_down;
  }

  luabind::object screen::get_on_right_up()
  {
    if (! _obj) { return luabind::object(); }
    return cast_screen(_obj)->on_right_up;
  }

  luabind::object screen::get_on_wheel()
  {
    if (! _obj) { return luabind::object(); }
    return cast_screen(_obj)->on_wheel;
  }

  luabind::object screen::get_on_wheel_down()
  {
    if (! _obj) { return luabind::object(); }
    return cast_screen(_obj)->on_wheel_down;
  }

  luabind::object screen::get_on_wheel_up()
  {
    if (! _obj) { return luabind::object(); }
    return cast_screen(_obj)->on_wheel_up;
  }

  boost::shared_ptr<bitmap> screen::get_sub_bitmap(int x, int y, int w, int h)
  {
    boost::shared_ptr<bitmap> sub;
    try {
      sub.reset(new bitmap);
      if (! sub) { throw -1; }
      ALLEGRO_BITMAP* parent = al_get_backbuffer(cast_screen(_obj)->disp);
      sub->_obj = al_create_sub_bitmap(parent, x, y, w, h);
      if (! sub->_obj) { throw -2; }
    }
    catch (...) {
      sub.reset();
      lev::debug_print("error on screen's sub bitmap instance creation");
    }
    return sub;
  }

  int screen::get_w() const
  {
    if (! _obj) { return 0; }
    int w;
    return al_get_display_width(cast_screen(_obj)->disp);
  }

  bool screen::hide()
  {
    if (! _obj) { return false; }
//    SDL_HideWindow(cast_screen(_obj)->win);
  }

  bool screen::is_fullscreen()
  {
    if (! _obj) { return false; }
    return al_get_display_flags(cast_screen(_obj)->disp)
           & ALLEGRO_FULLSCREEN_WINDOW;
  }

  bool screen::is_valid()
  {
    return _obj;
  }

  bool screen::save(const std::string &filename) const
  {
    if (! _obj) { return false; }
    al_save_bitmap(filename.c_str(), al_get_backbuffer(cast_screen(_obj)->disp));
    return true;
  }

  bool screen::set_as_target()
  {
    if (! _obj) { return false; }
    al_set_target_bitmap(al_get_backbuffer(cast_screen(_obj)->disp));
    return true;
  }

  bool screen::set_fullscreen(bool enable)
  {
    if (! _obj) { return false; }
    return al_toggle_display_flag(cast_screen(_obj)->disp,
                                  ALLEGRO_FULLSCREEN_WINDOW, enable);
  }

  bool screen::set_h(int h)
  {
    if (! _obj) { return false; }
    int w = get_w();
    al_resize_display(cast_screen(_obj)->disp, w, h);
  }

  bool screen::set_on_button_down(luabind::object func)
  {
    if (! _obj) { return false; }
    cast_screen(_obj)->event_funcs[ALLEGRO_EVENT_MOUSE_BUTTON_DOWN] = func;
    return true;
  }

  bool screen::set_on_button_up(luabind::object func)
  {
    if (! _obj) { return false; }
    cast_screen(_obj)->event_funcs[ALLEGRO_EVENT_MOUSE_BUTTON_UP] = func;
    return true;
  }

  bool screen::set_on_close(luabind::object func)
  {
    if (! _obj) { return false; }
    cast_screen(_obj)->event_funcs[ALLEGRO_EVENT_DISPLAY_CLOSE] = func;
    return true;
  }

  bool screen::set_on_key_down(luabind::object func)
  {
    if (! _obj) { return false; }
    cast_screen(_obj)->event_funcs[ALLEGRO_EVENT_KEY_DOWN] = func;
    return true;
  }

  bool screen::set_on_key_up(luabind::object func)
  {
    if (! _obj) { return false; }
    cast_screen(_obj)->event_funcs[ALLEGRO_EVENT_KEY_UP] = func;
    return true;
  }

  bool screen::set_on_left_down(luabind::object func)
  {
    if (! _obj) { return false; }
    cast_screen(_obj)->on_left_down = func;
    return true;
  }

  bool screen::set_on_left_up(luabind::object func)
  {
    if (! _obj) { return false; }
    cast_screen(_obj)->on_left_up = func;
    return true;
  }

  bool screen::set_on_middle_down(luabind::object func)
  {
    if (! _obj) { return false; }
    cast_screen(_obj)->on_middle_down = func;
    return true;
  }

  bool screen::set_on_middle_up(luabind::object func)
  {
    if (! _obj) { return false; }
    cast_screen(_obj)->on_middle_up = func;
    return true;
  }

  bool screen::set_on_motion(luabind::object func)
  {
    if (! _obj) { return false; }
    cast_screen(_obj)->event_funcs[ALLEGRO_EVENT_MOUSE_AXES] = func;
    return true;
  }

  bool screen::set_on_right_down(luabind::object func)
  {
    if (! _obj) { return false; }
    cast_screen(_obj)->on_right_down = func;
    return true;
  }

  bool screen::set_on_right_up(luabind::object func)
  {
    if (! _obj) { return false; }
    cast_screen(_obj)->on_right_up = func;
    return true;
  }

  bool screen::set_on_wheel(luabind::object func)
  {
    if (! _obj) { return false; }
    cast_screen(_obj)->on_wheel = func;
    return true;
  }

  bool screen::set_on_wheel_down(luabind::object func)
  {
    if (! _obj) { return false; }
    cast_screen(_obj)->on_wheel_down = func;
    return true;
  }

  bool screen::set_on_wheel_up(luabind::object func)
  {
    if (! _obj) { return false; }
    cast_screen(_obj)->on_wheel_up = func;
    return true;
  }

  bool screen::set_w(int w)
  {
    if (! _obj) { return false; }
    int h = get_h();
    return al_resize_display(cast_screen(_obj)->disp, w, h);
  }

  bool screen::show(bool showing)
  {
    if (! _obj) { return false; }
//    if (showing)
//    {
//      SDL_ShowWindow(cast_screen(_obj)->win);
//      return true;
//    }
//    return hide();
  }

  boost::shared_ptr<bitmap> screen::take_screen_shot()
  {
    boost::shared_ptr<bitmap> img;
    try {
      img = bitmap::create(get_w(), get_h());
      if (! img) { throw -1; }
      img->draw(this);
      return img;
//      img->set_as_target();
//      al_draw_bitmap(al_get_backbuffer(cast_screen(_obj)->disp), 0, 0, 0);
    }
    catch (...) {
//      img.reset();
//      lev::debug_print("error on screenshot bitmap creation");
    }

//    boost::shared_ptr<bitmap> img;
//    try {
//      img.reset(new bitmap);
//      if (! img) { throw -1; }
//      img->_obj = al_clone_bitmap(al_get_backbuffer(cast_screen(_obj)->disp));
//      if (! img->_obj) { throw -2; }
//    }
//    catch (...) {
//      img.reset();
//      lev::debug_print("error on screenshot bitmap creation");
//    }
//    return img;
  }

//  static SDL_GLContext cast_ctx(void *obj) { return (SDL_GLContext)obj; }

//  screen::screen() : base(), _obj(NULL) { }
//
//  screen::~screen()
//  {
//    if (_obj)
//    {
//      SDL_GL_DeleteContext((SDL_GLContext)_obj);
//    }
//  }

//  bool screen::blit(int dst_x, int dst_y, image *src,
//                    int src_x, int src_y, int w, int h, unsigned char alpha)
//  {
//    if (src == NULL) { return false; }
//
//    int src_h = src->get_h();
//    int src_w = src->get_w();
//    if (w < 0) { w = src_w; }
//    if (h < 0) { h = src_h; }
//    glBegin(GL_POINTS);
//      for (int y = 0; y < h; y++)
//      {
//        for (int x = 0; x < w; x++)
//        {
//          int real_src_x = src_x + x;
//          int real_src_y = src_y + y;
//          if (real_src_x < 0 || real_src_x >= src_w || real_src_y < 0 || real_src_y >= src_h)
//          {
//            continue;
//          }
//          boost::shared_ptr<color> pixel(src->get_pixel(real_src_x, real_src_y));
//          if (! pixel) { continue; }
//          unsigned char a = (unsigned short)pixel->get_a() * alpha / 255;
//          if (a > 0)
//          {
//            glColor4ub(pixel->get_r(), pixel->get_g(), pixel->get_b(), a);
//            glVertex2i(dst_x + x, dst_y + y);
//          }
//        }
//      }
//    glEnd();
//    return true;
//  }

//  boost::shared_ptr<screen> screen::create(boost::shared_ptr<window> holder)
//  {
//    boost::shared_ptr<screen> screen;
//
//    if (! holder) { return screen; }
//    try {
//      screen.reset(new lev::screen);
//      if (! screen) { throw -1; }
//      screen->_obj = SDL_GL_CreateContext(cast_win(holder->get_rawobj())->win);
//      if (! screen->_obj) { throw -2; }
//      screen->holder = holder;
//
//      SDL_GL_SetSwapInterval(0);
////      glMatrixMode(GL_PROJECTION);
////      glLoadIdentity();
////      glOrtho(0.0f, 640, 480, 0.0f, 0.0f, 1000.0f);
////      glMatrixMode(GL_MODELVIEW);
//
//      glOrtho(0, holder->get_w(), holder->get_h(), 0, -1, 1);
//      glEnable(GL_BLEND);
//      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//    }
//    catch (...) {
//      screen.reset();
//      lev::debug_print("error on screen instance creation");
//    }
//    return screen;
//  }

//  bool screen::draw(drawable *src, int x, int y, unsigned char alpha)
//  {
//    if (! src) { return false; }
//    set_current();
//    return src->draw_on_screen(this, x, y, alpha);
//  }

//  int screen::draw_l(lua_State *L)
//  {
//    using namespace luabind;
//
//    try {
//      luaL_checktype(L, 1, LUA_TUSERDATA);
//      screen *s = object_cast<screen *>(object(from_stack(L, 1)));
//      object t = util::get_merged(L, 2, -1);
//      int x = 0, y = 0;
//      unsigned char a = 255;
//
//      if (t["x"]) { x = object_cast<int>(t["x"]); }
//      else if (t["lua.number1"]) { x = object_cast<int>(t["lua.number1"]); }
//
//      if (t["y"]) { y = object_cast<int>(t["y"]); }
//      else if (t["lua.number2"]) { y = object_cast<int>(t["lua.number2"]); }
//
//      if (t["lua.number3"]) { a = object_cast<int>(t["lua.number3"]); }
//      else if (t["alpha"]) { a = object_cast<int>(t["alpha"]); }
//      else if (t["a"]) { a = object_cast<int>(t["a"]); }
//
//      if (t["lev.drawable1"])
//      {
////printf("IMAGE CAST!\n");
//        object obj = t["lev.drawable1"];
////printf("OBJECT: %s\n", object_cast<const char *>(obj["type_name"]));
//        boost::shared_ptr<drawable> src = object_cast<boost::shared_ptr<drawable> >(obj["cast"](obj));
//        s->draw(src.get(), x, y, a);
////printf("IMAGE END!\n");
//      }
//      else if (t["lev.raster1"])
//      {
//        boost::shared_ptr<color> c;
//        raster *r = object_cast<raster *>(t["lev.raster1"]);
//
//        if (t["color"]) { c = object_cast<boost::shared_ptr<color> >(t["color"]); }
//        else if (t["c"]) { c = object_cast<boost::shared_ptr<color> >(t["c"]); }
//        else if (t["lev.prim.color1"])
//        {
//          c = object_cast<boost::shared_ptr<color> >(t["lev.prim.color1"]);
//        }
//
//        s->draw_raster(r, x, y, c);
//      }
//      else if (t["lev.font1"])
//      {
//        int spacing = 1;
//        boost::shared_ptr<color> c;
//        font *f = object_cast<font *>(t["lev.font1"]);
//        const char *str = NULL;
//
//        if (t["spacing"]) { spacing = object_cast<int>(t["spacing"]); }
//        else if (t["space"]) { spacing = object_cast<int>(t["space"]); }
//        else if (t["s"]) { spacing = object_cast<int>(t["s"]); }
//        else if (t["lua.number3"]) { spacing = object_cast<int>(t["lua.number3"]); }
//
//        if (t["color"]) { c = object_cast<boost::shared_ptr<color> >(t["color"]); }
//        else if (t["c"]) { c = object_cast<boost::shared_ptr<color> >(t["c"]); }
//        else if (t["lev.prim.color1"]) { c = object_cast<boost::shared_ptr<color> >(t["lev.prim.color1"]); }
//
//        if (t["lua.string1"]) { str = object_cast<const char *>(t["lua.string1"]); }
//
//        if (! str)
//        {
//          lua_pushboolean(L, false);
//          return 1;
//        }
//
//        boost::shared_ptr<raster> r = f->rasterize_utf8(str, spacing);
//        if (! r.get()) { throw -2; }
//        s->draw_raster(r.get(), x, y, c);
//      }
//      else
//      {
//        lua_pushboolean(L, false);
//        return 1;
//      }
//      lua_pushboolean(L, true);
//    }
//    catch (std::exception &e)
//    {
//      lev::debug_print(e.what());
//      lev::debug_print("error on drawing");
//    }
//    catch (...) {
//      lev::debug_print("error on drawing");
//    }
//    return 1;
//  }

//  bool screen::draw_pixel(int x, int y, const color &c)
//  {
//    glBegin(GL_POINTS);
//      glColor4ub(c.get_r(), c.get_g(), c.get_b(), c.get_a());
//      glVertex2i(x, y);
//    glEnd();
//    return true;
//  }

//  bool screen::draw_raster(const raster *r, int offset_x, int offset_y, boost::shared_ptr<const color> c)
//  {
//    if (! r) { return false; }
//
//    if (! c) { c = color::white(); }
//    if (! c) { return false; }
//    glBegin(GL_POINTS);
//      for (int y = 0; y < r->get_h(); y++)
//      {
//        for (int x = 0; x < r->get_w(); x++)
//        {
//          unsigned char a = (unsigned short)c->get_a() * r->get_pixel(x, y) / 255;
//          if (a > 0)
//          {
//            glColor4ub(c->get_r(), c->get_g(), c->get_b(), a);
//            glVertex2i(offset_x + x, offset_y + y);
//          }
//        }
//      }
//    glEnd();
//    return true;
//  }

//  bool screen::enable_alpha_blending(bool enable)
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

//  bool screen::map2d_auto()
//  {
//    if (boost::shared_ptr<window> win = holder.lock())
//    {
//      int w = win->get_w();
//      int h = win->get_h();
//      set_current();
//      glLoadIdentity();
//      glOrtho(0, w, h, 0, -1, 1);
//      return true;
//    }
//    return false;
//  }

//  bool screen::map2d(int left, int right, int top, int bottom)
//  {
//    set_current();
//    glLoadIdentity();
//    glOrtho(left, right, bottom, top, -1, 1);
//    return true;
//  }

//  bool screen::swap()
//  {
//    if (boost::shared_ptr<window> win = holder.lock())
//    {
//      if (win->get_id() < 0) { return false; }
//      SDL_GL_SwapWindow(cast_win(win->get_rawobj())->win);
//      return true;
//    }
//    return false;
//  }

//  debug_window::debug_window() : window(), ptr_screen() { }

//  debug_window::~debug_window()
//  {
//    if (_obj)
//    {
//      delete cast_win(_obj);
//      _obj = NULL;
//    }
//  }

//  bool debug_window::clear()
//  {
//    ptr_screen->clear();
//    ptr_layout->clear();
//    ptr_screen->swap();
//    log = "";
//    buffer = "";
//    return true;
//  }

//  boost::shared_ptr<debug_window> debug_window::get()
//  {
//    if (singleton && singleton->_obj != NULL) { return singleton; }
//    return boost::shared_ptr<debug_window>();
//  }

//  boost::shared_ptr<debug_window> debug_window::init()
//  {
//    const int w = 640;
//    const int h = 480;
//    // singleton already exists
//    if (singleton) { return singleton; }
//    // system is not yet initialized
//    if (! system::get()) { return singleton; }
//    try {
//      singleton.reset(new debug_window);
//      if (! singleton) { throw -1; }
////      long flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
//      long flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
//      singleton->_obj = myWindow::Create("Lev Debug Window", 0, 0, w, h, flags);
//      if (! singleton->_obj) { throw -2; }
//      singleton->ptr_screen = screen::create(singleton);
//      if (! singleton->ptr_screen) { throw -3; }
//      singleton->ptr_layout = layout::create(w);
//      if (! singleton->ptr_layout) { throw -4; }
//    }
//    catch (...) {
//      singleton.reset();
//      lev::debug_print("error on debug window singleton initialization");
//    }
//    return singleton;
//  }

//  bool debug_window::print(const std::string &message_utf8, int font_size)
//  {
//    if (! _obj) { return false; }
//    if (! ptr_layout || ! ptr_layout->get_font()) { return false; }
//    if (font_size <= 0) { return false; }
//    try {
//      ptr_screen->set_current();
//      boost::shared_ptr<unicode> str = unicode::from_utf8(message_utf8);
//      if (! str) { throw -1; }
//      ptr_layout->get_font()->set_pixel_size(font_size);
//      for (int i = 0; i < str->length(); i++)
//      {
//        std::string unit = str->index_str(i)->to_utf8();
//        if (unit == "\r") { continue; }
//        else if (unit == "\n") { ptr_layout->reserve_new_line(); }
//        else { ptr_layout->reserve_word(unit); }
//      }
//      ptr_layout->complete();
//      ptr_screen->clear();
//      if (ptr_layout->get_h() > get_h())
//      {
//        ptr_screen->draw(ptr_layout.get(), 0, - (ptr_layout->get_h() - get_h()), 255);
//      }
//      else
//      {
//        ptr_screen->draw(ptr_layout.get(), 0, 0, 255);
//      }
//      ptr_screen->swap();
//      log += message_utf8;
//      buffer += message_utf8;
//
//      if (ptr_layout->get_h() > get_h() * 2)
//      {
//        // messages are to long, purging the oldest part
//        str = unicode::from_utf8(buffer);
//        str = str->sub_string1(str->length() / 4);
//        buffer = str->to_utf8();
//        ptr_layout->clear();
//
//        for (int i = 0; i < str->length(); i++)
//        {
//          std::string unit = str->index_str(i)->to_utf8();
//          if (unit == "\r") { continue; }
//          else if (unit == "\n") { ptr_layout->reserve_new_line(); }
//          else { ptr_layout->reserve_word(unit); }
//        }
//      }
//    }
//    catch (...) {
//      fprintf(stderr, "error on debug printing\n");
//      return false;
//    }
//    return true;
//  }

//  bool debug_window::stop()
//  {
//    if (singleton and singleton->_obj)
//    {
//      singleton->close();
//    }
//    singleton.reset();
//    return true;
//  }

}

