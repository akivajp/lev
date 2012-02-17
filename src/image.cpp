/////////////////////////////////////////////////////////////////////////////
// Name:        src/image.cpp
// Purpose:     source for image handling classes
// Author:      Akiva Miura <akiva.miura@gmail.com>
// Modified by:
// Created:     01/25/2011
// Copyright:   (C) 2010-2011 Akiva Miura
// Licence:     MIT License
/////////////////////////////////////////////////////////////////////////////

// pre-compiled header
#include "prec.h"

// declarations
#include "lev/image.hpp"

// dependencies
#include "lev/debug.hpp"
#include "lev/font.hpp"
#include "lev/fs.hpp"
#include "lev/map.hpp"
#include "lev/util.hpp"
#include "lev/system.hpp"
#include "lev/timer.hpp"
#include "lev/window.hpp"
#include "register.hpp"
//#include "resource/levana.xpm"

// libraries
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <luabind/adopt_policy.hpp>
#include <luabind/luabind.hpp>

#include "stb_image.c"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int luaopen_lev_image(lua_State *L)
{
  using namespace luabind;
  using namespace lev;

  open(L);
  globals(L)["package"]["loaded"]["lev.image"] = true;
  globals(L)["require"]("lev.base");
  globals(L)["require"]("lev.draw");
  globals(L)["require"]("lev.font");
  globals(L)["require"]("lev.prim");

  module(L, "lev")
  [
    namespace_("image"),
    namespace_("classes")
    [
      class_<image, drawable, boost::shared_ptr<base> >("image")
        .def("blit", &image::blit)
        .def("blit", &image::blit1)
        .def("blit", &image::blit2)
        .def("blit", &image::blit3)
        .def("blit", &image::blit4)
        .def("clear", &image::clear)
        .def("clear", &image::clear0)
        .def("clear", &image::clear_rect)
        .def("clear", &image::clear_rect1)
        .def("clone", &image::clone)
        .def("draw_pixel", &image::draw_pixel)
        .def("draw_raster", &image::draw_raster)
//        .def("fill_circle", &image::fill_circle)
        .def("fill_rect", &image::fill_rect)
        .def("fill_rectangle", &image::fill_rect)
        .def("get_color", &image::get_pixel)
        .def("get_pixel", &image::get_pixel)
        .def("load", &image::reload)
        .property("rect",  &image::get_rect)
        .def("reload", &image::reload)
        .def("resize", &image::resize)
        .def("save", &image::save)
        .def("set_color", &image::set_pixel)
        .def("set_pixel", &image::set_pixel)
        .property("sz",  &image::get_size)
        .property("size",  &image::get_size)
//        .def("stroke_circle", &image::stroke_circle)
//        .def("stroke_line", &image::stroke_line)
//        .def("stroke_line", &image::stroke_line6)
//        .def("stroke_rect", &image::stroke_rect)
//        .def("stroke_rectangle", &image::stroke_rect)
        .scope
        [
          def("create",  &image::create),
//          def("draw_text_c", &image::draw_text),
//          def("levana_icon", &image::levana_icon),
          def("load",    &image::load),
          def("load",    &image::load_path),
          def("string_c",  &image::string),
          def("sub_image_c", &image::sub_image)
        ],
      class_<texture, drawable, boost::shared_ptr<base> >("texture")
        .scope
        [
          def("create", &texture::create),
          def("create", &texture::load)
        ],
//      class_<animation, drawable>("animation")
//        .def("add", &animation::add_file)
//        .property("current", &animation::get_current)
//        .scope
//        [
//          def("create", &animation::create, adopt(result)),
//          def("create", &animation::create0, adopt(result))
//        ],
      class_<transition, drawable, boost::shared_ptr<base> >("transition")
        .property("is_running", &transition::is_running)
        .def("rewind", &transition::rewind)
        .scope
        [
          def("create", &transition::create),
          def("create", &transition::create0),
          def("create", &transition::create_with_path),
          def("create", &transition::create_with_string)
        ],
      class_<layout, drawable, boost::shared_ptr<base> >("layout")
        .def("clear", &layout::clear)
        .property("color",  &layout::get_fg_color, &layout::set_fg_color)
        .def("complete", &layout::complete)
        .property("fg",  &layout::get_fg_color, &layout::set_fg_color)
        .property("fg_color", &layout::get_fg_color, &layout::set_fg_color)
        .property("font",  &layout::get_font, &layout::set_font)
        .property("fore",  &layout::get_fg_color, &layout::set_fg_color)
        .property("is_done", &layout::is_done)
        .def("on_hover", &layout::on_hover)
        .def("on_lclick", &layout::on_left_click)
        .def("on_left_click", &layout::on_left_click)
        .def("rearrange", &layout::rearrange)
        .def("reserve_clickable", &layout::reserve_clickable)
        .def("reserve_clickable", &layout::reserve_clickable_text)
        .def("reserve_image", &layout::reserve_image)
        .def("reserve_new_line", &layout::reserve_new_line)
        .def("reserve_word", &layout::reserve_word_lua)
        .def("reserve_word", &layout::reserve_word_lua1)
        .property("ruby",  &layout::get_ruby_font, &layout::set_ruby_font)
        .property("ruby_font",  &layout::get_ruby_font, &layout::set_ruby_font)
        .property("shade", &layout::get_shade_color, &layout::set_shade_color)
        .property("shade_color", &layout::get_shade_color, &layout::set_shade_color)
        .property("space",  &layout::get_spacing, &layout::set_spacing)
        .property("spacing",  &layout::get_spacing, &layout::set_spacing)
        .def("show_next", &layout::show_next)
        .property("text_font",  &layout::get_font, &layout::set_font)
        .scope
        [
          def("create", &layout::create)
        ],
      class_<map, drawable, boost::shared_ptr<base> >("map")
        .def("clear", &map::clear)
        .def("on_hover", &map::on_hover)
        .def("on_lclick", &map::on_left_click)
        .def("on_left_click", &map::on_left_click)
        .def("pop_back", &map::pop_back)
        .scope
        [
          def("create", &map::create)
        ]
    ]
  ];
  object lev = globals(L)["lev"];
  object classes = lev["classes"];
  object image = lev["image"];
//  register_to(classes["image"], "draw_text", &image::draw_text_l);
  register_to(classes["image"], "draw", &image::draw_l);
  register_to(classes["image"], "get_sub", &image::sub_image_l);
  register_to(classes["image"], "get_sub_image", &image::sub_image_l);
  register_to(classes["image"], "string", &image::string_l);
  register_to(classes["image"], "sub", &image::sub_image_l);
  register_to(classes["image"], "sub_image", &image::sub_image_l);
  register_to(classes["map"], "map_image", &map::map_image_l);
  register_to(classes["map"], "map_link", &map::map_link_l);
  register_to(classes["transition"], "set_current", &transition::set_current_l);
  register_to(classes["transition"], "set_next", &transition::set_next_l);

//  image["animation"]   = classes["animation"]["create"];
  image["create"]      = classes["image"]["create"];
  image["layout"]      = classes["layout"]["create"];
//  image["levana_icon"] = classes["image"]["levana_icon"];
  image["load"]        = classes["image"]["load"];
  image["map"]         = classes["map"]["create"];
  image["string"]      = classes["image"]["string"];
  image["tex2d"]       = classes["texture"]["create"];
  image["texture"]     = classes["texture"]["create"];
  image["transition"]  = classes["transition"]["create"];

  globals(L)["package"]["loaded"]["lev.image"] = image;
  return 0;
}


namespace lev
{

  class myImage
  {
    public:
      myImage(int w, int h) : w(w), h(h), tex() { }

      virtual ~myImage()
      {
        if (buf) { delete [] buf; }
      }

      static myImage* Create(int w, int h)
      {
        if (w <= 0 || h <= 0) { return NULL; }
        myImage *img = NULL;
        try {
          img = new myImage(w, h);
          img->buf = new unsigned char [w * h * 4];
          return img;
        }
        catch (...) {
          delete img;
          return NULL;
        }
      }

