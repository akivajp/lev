#ifndef _IMAGE_HPP
#define _IMAGE_HPP

/////////////////////////////////////////////////////////////////////////////
// Name:        lev/image.hpp
// Purpose:     header for image handling classes
// Author:      Akiva Miura <akiva.miura@gmail.com>
// Modified by:
// Created:     01/25/2011
// Copyright:   (C) 2010-2011 Akiva Miura
// Licence:     MIT License
/////////////////////////////////////////////////////////////////////////////

#include "base.hpp"
#include "prim.hpp"
#include <luabind/luabind.hpp>

extern "C" {
  int luaopen_lev_image(lua_State *L);
}

namespace lev
{

  // class dependencies
//  class canvas;
//  class control;
//  class font;
  class image;

  class drawable : public base
  {
    protected:
      drawable() : base() { }
    public:
      virtual ~drawable() { }
      virtual bool draw_on(image *dst, int x = 0, int y = 0, unsigned char alpha = 255) { return false; }
      virtual bool draw_on1(image *dst) { return draw_on(dst); }
      virtual bool draw_on3(image *dst, int x = 0, int y = 0) { return draw_on(dst); }
      virtual type_id get_type_id() const { return LEV_TDRAWABLE; }
      virtual const char *get_type_name() const { return "lev.drawable"; }
  };

  class image : public drawable
  {
    protected:
      image();
    public:
      virtual ~image();
      bool blit(int src_x, int src_y, int w, int h, image *dst, int dst_x = 0, int dst_y = 0);
      bool blit1(image *dst) { return blit3(dst); }
      bool blit3(image *dst, int dst_x = 0, int dst_y = 0);
      bool blit5(int src_x, int src_y, int w, int h, image *dst)
      { return blit(src_x, src_y, w, h, dst); }

//      static size calc_string(const std::string &str, font *f);
//      static size calc_string1(const std::string &str) { return calc_string(str, NULL); }
      virtual bool clear(const color &c);
      virtual bool clear0() { return clear(color::transparent()); }
      bool clear_rect(const rect &r, const color &c = color::transparent());
      bool clear_rect1(const rect &r) { return clear_rect(r); }
      image* clone();
      static image* create(int width, int height, int depth = 32);
      static image* create2(int width, int height) { return create(width, height); }
      virtual bool draw_on(image *target, int x = 0, int y = 0, unsigned char alpha = 255);
      bool draw_pixel(int x, int y, const color &c);
//      bool draw_text(const std::string &text, font *f = NULL, color *fg = NULL,
//                     color *bg = NULL, int x = 0, int y = 0);
      static int draw_text_l(lua_State *L);
      bool fill_circle(int x, int y, int radius, color *filling);
      bool fill_rect(int x, int y, int w, int h, color *filling);
      virtual int get_h() const;
      color* get_pixel(int x, int y);
      void* get_rawobj() { return _obj; }
      const rect get_rect() const;
      const size get_size() const;
      virtual type_id get_type_id() const { return LEV_TIMAGE; }
      virtual const char *get_type_name() const { return "lev.image"; }
      virtual int get_w() const;
      bool is_compiled();
      bool is_texturized();
      static image* levana_icon();
      static image* load(const std::string &filename);
      bool reload(const std::string &filename);
      bool save(const std::string &filename) const;
      bool set_pixel(int x, int y, const color &c);
//      static image* string(const std::string &str, font *f = NULL, color *fore = NULL, color *back = NULL);
//      static image* string1(const std::string &str) { return string(str, NULL, NULL, NULL); }
      static int string_l(lua_State *L);
      bool stroke_circle(int x, int y, int radius, color *border, int width);
      bool stroke_line(int x1, int y1, int x2, int y2, color *border, int width,
                       const std::string &style = "");
      bool stroke_line6(int x1, int y1, int x2, int y2, color *border, int width)
      { return stroke_line(x1, y1, x2, y2, border, width); }

      bool stroke_rect(int x, int y, int w, int h, color *border, int width);
      image* sub_image(int x, int y, int w, int h);
      static int sub_image_l(lua_State *L);
      bool swap(image *img);
    protected:
      void *_obj;
      void *_status;
  };

  class screen : public image
  {
    protected:
      screen();
    public:
      ~screen();
      bool flip();
      static screen *get();
      virtual type_id get_type_id() const { return LEV_TSCREEN; }
      virtual const char *get_type_name() const { return "lev.image.screen"; }
      static screen *set_mode(int width, int height, int depth = 32);
  };

//  class animation : public drawable
//  {
//    protected:
//      animation();
//    public:
//      virtual ~animation();
//      virtual bool compile(canvas *cv, bool force = false);
//      image* get_current();
//      bool add_file(const std::string &filename, int duration);
//      static animation* create(bool repeating = true);
//      static animation* create0() { return create(); }
//      virtual type_id get_type_id() const { return LEV_TANIMATION; }
//      virtual const char *get_type_name() const { return "lev.image.animation"; }
//      virtual bool texturize(canvas *cv, bool force = false);
//    protected:
//      void *_obj;
//  };

//  class transition : public drawable
//  {
//    protected:
//      transition();
//    public:
//      virtual ~transition();
//      virtual bool draw_on(canvas *cv, int x = 0, int y = 0, unsigned char alpha = 255);
//      virtual bool draw_on1(canvas *cv) { return draw_on(cv); }
//      virtual bool draw_on3(canvas *cv, int x = 0, int y = 0) { return draw_on(cv, x, y); }
//      static transition* create(luabind::object img);
//      static transition* create0() { return create(luabind::object()); }
//      virtual type_id get_type_id() const { return LEV_TTRANSITION; }
//      virtual const char *get_type_name() const { return "lev.image.transition"; }
//      bool is_running();
//      bool set_current(luabind::object current);
//      bool set_next(luabind::object next, int duration = 1000, const std::string &type = "");
//      virtual bool texturize(canvas *cv, bool force = false);
//    protected:
//      void *_obj;
//  };

//  class layout : public drawable
//  {
//    protected:
//      layout();
//    public:
//      virtual ~layout();
//      virtual bool clear(const color &c);
//      virtual bool clear0() { return clear(color::transparent()); }
//      bool complete();
//      static layout* create(int width_stop = -1);
//      virtual bool draw_on(canvas *cv, int x = 0, int y = 0, unsigned char alpha = 255);
//      luabind::object get_color();
//      luabind::object get_font();
//      virtual type_id get_type_id() const { return LEV_TLAYOUT; }
//      virtual const char *get_type_name() const { return "lev.image.layout"; }
//      bool is_done();
//      bool on_hover(int x, int y);
//      bool on_left_click(int x, int y);
//      bool reserve_clickable(const std::string &name, image *normal, image *hover,
//                             luabind::object lclick_func);
//      bool reserve_clickable_text(const std::string &name, const std::string &text,
//                                  luabind::object lclick_func);
//      bool reserve_image(const std::string &name, image *img);
//      bool reserve_new_line();
//      bool reserve_word(const std::string &word, const std::string &ruby);
//      bool reserve_word1(const std::string &word) { return reserve_word(word, ""); }
//      bool set_color(luabind::object c);
//      bool set_font(luabind::object f);
//      bool set_on_hover(const std::string &name, luabind::object hover_func);
//      bool show_next();
//    protected:
//      void *_obj;
//  };

}

#endif // _IMAGE_HPP

