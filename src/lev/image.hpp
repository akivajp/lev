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
#include "draw.hpp"
#include "prim.hpp"
#include <luabind/luabind.hpp>

extern "C" {
  int luaopen_lev_image(lua_State *L);
}

namespace lev
{

  // class dependency
  class file_path;
  class font;
  class raster;
  class window;
  typedef boost::shared_ptr<class screen> screen_ptr;

  class bitmap : public canvas
  {
    public:
      typedef boost::shared_ptr<bitmap> ptr;
    protected:
      bitmap() : canvas() { }
    public:
      virtual ~bitmap() { }

      // clone method
      virtual bitmap::ptr clone() { return bitmap::ptr(); }

      // create method (static)
      static bitmap::ptr create(int width, int height);

      // get methods
      virtual unsigned char *get_buffer() { return NULL; }
      virtual const unsigned char *get_buffer() const { return NULL; }
      virtual rect::ptr get_rect() const { return rect::ptr(); }
      virtual size::ptr get_size() const { return size::ptr(); }
      virtual boost::shared_ptr<texture> get_texture() const
      { return boost::shared_ptr<texture>(); }

      virtual type_id get_type_id() const { return LEV_TBITMAP; }
//      static bitmap* levana_icon();
      static bitmap::ptr load(const std::string &filename);
      static bitmap::ptr load_path(boost::shared_ptr<file_path> path);
      virtual bitmap::ptr resize(int width, int height) { return bitmap::ptr(); }
      virtual bool save(const std::string &filename) const { return false; }
      virtual bool set_pixel(int x, int y, const color &c) { return false; }
      virtual bitmap::ptr sub(int x, int y, int w, int h) { return bitmap::ptr(); }
//      static bitmap::ptr take_screenshot(boost::shared_ptr<window> win) { return bitmap::ptr(); }
      virtual bitmap::ptr to_bitmap() { return bitmap::ptr(); }
  };

  class texture : public drawable
  {
    public:
      typedef boost::shared_ptr<texture> ptr;
    protected:
      texture();
    public:
      ~texture();
      virtual bool blit_on(boost::shared_ptr<class screen> dst,
                           int dst_x = 0, int dst_y = 0,
                           int src_x = 0, int src_y = 0,
                           int w = -1, int h = -1,
                           unsigned char alpha = 255) const;
      static texture::ptr create(bitmap::ptr src);
//      virtual bool draw_on_screen(screen_ptr dst, int x = 0, int y = 0, unsigned char alpha = 255) const;
      virtual int get_h() const;
      virtual type_id get_type_id() const { return LEV_TTEXTURE; }
      virtual int get_w() const;
      virtual bool is_texturized() { return true; }
      static boost::shared_ptr<texture> load(const std::string &file);
    protected:
      void *_obj;
  };

  class animation : public drawable
  {
    protected:
      animation() : drawable() { }
    public:
      virtual ~animation() { }

      virtual bool append(boost::shared_ptr<drawable> img, double duration) = 0;
      virtual bool append_file(const std::string &filename, double duration) = 0;
      virtual bool append_path(const file_path *path, double duration) = 0;
      static animation::ptr create(bool repeating = true);
      static animation::ptr create0() { return create(); }
      virtual drawable::ptr get_current() const = 0;
      virtual type_id get_type_id() const { return LEV_TANIMATION; }
    protected:
      void *_obj;
  };

  class transition : public drawable
  {
    protected:
      transition();
    public:
      virtual ~transition();
      virtual bool draw_on(canvas::ptr dst, int x = 0, int y = 0, unsigned char alpha = 255);
      static boost::shared_ptr<transition> create(boost::shared_ptr<drawable> img);
      static boost::shared_ptr<transition> create_with_path(boost::shared_ptr<file_path> path);
      static boost::shared_ptr<transition> create_with_string(const std::string &image_path);
      static boost::shared_ptr<transition> create0() { return create(boost::shared_ptr<drawable>()); }
      virtual int get_h() const;
      virtual type_id get_type_id() const { return LEV_TTRANSITION; }
      virtual int get_w() const;
      bool is_running();
      bool rewind();
      bool set_current(boost::shared_ptr<drawable> current);
      bool set_current(const std::string &image_path);
      static int set_current_l(lua_State *L);
      bool set_next(boost::shared_ptr<drawable> next, double duration = 1, const std::string &mode = "");
      bool set_next(const std::string &image_path, double duration = 1, const std::string &mode = "");
      static int set_next_l(lua_State *L);
      virtual bool texturize(bool force = false);
    protected:
      void *_obj;
  };

  class layout : public drawable
  {
    public:
      typedef boost::shared_ptr<layout> ptr;
    protected:
      layout();
    public:
      virtual ~layout();
      virtual bool clear();
      bool complete();
      static boost::shared_ptr<layout> create(int width_stop = -1);
      virtual bool draw_on(canvas::ptr dst, int x = 0, int y = 0, unsigned char alpha = 255);
      boost::shared_ptr<color> get_fg_color();
      boost::shared_ptr<font> get_font();
      virtual int get_h() const;
      boost::shared_ptr<font> get_ruby_font();
      boost::shared_ptr<color> get_shade_color();
      virtual type_id get_type_id() const { return LEV_TLAYOUT; }
      virtual int get_w() const;
      bool is_done();
      bool on_hover(int x, int y);
      bool on_left_click(int x, int y);
      bool rearrange();
      bool reserve_clickable(boost::shared_ptr<bitmap> normal, boost::shared_ptr<bitmap> hover,
                             luabind::object lclick_func, luabind::object hover_func);
      bool reserve_clickable_text(const std::string &text,
                                  luabind::object lclick_func, luabind::object hover_func);
      bool reserve_image(boost::shared_ptr<bitmap> img);
      bool reserve_new_line();
      bool reserve_word(const std::string &word, const std::string &ruby = "");
      bool reserve_word_lua(luabind::object word, luabind::object ruby);
      bool reserve_word_lua1(luabind::object word);
      bool set_fg_color(boost::shared_ptr<color> fg);
      bool set_font(boost::shared_ptr<font> f);
      bool set_shade_color(const color *c);
      bool set_ruby_font(boost::shared_ptr<font> f);
      bool show_next();
      virtual bool texturize(bool force = false);
    protected:
      void *_obj;
  };

}

#endif // _IMAGE_HPP