      bool OnChange()
      {
        if (tex)
        {
          tex.reset();
        }
        return true;
      }

      bool Texturize(image *orig, bool force = false)
      {
        if (tex && !force) { return false; }
        tex = texture::create(orig);
        return true;
      }

      int w, h;
      unsigned char *buf;

//      texture* tex;
      boost::shared_ptr<texture> tex;
  };

  static myImage *cast_image(void *obj) { return (myImage *)obj; }

  static bool blend_pixel(unsigned char *dst, unsigned char *src)
  {
    if (src[3] == 0) { return true; }

    unsigned char dst_r = dst[0];
    unsigned char dst_g = dst[1];
    unsigned char dst_b = dst[2];
    unsigned char dst_a = dst[3];
    unsigned char src_r = src[0];
    unsigned char src_g = src[1];
    unsigned char src_b = src[2];
    unsigned char src_a = src[3];

    if (dst_a == 0 || src_a == 255)
    {
      dst[0] = src_r;
      dst[1] = src_g;
      dst[2] = src_b;
      dst[3] = src_a;
    }
    else if (dst_a == 255)
    {
      unsigned char base_alpha = 255 - src_a;
      dst[0] = ((unsigned short)src_r * src_a + (unsigned short)dst_r * base_alpha) / 255;
      dst[1] = ((unsigned short)src_g * src_a + (unsigned short)dst_g * base_alpha) / 255;
      dst[2] = ((unsigned short)src_b * src_a + (unsigned short)dst_b * base_alpha) / 255;
      // dst[3] = 255;
    }
    else
    {
      unsigned char base_alpha = (unsigned short)dst_a * (255 - src_a) / 255;
      dst[3] = src_a + base_alpha;
      dst[0] = ((unsigned short)src_r * src_a + (unsigned short)dst_r * base_alpha) / dst[3];
      dst[1] = ((unsigned short)src_g * src_a + (unsigned short)dst_g * base_alpha) / dst[3];
      dst[2] = ((unsigned short)src_b * src_a + (unsigned short)dst_b * base_alpha) / dst[3];
    }
    return true;
  }

  static bool blend_pixel(unsigned char *dst, const color &c)
  {
    if (c.get_a() == 0) { return true; }

    unsigned char dst_r = dst[0];
    unsigned char dst_g = dst[1];
    unsigned char dst_b = dst[2];
    unsigned char dst_a = dst[3];
    unsigned char src_r = c.get_r();
    unsigned char src_g = c.get_g();
    unsigned char src_b = c.get_b();
    unsigned char src_a = c.get_a();

    if (dst_a == 0 || src_a == 255)
    {
      dst[0] = src_r;
      dst[1] = src_g;
      dst[2] = src_b;
      dst[3] = src_a;
    }
    else if (dst_a == 255)
    {
      unsigned char base_alpha = 255 - src_a;
      dst[0] = ((unsigned short)src_r * src_a + (unsigned short)dst_r * base_alpha) / 255;
      dst[1] = ((unsigned short)src_g * src_a + (unsigned short)dst_g * base_alpha) / 255;
      dst[2] = ((unsigned short)src_b * src_a + (unsigned short)dst_b * base_alpha) / 255;
      // dst[3] = 255;
    }
    else
    {
      unsigned char base_alpha = (unsigned short)dst_a * (255 - src_a) / 255;
      dst[3] = src_a + base_alpha;
      dst[0] = ((unsigned short)src_r * src_a + (unsigned short)dst_r * base_alpha) / dst[3];
      dst[1] = ((unsigned short)src_g * src_a + (unsigned short)dst_g * base_alpha) / dst[3];
      dst[2] = ((unsigned short)src_b * src_a + (unsigned short)dst_b * base_alpha) / dst[3];
    }
    return true;
  }

  image::image() : drawable(), _obj(NULL) { }

  image::~image()
  {
    if (_obj)
    {
      delete cast_image(_obj);
      _obj = NULL;
    }
  }

  bool image::blit(int dst_x, int dst_y, image *src,
                   int src_x, int src_y, int w, int h, unsigned char alpha)
  {
    if (src == NULL) { return false; }

    unsigned char *dst_buf = cast_image(this->_obj)->buf;
    unsigned char *src_buf = cast_image(src->_obj)->buf;
    int dst_h = get_h();
    int dst_w = get_w();
    int src_h = src->get_h();
    int src_w = src->get_w();
    if (w < 0) { w = src_w; }
    if (h < 0) { h = src_h; }
    for (int y = 0; y < h; y++)
    {
      for (int x = 0; x < w; x++)
      {
        int real_src_x = src_x + x;
        int real_src_y = src_y + y;
        if (real_src_x < 0 || real_src_x >= src_w || real_src_y < 0 || real_src_y >= src_h)
        { continue; }
        int real_dst_x = dst_x + x;
        int real_dst_y = dst_y + y;
        if (real_dst_x < 0 || real_dst_x >= dst_w || real_dst_y < 0 || real_dst_y >= dst_h)
        { continue; }
        unsigned char *src_pixel = &src_buf[4 * (real_src_y * src_w + real_src_x)];
        blend_pixel(&dst_buf[4 * (real_dst_y * dst_w + real_dst_x)], src_pixel);
      }
    }
    cast_image(_obj)->OnChange();
    return true;
  }

  bool image::clear(const color &c)
  {
    return clear_rect(0, 0, get_w(), get_h(), c);
  }

  bool image::clear_rect(int offset_x, int offset_y, int w, int h, const color &c)
  {
    int img_w = get_w();
    int img_h = get_h();
    unsigned char *pixel = cast_image(_obj)->buf;
    unsigned char r = c.get_r(), g = c.get_g(), b = c.get_b(), a = c.get_a();
    if (a > 0)
    {
      for (int y = 0; y < h; y++)
      {
        for (int x = 0; x < w; x++)
        {
          int real_x = offset_x + x;
          int real_y = offset_y + y;
          if (real_x < 0 || real_x >= img_w || real_y < 0 || real_y >= img_h)
          {
            pixel += 4;
            continue;
          }
          pixel[0] = r;
          pixel[1] = g;
          pixel[2] = b;
          pixel[3] = a;
          pixel += 4;
        }
      }
    }
    else
    {
      for (int y = 0; y < h; y++)
      {
        for (int x = 0; x < w; x++)
        {
          int real_x = offset_x + x;
          int real_y = offset_y + y;
          if (real_x < 0 || real_x >= img_w || real_y < 0 || real_y >= img_h)
          {
            pixel += 4;
            continue;
          }
          pixel[3] = 0;
          pixel += 4;
        }
      }
    }
    cast_image(_obj)->OnChange();
    return true;
  }

  bool image::clear_rect2(const rect &r, const color &c)
  {
    return clear_rect(r.get_x(), r.get_y(), r.get_w(), r.get_h(), c);
  }

  boost::shared_ptr<image> image::clone()
  {
    boost::shared_ptr<image> img;
    try {
      img.reset(new image);
      if (! img) { throw -1; }
      img->_obj = myImage::Create(get_w(), get_h());
      if (! img->_obj) { throw -2; }
      long length = 4 * get_w() * get_h();
      unsigned char *src_buf = cast_image(_obj)->buf;
      unsigned char *new_buf = cast_image(img->_obj)->buf;
      for (int i = 0; i < length; i++)
      {
        new_buf[i] = src_buf[i];
      }
    }
    catch (...) {
      img.reset();
      lev::debug_print("error on image memory allocation or copying");
    }
    return img;
  }

//  image* image::create(int width, int height)
  boost::shared_ptr<image> image::create(int width, int height)
  {
//    image *img = NULL;
    boost::shared_ptr<image> img;
    if (width <= 0 || height <= 0) { return img; }
    try {
      img.reset(new image);
      if (! img) { throw -1; }
      img->_obj = myImage::Create(width, height);
      if (! img->_obj) { throw -2; }
      img->clear();
    }
    catch (...) {
      img.reset();
      lev::debug_print("error on image memory allocation");
    }
    return img;
  }

