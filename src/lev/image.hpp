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
      virtual rect::ptr get_rect() const = 0;
      virtual size::ptr get_size() const = 0;
      virtual boost::shared_ptr<texture> get_texture() const = 0;

      virtual type_id get_type_id() const { return LEV_TBITMAP; }
//      static bitmap* levana_icon();
      static bitmap::ptr load(const std::string &filename);
      static bitmap::ptr load_file(boost::shared_ptr<class file> f);
      static bitmap::ptr load_path(boost::shared_ptr<class filepath> path);
      virtual bitmap::ptr resize(int width, int height) = 0;
      virtual bool save(const std::string &filename) const = 0;
      virtual bool set_pixel(int x, int y, const color &c) = 0;
      virtual bitmap::ptr sub(int x, int y, int w, int h) = 0;
      virtual bitmap::ptr to_bitmap() = 0;
  };

  class texture : public drawable
  {
    public:
      typedef boost::shared_ptr<texture> ptr;
    protected:
      texture() { }
    public:
      virtual ~texture() { }
      virtual bool blit_on(boost::shared_ptr<class screen> dst,
                           int dst_x = 0, int dst_y = 0,
                           int src_x = 0, int src_y = 0,
                           int w = -1, int h = -1,
                           unsigned char alpha = 255) const = 0;
      static texture::ptr create(bitmap::ptr src);
      virtual type_id get_type_id() const { return LEV_TTEXTURE; }
      static boost::shared_ptr<texture> load(const std::string &file);
  };

  class animation : public drawable
  {
    public:
      typedef boost::shared_ptr<animation> ptr;
    protected:
      animation() : drawable() { }
    public:
      virtual ~animation() { }

      virtual bool append(boost::shared_ptr<drawable> img, double duration) = 0;
      static animation::ptr create(bool repeating = true);
      static animation::ptr create0() { return create(); }
      virtual drawable::ptr get_current() const = 0;
      virtual type_id get_type_id() const { return LEV_TANIMATION; }
  };

  class transition : public drawable
  {
    public:
      typedef boost::shared_ptr<transition> ptr;
    protected:
      transition() : drawable() { }
    public:
      virtual ~transition() { }
      static boost::shared_ptr<transition> create(boost::shared_ptr<drawable> img);
      static boost::shared_ptr<transition> create_with_file(boost::shared_ptr<class file> f);
      static boost::shared_ptr<transition> create_with_path(boost::shared_ptr<filepath> path);
      static boost::shared_ptr<transition> create_with_string(const std::string &image_path);
      static boost::shared_ptr<transition> create0() { return create(boost::shared_ptr<drawable>()); }
      virtual drawable::ptr get_current() = 0;
      virtual type_id get_type_id() const { return LEV_TTRANSITION; }
      virtual bool is_running() const = 0;
      virtual bool rewind() = 0;
      virtual bool set_current(boost::shared_ptr<drawable> current) = 0;
      virtual bool set_current(const std::string &image_path) = 0;
      virtual bool set_next(drawable::ptr next, double duration = 1, const std::string &mode = "") = 0;
      virtual bool set_next(const std::string &image_path, double duration = 1,
                            const std::string &mode = "") = 0;
  };

  class layout : public clickable
  {
    public:
      typedef boost::shared_ptr<layout> ptr;
    protected:
      layout() : clickable() { }
    public:
      virtual ~layout() { }
      virtual bool clear() = 0;
      virtual bool complete() = 0;
      static boost::shared_ptr<layout> create(int width_stop = -1);
      static layout::ptr create0() { return create(); }
      virtual boost::shared_ptr<class color> get_fg_color() = 0;
      virtual boost::shared_ptr<class font> get_font() = 0;
      virtual boost::shared_ptr<class font> get_ruby_font() = 0;
      virtual boost::shared_ptr<class color> get_shade_color() = 0;
      virtual type_id get_type_id() const { return LEV_TLAYOUT; }
      virtual bool is_done() const = 0;
      virtual bool rearrange() = 0;
      virtual bool reserve_clickable(drawable::ptr normal, drawable::ptr hover,
                                     luabind::object lsingle_func, luabind::object hover_func) = 0;
      virtual bool reserve_clickable_text(const std::string &text,
                                          luabind::object lsingle_func, luabind::object hover_func) = 0;
      virtual bool reserve_image(drawable::ptr img, bool auto_filling = true) = 0;
      bool reserve_image1(drawable::ptr img) { return reserve_image(img); }
      virtual bool reserve_new_line() = 0;

      virtual bool reserve_word(const std::string &word, const std::string &ruby = "",
                                bool auto_filling = true) = 0;
      bool reserve_word_filling(const std::string &word, bool auto_filling = true)
      { return reserve_word(word, "", auto_filling); }
      bool reserve_word1(const std::string &word) { return reserve_word(word); }
      bool reserve_word2(const std::string &word, const std::string &ruby = "")
      { return reserve_word(word, ruby); }

      virtual bool set_fg_color(boost::shared_ptr<color> fg) = 0;
      virtual bool set_font(boost::shared_ptr<font> f) = 0;
      virtual bool set_ruby_font(boost::shared_ptr<font> f) = 0;
      virtual bool set_shade_color(boost::shared_ptr<color> sh) = 0;
      virtual bool show_next() = 0;
  };

}

#endif // _IMAGE_HPP

