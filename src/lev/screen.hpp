#ifndef _WINDOW_HPP
#define _WINDOW_HPP

/////////////////////////////////////////////////////////////////////////////
// Name:        lev/screen.hpp
// Purpose:     header for screen managing class
// Author:      Akiva Miura <akiva.miura@gmail.com>
// Modified by:
// Created:     06/19/2012
// Copyright:   (C) 2010-2012 Akiva Miura
// Licence:     MIT License
/////////////////////////////////////////////////////////////////////////////

#include "base.hpp"
#include "image.hpp"
#include <boost/shared_ptr.hpp>

extern "C" {
  int luaopen_lev_screen(lua_State *L);
}

namespace lev
{

  class color;

  class screen: public bitmap
  {
    protected:
      screen();
    public:
      ~screen();

      virtual boost::shared_ptr<bitmap> clone() { return take_screen_shot(); }
      virtual bool clear(unsigned char r = 0, unsigned char g = 0, unsigned char b = 0, unsigned char a = 0);
      virtual bool close();
      static boost::shared_ptr<screen> create(const char *title, int w, int h, int x, int y);
      bool flip();
      virtual int get_h() const;
      long get_id();
      virtual luabind::object get_on_button_down();
      virtual luabind::object get_on_button_up();
      virtual luabind::object get_on_close();
      virtual luabind::object get_on_key_down();
      virtual luabind::object get_on_key_up();
      virtual luabind::object get_on_left_down();
      virtual luabind::object get_on_left_up();
      virtual luabind::object get_on_middle_down();
      virtual luabind::object get_on_middle_up();
      virtual luabind::object get_on_motion();
      virtual luabind::object get_on_right_down();
      virtual luabind::object get_on_right_up();
      virtual luabind::object get_on_wheel();
      virtual luabind::object get_on_wheel_down();
      virtual luabind::object get_on_wheel_up();
      void* get_rawobj() { return _obj; }
      virtual boost::shared_ptr<bitmap> get_sub_bitmap(int x, int y, int w, int h);
      virtual type_id get_type_id() const { return LEV_TSCREEN; }
      virtual int get_w() const;
      bool hide();
      bool is_fullscreen();
      bool is_valid();
      virtual bool save(const std::string &filename) const;
      virtual bool set_as_target();
      bool set_fullscreen(bool enable = true);
      bool set_fullscreen0() { return set_fullscreen(); }
      bool set_h(int h);
      virtual bool set_on_button_down(luabind::object func);
      virtual bool set_on_button_up(luabind::object func);
      virtual bool set_on_close(luabind::object func);
      virtual bool set_on_key_down(luabind::object func);
      virtual bool set_on_key_up(luabind::object func);
      virtual bool set_on_left_down(luabind::object func);
      virtual bool set_on_left_up(luabind::object func);
      virtual bool set_on_motion(luabind::object func);
      virtual bool set_on_middle_down(luabind::object func);
      virtual bool set_on_middle_up(luabind::object func);
      virtual bool set_on_right_down(luabind::object func);
      virtual bool set_on_right_up(luabind::object func);
      virtual bool set_on_wheel(luabind::object func);
      virtual bool set_on_wheel_down(luabind::object func);
      virtual bool set_on_wheel_up(luabind::object func);
      bool set_w(int w);
      bool show(bool showing = true);
      bool show0() { return show(); }
      boost::shared_ptr<bitmap> take_screen_shot();
    protected:
      void* _obj;
  };

//  class debug_window : public window
//  {
//    protected:
//      debug_window();
//    public:
//      virtual ~debug_window();
//      bool clear();
////      virtual bool close();
//      static boost::shared_ptr<debug_window> get();
//      std::string get_log() { return log; }
//      virtual type_id get_type_id() const { return LEV_TDEBUG_WINDOW; }
//      static boost::shared_ptr<debug_window> init();
//      bool print(const std::string &message_utf8, int font_size = 18);
//      bool print1(const std::string &message_utf8) { return print(message_utf8); }
//      static bool stop();
//    protected:
//      std::string log, buffer;
//      boost::shared_ptr<screen> ptr_screen;
//      boost::shared_ptr<layout> ptr_layout;
//      static boost::shared_ptr<debug_window> singleton;
//  };

}

#endif // _WINDOW_HPP