  bool image::draw(drawable *src, int x, int y, unsigned char alpha)
  {
    if (! src) { return false; }
    cast_image(_obj)->OnChange();
    return src->draw_on_image(this, x, y, alpha);
  }

  bool image::draw_on_image(image *dst, int offset_x, int offset_y, unsigned char alpha)
  {
    if (! dst) { return false; }
    return dst->blit(offset_x, offset_y, this, 0, 0, get_w(), get_h(), alpha);
  }

  bool image::draw_on_screen(screen *dst, int offset_x, int offset_y, unsigned char alpha)
  {
    if (! dst) { return false; }
    if (is_texturized())
    {
      return cast_image(_obj)->tex->draw_on_screen(dst, offset_x, offset_y, alpha);
    }

    int w = get_w();
    int h = get_h();
    unsigned char *pixel = cast_image(_obj)->buf;
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

  bool image::draw_pixel(int x, int y, const color &c)
  {
    if (x < 0 || x >= get_w() || y < 0 || y >= get_h()) { return false; }
    unsigned char *buf = cast_image(_obj)->buf;
    unsigned char *pixel = &buf[4 * (y * get_w() + x)];
    cast_image(_obj)->OnChange();
    return blend_pixel(pixel, c);
  }

  bool image::draw_raster(const raster *r, int offset_x, int offset_y, const color *c)
  {
    if (! r) { return false; }

    color orig = color::white();
    if (c) { orig = *c; }
    color copy = orig;
    for (int y = 0; y < r->get_h(); y++)
    {
      for (int x = 0; x < r->get_w(); x++)
      {
        copy.set_a((unsigned short)orig.get_a() * r->get_pixel(x, y) / 255);
        draw_pixel(offset_x + x, offset_y + y, copy);
      }
    }
    cast_image(_obj)->OnChange();
    return true;
  }

  int image::draw_l(lua_State *L)
  {
    using namespace luabind;

    try {
      luaL_checktype(L, 1, LUA_TUSERDATA);
      image *img = object_cast<image *>(object(from_stack(L, 1)));
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
        image *src = object_cast<image *>(t["lev.image1"]);

        img->draw(src, x, y, a);
      }
      else if (t["lev.image.layout1"])
      {
        layout *src = object_cast<layout *>(t["lev.image.layout1"]);

        img->draw(src, x, y, a);
      }
      else if (t["lev.raster1"])
      {
        color *c = NULL;
        raster *r = object_cast<raster *>(t["lev.raster1"]);

        if (t["color"]) { c = object_cast<color *>(t["color"]); }
        else if (t["c"]) { c = object_cast<color *>(t["c"]); }
        else if (t["lev.prim.color1"]) { c = object_cast<color *>(t["lev.prim.color1"]); }

        img->draw_raster(r, x, y, c);
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

        if (! str) { throw -1; }

        boost::shared_ptr<raster> r = f->rasterize_utf8(str, spacing);
        if (! r) { throw -2; }
        img->draw_raster(r.get(), x, y, c);
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
      lua_pushboolean(L, false);
      return 1;
    }
  }

  int image::get_h() const
  {
    return cast_image(_obj)->h;
  }
//
//  bool image::fill_circle(int x, int y, int radius, color *filling)
//  {
//    wxBitmap tmp(get_w(), get_h(), 32);
//    try {
//      wxMemoryDC mdc(tmp);
//      mdc.SetPen(wxColour(255, 255, 255, 255));
//      mdc.SetBrush(wxColour(255, 255, 255, 255));
//      mdc.SetBackground(wxColour(0, 0, 0, 255));
//      mdc.Clear();
//      mdc.DrawCircle(x, y, radius);
//    }
//    catch (...) {
//      return false;
//    }
//    return image_draw_mask(this, &tmp, filling);
//  }

  bool image::fill_rect(int offset_x, int offset_y, int w, int h, color *filling)
  {
    for (int y = 0; y < h; y++)
    {
      for (int x = 0; x < w; x++)
      {
        draw_pixel(offset_x + x, offset_y + y, *filling);
      }
    }
    cast_image(_obj)->OnChange();
    return true;
  }

  boost::shared_ptr<color> image::get_pixel(int x, int y)
  {
    boost::shared_ptr<color> c;
    try {
      if (x < 0 || x >= get_w() || y < 0 || y >= get_h()) { throw -1; }
      unsigned char *buf = cast_image(_obj)->buf;
      unsigned char *pixel = &buf[4 * (y * get_w() + x)];
      c = color::create(pixel[0], pixel[1], pixel[2], pixel[3]);
      if (! c) { throw -2; }
    }
    catch (...) {
      c.reset();
    }
    return c;
  }

  boost::shared_ptr<rect> image::get_rect() const
  {
    return rect::create(0, 0, get_w(), get_h());
  }

  boost::shared_ptr<size> image::get_size() const
  {
    return size::create(get_w(), get_h());
  }

  int image::get_w() const
  {
    return cast_image(_obj)->w;
  }

  bool image::is_compiled() { return false; }

  bool image::is_texturized()
  {
    if (cast_image(_obj)->tex) { return true; }
    return false;
  }

//  image* image::levana_icon()
//  {
//    static image *img = NULL;
//    wxBitmap *obj = NULL;
//
//    if (img) { return img; }
//    try {
//      img = new image;
//      img->_obj = obj = new wxBitmap(levana_xpm);
//      img->_status = new myImageStatus;
//      return img;
//    }
//    catch (...) {
//      delete img;
//      return NULL;
//    }
//  }

  boost::shared_ptr<image> image::load(const std::string &filename)
  {
    boost::shared_ptr<image> img;
    try {
      if (! file_system::file_exists(filename)) { throw -1; }
      int w, h;
      boost::shared_ptr<unsigned char> buf(stbi_load(filename.c_str(), &w, &h, NULL, 4), stbi_image_free);
      if (! buf) { throw -2; }
      img = image::create(w, h);
      if (! img) { throw -3; }

      for (int y = 0; y < h; y++)
      {
        for (int x = 0; x < w; x++)
        {
          unsigned char *pixel = buf.get() + (y * w + x) * 4;
          unsigned char r, g, b, a;
          r = pixel[0];
          g = pixel[1];
          b = pixel[2];
          a = pixel[3];
          img->set_pixel(x, y, color(r, g, b, a));
        }
      }
    }
    catch (...) {
      img.reset();
      lev::debug_print("error on image data loading");
    }
    return img;
  }

  boost::shared_ptr<image> image::load_path(boost::shared_ptr<file_path> path)
  {
    return load(path->get_full_path());
  }

  bool image::reload(const std::string &filename)
  {
    boost::shared_ptr<image> img = image::load(filename);
    if (! img) { return false; }
    this->swap(img);
    return true;
  }

  boost::shared_ptr<image> image::resize(int width, int height)
  {
    boost::shared_ptr<image> img;
    try {
      img = image::create(width, height);
      for (int y = 0; y < height; y++)
      {
        for (int x = 0; x < width; x++)
        {
          boost::shared_ptr<color> c;
          c = get_pixel(long(x) * get_w() / width, long(y) * get_h() / height);
          if (c) { img->set_pixel(x, y, *c); }
        }
      }
    }
    catch (...) {
      img.reset();
      lev::debug_print("error on resized image creation");
    }
    return img;
  }

  bool image::save(const std::string &filename) const
  {
    unsigned char *buf = cast_image(_obj)->buf;
    if (stbi_write_png(filename.c_str(), get_w(), get_h(), 4, buf, 4 * get_w()) != 0)
    { return true; }
    else { return false; }
  }

  bool image::set_pixel(int x, int y, const color &c)
  {
    if (x < 0 || x >= get_w() || y < 0 || y >= get_h()) { return false; }
    unsigned char *buf = cast_image(_obj)->buf;
    unsigned char *pixel = &buf[4 * (y * get_w() + x)];
    pixel[0] = c.get_r();
    pixel[1] = c.get_g();
    pixel[2] = c.get_b();
    pixel[3] = c.get_a();
    cast_image(_obj)->OnChange();
    return true;
  }


  boost::shared_ptr<image> image::string(font *f, const std::string &str,
                                         const color *fore,
                                         const color *shade,
                                         const color *back,
                                         int spacing)
  {
    boost::shared_ptr<image> img;
    if (! f) { return img; }

    color fg = color::white();
    color bg = color::transparent();
    if (fore) { fg = *fore; }
    if (back) { bg = *back; }

    try {
      boost::shared_ptr<raster> r = f->rasterize_utf8(str, spacing);
      if (! r) { throw -1; }

      if (shade)
      {
        img = image::create(r->get_w() + 1, r->get_h() + 1);
        if (! img) { throw -2; }
        img->clear(bg);
        img->draw_raster(r.get(), 1, 1, shade);
        img->draw_raster(r.get(), 0, 0, &fg);
      }
      else
      {
        img = image::create(r->get_w(), r->get_h());
        if (! img) { throw -2; }
        img->clear(bg);
        img->draw_raster(r.get(), 0, 0, &fg);
      }
    }
    catch (...) {
      img.reset();
      lev::debug_print("error on string image instance creation");
    }
    return img;
  }

  int image::string_l(lua_State *L)
  {
    using namespace luabind;

    try {
      const char *str = NULL;
      object font, fore, back, shade;
      int spacing = 1;

      object t = util::get_merged(L, 1, -1);
      if (t["lua.string1"]) { str = object_cast<const char *>(t["lua.string1"]); }
      else if (t["lev.unistr1"]) { str = object_cast<const char *>(t["lev.unistr1"]["str"]); }
      else if (t["text"]) { str = object_cast<const char *>(t["text"]); }
      else if (t["t"]) { str = object_cast<const char *>(t["t"]); }
      else if (t["string"]) { str = object_cast<const char *>(t["string"]); }
      else if (t["str"]) { str = object_cast<const char *>(t["str"]); }
      if (not str)
      {
        luaL_error(L, "text (string) is not specified!\n");
        return 0;
      }

      if (t["lev.font"]) { font = t["lev.font"]; }
      else if (t["font"]) { font = t["font"]; }
      else if (t["f"]) { font = t["f"]; }
      else
      {
        font = globals(L)["lev"]["font"]["load"]();
      }

      if (t["lev.prim.color1"]) { fore = t["lev.prim.color1"]; }
      else if (t["fg_color"]) { fore = t["fg_color"]; }
      else if (t["fg"]) { fore = t["fg"]; }
      else if (t["fore"]) { fore = t["fore"]; }
      else if (t["f"]) { fore = t["f"]; }
      else if (t["color"]) { fore = t["color"]; }
      else if (t["c"]) { fore = t["c"]; }

      if (t["lev.prim.color2"]) { shade = t["lev.prim.color2"]; }
      else if (t["shade_color"]) { shade = t["shade_color"]; }
      else if (t["shade"]) { shade = t["shade"]; }
      else if (t["sh"]) { shade = t["sh"]; }
      else if (t["s"]) { shade = t["s"]; }

      if (t["lev.prim.color3"]) { back = t["lev.prim.color3"]; }
      else if (t["bg_color"]) { back = t["bg_color"]; }
      else if (t["bg"]) { back = t["bg"]; }
      else if (t["back"]) { back = t["back"]; }
      else if (t["b"]) { back = t["b"]; }

      if (t["lua.number1"]) { spacing = object_cast<int>(t["lua.number1"]); }
      else if (t["spacing"]) { spacing = object_cast<int>(t["spacing"]); }
      else if (t["space"]) { spacing = object_cast<int>(t["space"]); }

      object o = globals(L)["lev"]["classes"]["image"]["string_c"](font, str, fore, shade, back, spacing);
      o.push(L);
      return 1;
    }
    catch (...) {
      lua_pushnil(L);
      return 1;
    }
  }
//
//  bool image::stroke_circle(int x, int y, int radius, color *border, int width)
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
//    return image_draw_mask(this, &tmp, border);
//  }
//
//  bool image::stroke_line(int x1, int y1, int x2, int y2, color *border, int width,
//                          const std::string &style)
//  {
//    wxBitmap tmp(get_w(), get_h(), 32);
//    try {
//      wxMemoryDC mdc(tmp);
//      if (style == "dot")
//      {
//        mdc.SetPen(wxPen(wxColour(255, 255, 255, 255), width, wxDOT));
//      }
//      else
//      {
//        mdc.SetPen(wxPen(wxColour(255, 255, 255, 255), width));
//      }
//      mdc.SetBrush(wxColour(0, 0, 0, 255));
//      mdc.SetBackground(wxColour(0, 0, 0, 255));
//      mdc.Clear();
//      mdc.DrawLine(x1, y1, x2, y2);
//    }
//    catch (...) {
//      return false;
//    }
//    return image_draw_mask(this, &tmp, border);
//  }
//
//  bool image::stroke_rect(int x, int y, int w, int h, color *border, int width)
//  {
//    wxBitmap tmp(get_w(), get_h(), 32);
//    try {
//      wxMemoryDC mdc(tmp);
//      mdc.SetPen(wxPen(wxColour(255, 255, 255, 255), width));
//      mdc.SetBrush(wxColour(0, 0, 0, 255));
//      mdc.SetBackground(wxColour(0, 0, 0, 255));
//      mdc.Clear();
//      mdc.DrawRectangle(x, y, w, h);
//    }
//    catch (...) {
//      return false;
//    }
//    return image_draw_mask(this, &tmp, border);
//  }

  boost::shared_ptr<image> image::sub_image(int x, int y, int w, int h)
  {
    boost::shared_ptr<image> img;
    try {
      img = image::create(w, h);
      if (! img) { throw -1; }
      img->blit(0, 0, this, x, y, w, h);
    }
    catch (...) {
      img.reset();
      lev::debug_print("error on sub image instance creation");
    }
    return img;
  }

  int image::sub_image_l(lua_State *L)
  {
    using namespace luabind;
    int x = 0, y = 0, w = -1, h = -1;

    luaL_checktype(L, 1, LUA_TUSERDATA);
    image *img = object_cast<image *>(object(from_stack(L, 1)));
    if (img == NULL) { return 0; }
    object t = util::get_merged(L, 2, -1);

    if (t["x"]) { x = object_cast<int>(t["x"]); }
    else if (t["lua.number1"]) { x = object_cast<int>(t["lua.number1"]); }
    if (x < 0) { x = 0; }

    if (t["y"]) { y = object_cast<int>(t["y"]); }
    else if (t["lua.number2"]) { y = object_cast<int>(t["lua.number2"]); }
    if (y < 0) { y = 0; }

    if (t["w"]) { w = object_cast<int>(t["w"]); }
    else if (t["lua.number3"]) { w = object_cast<int>(t["lua.number3"]); }
    if (w < 0) { w = img->get_w() - x; }

    if (t["h"]) { h = object_cast<int>(t["h"]); }
    else if (t["lua.number4"]) { h = object_cast<int>(t["lua.number4"]); }
    if (h < 0) { h = img->get_h() - y; }

    object o = globals(L)["lev"]["classes"]["image"]["sub_image_c"](img, x, y, w, h);
    o.push(L);
    return 1;
  }

//  bool image::swap(image *img)
  bool image::swap(boost::shared_ptr<image> img)
  {
    if (! img) { return false; }
    // swap the images
    void *tmp  = this->_obj;
    this->_obj = img->_obj;
    img->_obj  = tmp;
    cast_image(_obj)->OnChange();
    cast_image(img->get_rawobj())->OnChange();
    return true;
  }

  boost::shared_ptr<image> image::take_screenshot(boost::shared_ptr<window> win)
  {
    boost::shared_ptr<image> img;
    if (! win) { return img; }
    try {
      if (win->get_id() > 0)
      {
        const int w = win->get_w();
        const int h = win->get_h();
        img = image::create(w, h);
        if (! img) { throw -1; }
        glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, cast_image(img->_obj)->buf);
        Uint32 *buf = (Uint32 *)cast_image(img->_obj)->buf;
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
      lev::debug_print("error on screen shot image creation");
    }
    return img;
  }

  bool image::texturize(bool force)
  {
    return cast_image(_obj)->Texturize(this, force);
  }

  class myTexture
  {
    public:
      myTexture(int w, int h) : img_w(w), img_h(h), tex_w(1), tex_h(1)
      {
        while(tex_w < w) { tex_w <<= 1; }
        while(tex_h < h) { tex_h <<= 1; }
        coord_x = (double)w / tex_w;
        coord_y = (double)h / tex_h;
      }

      ~myTexture()
      {
        if (index > 0)
        {
//printf("Rel: %d\n", index);
          glDeleteTextures(1, &index);
          index = 0;
        }
      }

      static myTexture* Create(int w, int h)
      {
        if (w <= 0 || h <= 0) { return NULL; }
        myTexture *tex = NULL;
        try {
          tex = new myTexture(w, h);
          glGenTextures(1, &tex->index);
//printf("Gen: %d\n", tex->index);
          if (tex->index == 0) { throw -1; }
          return tex;
        }
        catch (...) {
//printf("ERROR!\n");
          delete tex;
          return NULL;
        }
      }

      int img_w, img_h;
      int tex_w, tex_h;
      double coord_x, coord_y;
      GLuint index;
  };
  static myTexture* cast_tex(void *obj) { return (myTexture *)obj; }

  texture::texture() : _obj(NULL) { }

  texture::~texture()
  {
    if (_obj) { delete cast_tex(_obj); }
  }

  boost::shared_ptr<texture> texture::create(const image *src)
  {
    boost::shared_ptr<texture> tex;
    if (! src) { return tex; }
    try {
      myTexture *obj = NULL;
      tex.reset(new texture);
      if (! tex) { throw -1; }
      tex->_obj = obj = myTexture::Create(src->get_w(), src->get_h());
      if (! tex->_obj) { throw -2; }

      glBindTexture(GL_TEXTURE_2D, obj->index);
      glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexImage2D(GL_TEXTURE_2D, 0 /* level */, GL_RGBA, obj->tex_w, obj->tex_h, 0 /* border */,
                   GL_RGBA, GL_UNSIGNED_BYTE, NULL /* only buffer reservation */);
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0 /* x offset */, 0 /* y offset */,
                      obj->img_w, obj->img_h, GL_RGBA, GL_UNSIGNED_BYTE,
                      cast_image(src->get_rawobj())->buf);
    }
    catch (...) {
      tex.reset();
      lev::debug_print("error on texture instance creation");
    }
    return tex;
  }

  bool texture::draw_on_screen(screen *target, int x, int y, unsigned char alpha)
  {
    if (! target) { return NULL; }

    myTexture *tex = cast_tex(_obj);
    glBindTexture(GL_TEXTURE_2D, tex->index);
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
      glColor4ub(255, 255, 255, alpha);
      glTexCoord2d(0, 0);
      glVertex2i(x, y);
      glTexCoord2d(0, tex->coord_y);
      glVertex2i(x, y + tex->img_h);
      glTexCoord2d(tex->coord_x, tex->coord_y);
      glVertex2i(x + tex->img_w, y + tex->img_h);
      glTexCoord2d(tex->coord_x, 0);
      glVertex2i(x + tex->img_w, y);

//      glTexCoord2d(0, tex->coord_y);
//      glVertex2i(x, y);
//      glTexCoord2i(0, 0);
//      glVertex2i(x, y + tex->img_h);
//      glTexCoord2d(tex->coord_x, 0);
//      glVertex2i(x + tex->img_w, y + tex->img_h);
//      glTexCoord2d(tex->coord_x, tex->coord_y);
//      glVertex2i(x + tex->img_w, y);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    return true;
  }

  int texture::get_h() const
  {
    return cast_tex(_obj)->img_h;
  }

  int texture::get_w() const
  {
    return cast_tex(_obj)->img_w;
  }

  boost::shared_ptr<texture> texture::load(const std::string &file)
  {
    try {
      boost::shared_ptr<image> img(image::load(file));
      if (! img) { throw -1; }
      return texture::create(img.get());
    }
    catch (...) {
      lev::debug_print("error on texture image loading");
      return boost::shared_ptr<texture>();
    }
  }

  /*
  class myAnimation
  {
    public:

      myAnimation(bool repeating = true) : imgs(), repeating(repeating)
      {
        sw.Start(0);
      }

      ~myAnimation() { }

      bool AddFile(const std::string &filename, int duration)
      {
        if (duration <= 0) { return false; }
        try {
          boost::shared_ptr<image> img(image::load(filename));
          if (! img) { throw -1; }
          imgs.push_back(img);
          durations.push_back(duration);
          return true;
        }
        catch (...) {
          return false;
        }
      }

      bool CompileAll(canvas *cv, bool force)
      {
        for (int i = 0; i < imgs.size(); i++)
        {
          cv->compile(imgs[i].get(), force);
        }
        return true;
      }

      image* GetCurrent()
      {
        long now = sw.Time();
        long total = 0;
        if (imgs.size() == 0) { return NULL; }
        for (int i = 0; i < durations.size(); i++)
        {
          if (total <= now && now < total + durations[i])
          {
            return imgs[i].get();
          }
          total += durations[i];
        }
        if (! repeating) { return imgs[imgs.size() - 1].get(); }
        sw.Start(sw.Time() - total);
        return GetCurrent();
      }

      bool TexturizeAll(canvas *cv, bool force)
      {
        for (int i = 0; i < imgs.size(); i++)
        {
          cv->texturize(imgs[i].get(), force);
        }
        return true;
      }

      bool repeating;
      wxStopWatch sw;
      std::vector<boost::shared_ptr<image> > imgs;
      std::vector<int> durations;
  };
  static myAnimation* cast_anim(void *obj) { return (myAnimation *)obj; }

  animation::animation()
    : drawable(), _obj(NULL)
  { }

  animation::~animation()
  {
    if (_obj) { delete cast_anim(_obj); }
  }

  bool animation::add_file(const std::string &filename, int duration)
  {
    return cast_anim(_obj)->AddFile(filename, duration);
  }

  bool animation::compile(canvas *cv, bool force)
  {
    return cast_anim(_obj)->CompileAll(cv, force);
  }

  image* animation::get_current()
  {
    return cast_anim(_obj)->GetCurrent();
  }

  animation* animation::create(bool repeating)
  {
    animation* anim = NULL;
    try {
      anim = new animation;
      anim->_obj = new myAnimation(repeating);
      return anim;
    }
    catch (...) {
      delete anim;
      return NULL;
    }
  }

  bool animation::texturize(canvas *cv, bool force)
  {
    return cast_anim(_obj)->TexturizeAll(cv, force);
  }


*/

  class myTransition
  {
    public:

      enum transition_mode
      {
        LEV_TRAN_NONE = 0,
        LEV_TRAN_CROSS_FADE,
        LEV_TRAN_FADE_OUT,
      };

    protected:

      myTransition() : imgs(), sw(), texturized(false) { }

    public:

      ~myTransition() { }

      static myTransition *Create(boost::shared_ptr<drawable> img)
      {
        myTransition *tran = NULL;
        try {
          tran = new myTransition;
          if (! tran) { throw -1; }
          tran->sw = stop_watch::create();
          if (! tran->sw) { throw -2; }

          tran->sw->start();
          tran->imgs.push_back(img);
          return tran;
        }
        catch (...) {
          delete tran;
          return NULL;
        }
      }

      bool DrawOn(screen *cv, int x, int y, unsigned char alpha)
      {
        double grad = 1.0;

        if (durations.size() >= 1 && durations[0] > 0)
        {
          grad = ((double)sw->get_time()) / durations[0];
          if (grad > 1.0) { grad = 1.0; }
        }

//printf("IMGS.SIZE: %p, %d\n", imgs[0].get(), imgs.size());
//printf("ALPHA: %ld\n", alpha);
        if (imgs.size() == 0) { return false; }
        if (imgs[0])
        {
          if (modes.size() >= 1 && modes[0] == LEV_TRAN_FADE_OUT)
          {
//printf("FADE: alpha: %d, glad: %lf\n", (int)alpha, grad);
            imgs[0]->draw_on_screen(cv, x, y, (1 - grad) * alpha);
          }
          else { imgs[0]->draw_on_screen(cv, x, y, alpha); }
        }

        if (imgs.size() == 1) { return true; }
        if (imgs[1])
        {
          if (modes[0] == LEV_TRAN_CROSS_FADE)
          {
//printf("IMGS[1]: %p, %d\n", imgs[1].get(), (int)imgs.size());
//printf("1 FADE: alpha: %d, glad: %lf\n", (int)alpha, grad);
            imgs[1]->draw_on_screen(cv, x, y, alpha * grad);
          }
        }

        if (sw->get_time() >= durations[0])
        {
          sw->start(sw->get_time() - durations[0]);
          imgs.erase(imgs.begin());
          durations.erase(durations.begin());
          modes.erase(modes.begin());
        }
        return true;
      }

      bool Rewind()
      {
        return sw->start(0);
      }

      bool SetCurrent(boost::shared_ptr<drawable> img)
      {
        imgs.clear();
        durations.clear();
        modes.clear();
        imgs.push_back(img);
        texturized = false;
        return true;
      }

      bool SetNext(boost::shared_ptr<drawable> img, int duration, const std::string &type)
      {
        if (duration < 0) { return false; }

        try {
          imgs.push_back(img);
          durations.push_back(duration);
          texturized = false;

          if (type == "cross_fade") { modes.push_back(LEV_TRAN_CROSS_FADE); }
          else if (type == "crossfade") { modes.push_back(LEV_TRAN_CROSS_FADE); }
          else if (type == "fade") { modes.push_back(LEV_TRAN_CROSS_FADE); }
          else if (type == "fade_out") { modes.push_back(LEV_TRAN_FADE_OUT); }
          else if (type == "fadeout") { modes.push_back(LEV_TRAN_FADE_OUT); }
          else { modes.push_back(LEV_TRAN_CROSS_FADE); }
        }
        catch (...) {
          return false;
        }
        return true;
      }

      bool TexturizeAll(bool force)
      {
        if (texturized && !force) { return false; }
        for (int i = 0; i < imgs.size(); i++)
        {
          if (! imgs[i]) { continue; }
          imgs[i]->texturize(force);
        }
        texturized = true;
        return true;
      }

      std::vector<boost::shared_ptr<drawable> > imgs;
      std::vector<double> durations;
      std::vector<transition_mode> modes;
      boost::shared_ptr<stop_watch> sw;
      bool texturized;
  };
  static myTransition* cast_tran(void *obj) { return (myTransition *)obj; }

  transition::transition() : drawable(), _obj(NULL) { }

  transition::~transition()
  {
    if (_obj) { delete cast_tran(_obj); }
  }

  boost::shared_ptr<transition> transition::create(boost::shared_ptr<drawable> img)
  {
    boost::shared_ptr<transition> tran;
    try {
      tran.reset(new transition);
      if (! tran) { throw -1; }
      tran->_obj = myTransition::Create(img);
      if (! tran->_obj) { throw -2; }
    }
    catch (...) {
      tran.reset();
      lev::debug_print("error on transition instance creation");
    }
    return tran;
  }

  boost::shared_ptr<transition>
    transition::create_with_path(boost::shared_ptr<file_path> path)
  {
    return transition::create_with_string(path->get_full_path());
  }

  boost::shared_ptr<transition> transition::create_with_string(const std::string &image_path)
  {
    return transition::create(image::load(image_path));
  }

  bool transition::draw_on_screen(screen *cv, int x, int y, unsigned char alpha)
  {
    return cast_tran(_obj)->DrawOn(cv, x, y, alpha);
  }

  int transition::get_h() const
  {
    if (cast_tran(_obj)->imgs[0]) { return cast_tran(_obj)->imgs[0]->get_h(); }
    else { return 0; }
  }

  int transition::get_w() const
  {
    if (cast_tran(_obj)->imgs[0]) { return cast_tran(_obj)->imgs[0]->get_w(); }
    else { return 0; }
  }

  bool transition::is_running()
  {
    if (cast_tran(_obj)->imgs.size() <= 1) { return false; }
    else { return true; }
  }

  bool transition::rewind()
  {
    return cast_tran(_obj)->Rewind();
  }

  bool transition::set_current(boost::shared_ptr<drawable> current)
  {
    return cast_tran(_obj)->SetCurrent(current);
  }

  bool transition::set_current(const std::string &image_path)
  {
    return cast_tran(_obj)->SetCurrent(image::load(image_path));
  }

  int transition::set_current_l(lua_State *L)
  {
    using namespace luabind;

    try {
      bool result = false;

      luaL_checktype(L, 1, LUA_TUSERDATA);
      transition *tran = object_cast<transition *>(object(from_stack(L, 1)));
      object t = util::get_merged(L, 2, -1);

      if (t["lev.image1"])
      {
        boost::shared_ptr<image> img = object_cast<boost::shared_ptr<image> >(t["lev.image1"]);
        result = tran->set_current(img);
      }
      else if (t["lev.fs.file_path1"])
      {
        file_path* path = object_cast<file_path *>(t["lev.fs.file_path1"]);
        result = tran->set_current(path->get_full_path());
      }
      else if (t["lua.string1"])
      {
        std::string path = object_cast<const char *>(t["lua.string1"]);
        result = tran->set_current(path);
      }
      else
      {
        result = tran->set_current(boost::shared_ptr<drawable>());
      }

      lua_pushboolean(L, result);
    }
    catch (...) {
      lev::debug_print(lua_tostring(L, -1));
      lev::debug_print("error on transition current image setting");
      lua_pushnil(L);
    }
    return 1;
  }

  bool transition::set_next(boost::shared_ptr<drawable> next, double duration, const std::string &mode)
  {
    return cast_tran(_obj)->SetNext(next, duration, mode);
  }

  bool transition::set_next(const std::string &image_path, double duration,
                                        const std::string &mode)
  {
    return cast_tran(_obj)->SetNext(image::load(image_path), duration, mode);
  }

  int transition::set_next_l(lua_State *L)
  {
    using namespace luabind;

    try {
      double duration = 1;
      std::string mode = "";
      bool result = false;

      luaL_checktype(L, 1, LUA_TUSERDATA);
      transition *tran = object_cast<transition *>(object(from_stack(L, 1)));
      object t = util::get_merged(L, 2, -1);

      if (t["duration"]) { duration = object_cast<double>(t["duration"]); }
      else if (t["d"]) { duration = object_cast<double>(t["d"]); }
      else if (t["lua.number1"]) { duration = object_cast<double>(t["lua.number1"]); }

      if (t["mode"]) { mode = object_cast<const char *>(t["mode"]); }
      else if (t["m"]) { mode = object_cast<const char *>(t["m"]); }
      else if (t["type"]) { mode = object_cast<const char *>(t["type"]); }
      else if (t["t"]) { mode = object_cast<const char *>(t["t"]); }

      if (t["lev.image1"])
      {
        image* img = object_cast<image *>(t["lev.image1"]);
        result = tran->set_next(img->clone(), duration, mode);
      }
      else if (t["lev.fs.file_path1"])
      {
        boost::shared_ptr<file_path> path =
          object_cast<boost::shared_ptr<file_path> >(t["lev.fs.file_path1"]);
        result = tran->set_next(path->get_full_path(), duration, mode);
      }
      else if (t["lua.string1"])
      {
        std::string path = object_cast<const char *>(t["lua.string1"]);
        result = tran->set_next(path, duration, mode);
      }
      else
      {
        result = tran->set_next(boost::shared_ptr<drawable>(), duration, mode);
      }

      lua_pushboolean(L, result);
    }
    catch (...) {
      lev::debug_print(lua_tostring(L, -1));
      lev::debug_print("error on transition next image setting");
      lua_pushnil(L);
    }
    return 1;
  }

  bool transition::texturize(bool force)
  {
    return cast_tran(_obj)->TexturizeAll(force);
  }

  class myLayout
  {
    protected:

      struct myItem
      {
        public:
          myItem() : x(-1), y(-1), fixed(false) { }
          boost::shared_ptr<drawable> img;
          boost::shared_ptr<drawable> img_hover;
          boost::shared_ptr<drawable> img_showing;
          luabind::object func_hover;
          luabind::object func_lclick;
          int x, y;
          bool fixed;
      };

      myLayout(int width_stop = -1)
        : width_stop(width_stop),
          font_text(), font_ruby(), spacing(1),
          items(), texturized(false)
      {
        font_text = font::load0();
        font_ruby = font::load0();
        if (font_ruby) { font_ruby->set_pixel_size(font_ruby->get_pixel_size() / 2); }
        color_fg = color::white();
        color_shade = boost::shared_ptr<color>(new color(color::black()));
        hover_bg = color::transparent();
        hover_fg = color::red();
      }

    public:

      ~myLayout() { }

      static myLayout* Create(int width_stop)
      {
        myLayout *lay = NULL;
        try {
          lay = new myLayout(width_stop);
          if (! lay) { throw -1; }
          return lay;
        }
        catch (...) {
          delete lay;
          return NULL;
        }
      }

      int CalcMaxWidth()
      {
        int max_w = 0;
        int x = 0;
        for (int i = 0; i < items.size(); i++)
        {
          myItem &item = items[i];
          if (! item.img || (width_stop > 0 && x > 0 && x + item.img->get_w() > width_stop))
          {
            // newline
            if (x > max_w) { max_w = x; }
            x = 0;
          }
          if (item.img)
          {
            x += item.img->get_w();
          }
        }
        return (max_w > x ? max_w : x);
      }

      bool CalcPosition(int index)
      {
        int x = 0;
        int y = 0;
        int max_h = 0;

        if (index < 0 || index >= items.size()) { return false; }
        if (! items[index].img) { return false; }

        // if position is already fixed, return it
        if (items[index].fixed) { return true; }

        // back scan
        int i = index - 1;
        for (; i >= 0; i--)
        {
          myItem &item = items[i];
          if (item.fixed)
          {
            x = item.x;
            y = item.y;
            max_h = item.img->get_h();
            break;
          }
        }
        if (i < 0) { i = 0; }

        // fore scan
        for (; i < items.size(); i++)
        {
          myItem &item = items[i];
          if (! item.img || (width_stop > 0 && x > 0 && x + item.img->get_w() > width_stop))
          {
            // newline
            x = 0;
            y += max_h;
            max_h = 0;
            // back scan for position fixing
            for (int j = i - 1; j >= 0; j--)
            {
              if (items[j].fixed) { break; }
              if (items[j].img)
              {
                items[j].y = y - items[j].img->get_h();
                items[j].fixed = true;
              }
            }
            if (i > index)
            {
              return true;
            }
          }
          if (item.img)
          {
            // calc by next item
            item.x = x;
            x += item.img->get_w();
            if (item.img->get_h() > max_h) { max_h = item.img->get_h(); }
          }
        }
        y += max_h;
        items[index].y = y - items[index].img->get_h();
        return true;
      }

      int CalcTotalHeight()
      {
        int x = 0;
        int y = 0;
        int max_h = 0;
        for (int i = 0; i < items.size(); i++)
        {
          myItem &item = items[i];
          if (! item.img || (width_stop > 0 && x > 0 && x + item.img->get_w() > width_stop))
          {
            // newline
            x = 0;
            y += max_h;
            max_h = 0;
          }
          if (item.img)
          {
            // calc by next item
            x += item.img->get_w();
            if (item.img->get_h() > max_h) { max_h = item.img->get_h(); }
          }
        }
        y += max_h;
        return y;
      }

      bool Clear()
      {
        items.clear();
        texturized = false;
        return true;
      }

      bool Complete()
      {
        for ( ; ; )
        {
          int i = GetNextIndex();
          if (i < 0) { return true; }
          ShowIndex(i);
        }
      }

      bool DrawOn(image *dst, int x, int y, unsigned char alpha)
      {
        for (int i = 0; i < items.size(); i++)
        {
          myItem &item = items[i];
          if (item.img_showing)
          {
            CalcPosition(i);
            item.img_showing->draw_on_image(dst, x + item.x, y + item.y, alpha);
          }
        }
        return true;
      }

      bool DrawOn(screen *dst, int x, int y, unsigned char alpha)
      {
        for (int i = 0; i < items.size(); i++)
        {
          myItem &item = items[i];
          if (item.img_showing)
          {
            CalcPosition(i);
            item.img_showing->draw_on_screen(dst, x + item.x, y + item.y, alpha);
          }
        }
//printf("\n");
        return true;
      }

      int GetNextIndex()
      {
        for (int i = 0; i < items.size(); i++)
        {
          if (! items[i].img) { continue; }
          if (! items[i].img_showing) { return i; }
        }
        return -1;
      }

      bool OnHover(int x, int y)
      {
        using namespace luabind;

        for (int i = 0; i < items.size(); i++)
        {
          myItem &item = items[i];

          if (! item.img_hover) { continue; }
          if (! item.img_showing) { continue; }

          CalcPosition(i);
          rect r(item.x, item.y, item.img->get_w(), item.img->get_h());
          if (r.include(x, y))
          {
            // (x, y) is in the rect
            if (item.img_showing != item.img_hover)
            {
              if (item.func_hover && type(item.func_hover) == LUA_TFUNCTION)
              {
                item.func_hover(x, y);
              }
              item.img_showing = item.img_hover;
            }
          }
          else
          {
            // (x, y) isn't in the rect
            if (item.img_showing != item.img)
            {
              item.img_showing = item.img;
            }
          }
        }
        return true;
      }

      bool OnLeftClick(int x, int y)
      {
        using namespace luabind;

        for (int i = 0; i < items.size(); i++)
        {
          myItem &item = items[i];

          if (! item.img_hover) { continue; }
          if (! item.img_showing) { continue; }

          CalcPosition(i);
          rect r(item.x, item.y, item.img->get_w(), item.img->get_h());
          if (r.include(x, y))
          {
            if (item.func_lclick && type(item.func_lclick) == LUA_TFUNCTION)
            {
              item.func_lclick(x, y);
            }
            return true;
          }
        }
        return false;
      }

      bool RecalcAll()
      {
        for (int i = 0; i < items.size(); i++)
        {
          items[i].fixed = false;
        }
        for (int i = 0; i < items.size(); i++)
        {
          CalcPosition(i);
        }
        return true;
      }

      bool ReserveClickable(boost::shared_ptr<image> normal,
                            boost::shared_ptr<image> hover,
                            luabind::object lclick_func,
                            luabind::object hover_func)
      {
        try {
          if (! normal) { throw -1; }
          if (! hover) { hover = normal->clone(); }

          items.push_back(myItem());
          myItem &i = items[items.size() - 1];
          i.img = normal;
          i.img_hover = hover;
          i.func_hover = hover_func;
          i.func_lclick = lclick_func;
          texturized = false;
          return true;
        }
        catch (...) {
          return false;
        }
      }

      bool ReserveClickableText(const std::string &text,
                                luabind::object lclick_func,
                                luabind::object hover_func)
      {
        if (! font_text) { return false; }
        if (text.empty()) { return false; }
        try {
          boost::shared_ptr<image> img;
          boost::shared_ptr<image> hover_img;

          img = image::string(font_text.get(), text, &color_fg, color_shade.get(), NULL, spacing);
//          img->stroke_line(0, img->get_h() - 1,
//                           img->get_w() - 1, img->get_h() - 1, c, 1, "dot");
          hover_img = image::string(font_text.get(), text, &hover_fg, &hover_bg);
          return ReserveClickable(img, hover_img, lclick_func, hover_func);
        }
        catch (...) {
          return false;
        }
      }

      bool ReserveImage(boost::shared_ptr<drawable> img)
      {
        try {
          if (! img) { throw -1; }

          items.push_back(myItem());
          (items.end() - 1)->img = img;
          texturized = false;
          return true;
        }
        catch (...) {
          return false;
        }
      }

      bool ReserveNewLine()
      {
        // height spacing by image
        items.push_back(myItem());
//        (items.end() - 1)->img = image::create(1, font_text->get_pixel_size());
        (items.end() - 1)->img = image::create(1, font_text->get_pixel_size() * 1.25);
        // adding new line
        items.push_back(myItem());
        texturized = false;
        return true;
      }

      bool ReserveWord(const std::string &word, const std::string &ruby = "")
      {
        if (! font_text) { return false; }
        if (! ruby.empty() && ! font_ruby) { return false; }
        if (word.empty()) { return false; }
        try {
          boost::shared_ptr<image> img;
          if (ruby.empty()) {
            img = image::string(font_text.get(), word,
                                &color_fg, color_shade.get(), NULL, spacing);
            return ReserveImage(img);
          }
          else
          {
            boost::shared_ptr<image> img_ruby;
            boost::shared_ptr<image> img_word;
            img_ruby = image::string(font_ruby.get(), ruby, &color_fg, color_shade.get(), NULL, spacing);
            img_word = image::string(font_text.get(), word, &color_fg, color_shade.get(), NULL, spacing);
            if (!img_ruby || !img_word) { throw -1; }
            int h = img_ruby->get_h() + img_word->get_h();
            int w = img_ruby->get_w();
            if (img_word->get_w() > w) { w = img_word->get_w(); }

            img = image::create(w, h);
            img->draw(img_ruby.get(), (w - img_ruby->get_w()) / 2, 0);
            img->draw(img_word.get(), (w - img_word->get_w()) / 2, img_ruby->get_h());
            return ReserveImage(img);
          }
        }
        catch (...) {
          return false;
        }
      }

      bool ShowIndex(int index)
      {
        if (index >= items.size()) { return false; }
        myItem &item = items[index];
        item.img_showing = item.img;
        return true;
      }

      bool TexturizeAll(bool force)
      {
        if (texturized && !force) { return false; }
        for (int i = 0; i < items.size(); i++)
        {
          myItem &item = items[i];
          if (item.img) { item.img->texturize(force); }
          if (item.img_hover) { item.img_hover->texturize(force); }
        }
        texturized = true;
        return true;
      }

      // format properties
      color color_fg;
      boost::shared_ptr<color> color_shade;
      color hover_bg;
      color hover_fg;
      boost::shared_ptr<font> font_text;
      boost::shared_ptr<font> font_ruby;
      int spacing;
      int width_stop;

      // all items
      std::vector<myItem> items;

      bool texturized;
  };
  static myLayout* cast_lay(void *obj) { return (myLayout *)obj; }

  layout::layout()
    : drawable(), _obj(NULL)
  {
  }

  layout::~layout()
  {
    if (_obj) { delete cast_lay(_obj); }
  }

  bool layout::clear()
  {
    return cast_lay(_obj)->Clear();
  }

  bool layout::complete()
  {
    if (is_done()) { return false; }
    return cast_lay(_obj)->Complete();
  }

  boost::shared_ptr<layout> layout::create(int width_stop)
  {
    boost::shared_ptr<layout> lay;
    try {
      lay.reset(new layout);
      if (! lay) { throw -1; }
      lay->_obj = myLayout::Create(width_stop);
      if (! lay->_obj) { throw -2; }
    }
    catch (...) {
      lay.reset();
      lev::debug_print("error on image layout instance creation");

    }
    return lay;
  }

  bool layout::draw_on_image(image *dst, int x, int y, unsigned char alpha)
  {
    return cast_lay(_obj)->DrawOn(dst, x, y, alpha);
  }

  bool layout::draw_on_screen(screen *dst, int x, int y, unsigned char alpha)
  {
    return cast_lay(_obj)->DrawOn(dst, x, y, alpha);
  }

  color &layout::get_fg_color()
  {
    return cast_lay(_obj)->color_fg;
  }

  boost::shared_ptr<font> layout::get_font()
  {
    return cast_lay(_obj)->font_text;
  }

  int layout::get_h() const
  {
    return cast_lay(_obj)->CalcTotalHeight();
  }

  font *layout::get_ruby_font()
  {
    return cast_lay(_obj)->font_ruby.get();
  }

  boost::shared_ptr<color> layout::get_shade_color()
  {
    return cast_lay(_obj)->color_shade;
  }

  int layout::get_spacing()
  {
    return cast_lay(_obj)->spacing;
  }

  int layout::get_w() const
  {
    if (cast_lay(_obj)->width_stop > 0) { return cast_lay(_obj)->width_stop; }
    return cast_lay(_obj)->CalcMaxWidth();
  }

  bool layout::is_done()
  {
    myLayout *lay = cast_lay(_obj);
    return lay->GetNextIndex() < 0;
  }

  bool layout::on_hover(int x, int y)
  {
    return cast_lay(_obj)->OnHover(x, y);
  }

  bool layout::on_left_click(int x, int y)
  {
    return cast_lay(_obj)->OnLeftClick(x, y);
  }

  bool layout::rearrange()
  {
    return cast_lay(_obj)->RecalcAll();
  }

  bool layout::reserve_clickable(boost::shared_ptr<image> normal,
                                 boost::shared_ptr<image> hover,
                                 luabind::object lclick_func,
                                 luabind::object hover_func)
  {
    return cast_lay(_obj)->ReserveClickable(boost::shared_ptr<image>(normal),
                                            boost::shared_ptr<image>(hover),
                                            lclick_func,
                                            hover_func);
  }

  bool layout::reserve_clickable_text(const std::string &text,
                                      luabind::object lclick_func,
                                      luabind::object hover_func)
  {
    return cast_lay(_obj)->ReserveClickableText(text, lclick_func, hover_func);
  }

  bool layout::reserve_image(boost::shared_ptr<image> img)
  {
    return cast_lay(_obj)->ReserveImage(img);
  }

  bool layout::reserve_new_line()
  {
    return cast_lay(_obj)->ReserveNewLine();
  }

  bool layout::reserve_word(const std::string &word, const std::string &ruby)
  {
    return cast_lay(_obj)->ReserveWord(word, ruby);
  }

  bool layout::reserve_word_lua(luabind::object word, luabind::object ruby)
  {
    return cast_lay(_obj)->ReserveWord(util::tostring(word), util::tostring(ruby));
  }

  bool layout::reserve_word_lua1(luabind::object word)
  {
    return cast_lay(_obj)->ReserveWord(util::tostring(word));
  }

  bool layout::set_fg_color(const color &fg)
  {
    cast_lay(_obj)->color_fg = fg;
    return true;
  }

  bool layout::set_font(font *f)
  {
    if (! f) { return false; }
    cast_lay(_obj)->font_text.reset(f);
    return true;
  }

  bool layout::set_ruby_font(font *f)
  {
    if (! f) { return false; }
    cast_lay(_obj)->font_ruby.reset(f);
    return true;
  }

  bool layout::set_spacing(int space)
  {
    cast_lay(_obj)->spacing = space;
    return true;
  }

  bool layout::set_shade_color(const color *c)
  {
    try {
      if (c) { cast_lay(_obj)->color_shade.reset(new color(*c)); }
      else { cast_lay(_obj)->color_shade.reset(); }
      return true;
    }
    catch (...) {
      return false;
    }
  }

  bool layout::show_next()
  {
    if (is_done()) { return false; }
    myLayout *lay = cast_lay(_obj);
    return lay->ShowIndex(lay->GetNextIndex());
  }

  bool layout::texturize(bool force)
  {
    return cast_lay(_obj)->TexturizeAll(force);
  }

}

