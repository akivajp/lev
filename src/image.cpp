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
#include "lev/entry.hpp"
#include "lev/font.hpp"
#include "lev/fs.hpp"
#include "lev/map.hpp"
#include "lev/util.hpp"
#include "lev/system.hpp"
#include "lev/timer.hpp"
#include "lev/screen.hpp"
//#include "resource/levana.xpm"

// libraries
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <luabind/adopt_policy.hpp>
#include <luabind/luabind.hpp>

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
      class_<bitmap, drawable, boost::shared_ptr<drawable> >("bitmap")
        .def("blit", &bitmap::blit)
        .def("blit", &bitmap::blit1)
        .def("blit", &bitmap::blit2)
        .def("blit", &bitmap::blit3)
        .def("blit", &bitmap::blit4)
        .def("clear", &bitmap::clear)
        .def("clear", &bitmap::clear0)
        .def("clear", &bitmap::clear3)
        .def("clear", &bitmap::clear_color)
        .def("clear", &bitmap::clear_rect)
        .def("clear", &bitmap::clear_rect1)
        .def("clear", &bitmap::clear_rect2)
        .def("clone", &bitmap::clone)
        .def("draw_pixel", &bitmap::draw_pixel)
//        .def("draw_raster", &bitmap::draw_raster)
        .def("fill_circle", &bitmap::fill_circle)
        .def("fill_rect", &bitmap::fill_rectangle)
        .def("fill_rectangle", &bitmap::fill_rectangle)
        .def("get_color", &bitmap::get_pixel)
        .def("get_pixel", &bitmap::get_pixel)
        .def("load", &bitmap::reload)
        .property("rect",  &bitmap::get_rect)
        .def("reload", &bitmap::reload)
        .def("resize", &bitmap::resize)
        .def("save", &bitmap::save)
        .def("set_color", &bitmap::set_pixel)
        .def("set_pixel", &bitmap::set_pixel)
        .property("sz",  &bitmap::get_size)
        .property("size",  &bitmap::get_size)
        .def("stroke_circle", &bitmap::stroke_circle)
        .def("stroke_line", &bitmap::stroke_line)
        .def("stroke_rect", &bitmap::stroke_rectangle)
        .def("stroke_rectangle", &bitmap::stroke_rectangle)
        .scope
        [
          def("create",  &bitmap::create),
          def("create",  &bitmap::load),
          def("create",  &bitmap::load_path),
//          def("draw_text_c", &bitmap::draw_text),
//          def("levana_icon", &bitmap::levana_icon),
          def("load",    &bitmap::load),
          def("load",    &bitmap::load_path),
//          def("string_c",  &bitmap::string),
          def("get_sub_bitmap_c", &bitmap::get_sub_bitmap)
        ],
      class_<animation, drawable, boost::shared_ptr<drawable> >("animation")
        .property("current", &animation::get_current)
        .scope
        [
          def("create", &animation::create),
          def("create", &animation::create0)
        ],
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
        .def("reserve_bitmap", &layout::reserve_bitmap)
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
          def("create", &layout::create),
          def("create", &layout::create0)
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
//  register_to(classes["image"], "draw_text", &bitmap::draw_text_l);
  register_to(classes["bitmap"], "draw", &bitmap::draw_l);
  register_to(classes["bitmap"], "get_sub", &bitmap::get_sub_bitmap_l);
  register_to(classes["bitmap"], "get_sub_bitmap", &bitmap::get_sub_bitmap_l);
//  register_to(classes["bitmap"], "string", &bitmap::string_l);
  register_to(classes["bitmap"], "sub", &bitmap::get_sub_bitmap_l);
  register_to(classes["bitmap"], "sub_bitmap", &bitmap::get_sub_bitmap_l);
  register_to(classes["animation"], "append", &animation::append_l);
  register_to(classes["map"], "map_image", &map::map_image_l);
  register_to(classes["map"], "map_link", &map::map_link_l);
  register_to(classes["transition"], "set_current", &transition::set_current_l);
  register_to(classes["transition"], "set_next", &transition::set_next_l);

  lev["animation"] = classes["animation"]["create"];
  lev["bitmap"] = classes["bitmap"]["create"];
  lev["layout"] = classes["layout"]["create"];
  lev["map"] = classes["map"]["create"];
  lev["transition"] = classes["transition"]["create"];

//  image["animation"]   = classes["animation"]["create"];
//  image["create"]      = classes["bitmap"]["create"];
//  image["layout"]      = classes["layout"]["create"];
//  image["levana_icon"] = classes["bitmap"]["levana_icon"];
//  image["load"]        = classes["bitmap"]["load"];
//  image["map"]         = classes["map"]["create"];
//  image["string"]      = classes["bitmap"]["string"];
//  image["transition"]  = classes["transition"]["create"];

  globals(L)["package"]["loaded"]["lev.image"] = image;
  return 0;
}


namespace lev
{

  static ALLEGRO_BITMAP *cast_bmp(void *obj)
  {
    return (ALLEGRO_BITMAP *)obj;
  }
  static ALLEGRO_COLOR map_color(const color &c)
  {
    return al_map_rgba(c.get_r(), c.get_g(), c.get_b(), c.get_a());
  }
  static ALLEGRO_COLOR map_color(boost::shared_ptr<color> c)
  {
    if (! c) { return al_map_rgba(0, 0, 0, 0); }
    return al_map_rgba(c->get_r(), c->get_g(), c->get_b(), c->get_a());
  }

  bitmap::bitmap() : drawable(), _obj(NULL)  { }

  bitmap::~bitmap()
  {
    if (_obj && system::get_interpreter())
    {
      al_destroy_bitmap(cast_bmp(_obj));
      _obj = NULL;
    }
  }

  bool bitmap::blit(int dst_x, int dst_y, bitmap *src,
                    int src_x, int src_y, int w, int h, unsigned char alpha)
  {
    if (src == NULL) { return false; }
    if (alpha == 0) { return true; }
    if (w < 0) { w = src->get_w() - src_x; }
    if (h < 0) { h = src->get_h() - src_y; }

    boost::shared_ptr<bitmap> src_sub = src->get_sub_bitmap(src_x, src_y, w, h);
    ALLEGRO_BITMAP *src_raw = cast_bmp(src_sub->_obj);
    set_as_target();
    if (alpha == 255)
    {
      al_draw_bitmap(src_raw, dst_x, dst_y, 0);
    }
    else if (alpha > 0)
    {
      al_draw_tinted_bitmap(src_raw, al_map_rgba(255, 255, 255, alpha), dst_x, dst_y, 0);
    }
    return true;
  }

  bool bitmap::clear(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
  {
    set_as_target();
    al_clear_to_color(al_map_rgba(r, g, b, a));
    return true;
  }

  bool bitmap::clear_color(boost::shared_ptr<color> c)
  {
    set_as_target();
    al_clear_to_color(map_color(c));
    return true;
  }

  bool bitmap::clear_rect(int offset_x, int offset_y, int w, int h, boost::shared_ptr<color> c)
  {
    if (! c) { return false; }

    for (int y = 0; y < h; y++)
    {
      for (int x = 0; x < w; x++)
      {
        set_pixel(offset_x + x, offset_y + y, *c);
      }
    }
    return true;
  }

  bool bitmap::clear_rect2(const rect &r, boost::shared_ptr<color> c)
  {
    return clear_rect(r.get_x(), r.get_y(), r.get_w(), r.get_h(), c);
  }

  boost::shared_ptr<bitmap> bitmap::clone()
  {
    boost::shared_ptr<bitmap> img;
    try {
      img.reset(new bitmap);
      if (! img) { throw -1; }
      img->_obj = al_clone_bitmap(cast_bmp(_obj));
      if (! img->_obj) { throw -2; }
    }
    catch (...) {
      img.reset();
      lev::debug_print("error on cloning bitmap");
    }
    return img;
  }

  boost::shared_ptr<bitmap> bitmap::create(int width, int height)
  {
    boost::shared_ptr<bitmap> img;
    if (width <= 0 || height <= 0) { return img; }
    try {
      img.reset(new bitmap);
      if (! img) { throw -1; }
//      al_add_new_bitmap_flag(ALLEGRO_MIN_LINEAR || ALLEGRO_MIPMAP);
//      al_set_new_bitmap_flags(ALLEGRO_VIDEO_BITMAP);
      img->_obj = al_create_bitmap(width, height);
      if (! img->_obj) { throw -2; }
      img->clear();
    }
    catch (...) {
      img.reset();
      lev::debug_print("error on bitmzes ap creation");
    }
    return img;
  }

  bool bitmap::draw(drawable *src, int x, int y, unsigned char alpha)
  {
    if (! src) { return false; }
    return src->draw_on(this, x, y, alpha);
  }

  bool bitmap::draw_on(bitmap *dst, int offset_x, int offset_y, unsigned char alpha)
  {
    if (! dst) { return false; }
    return dst->blit(offset_x, offset_y, this, 0, 0, get_w(), get_h(), alpha);
  }

  bool bitmap::draw_pixel(int x, int y, const color &c)
  {
    set_as_target();
    al_draw_pixel(x, y, al_map_rgba(c.get_r(), c.get_g(), c.get_b(), c.get_a()));
    return true;
  }

  int bitmap::draw_l(lua_State *L)
  {
    using namespace luabind;

    try {
      luaL_checktype(L, 1, LUA_TUSERDATA);
      bitmap *img = object_cast<bitmap *>(object(from_stack(L, 1)));
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

      if (t["lev.drawable1"])
      {
        object obj = t["lev.drawable1"];
        boost::shared_ptr<drawable> src = object_cast<boost::shared_ptr<drawable> >(obj["as_drawable"](obj));
        img->draw(src.get(), x, y, a);
      }
      else if (t["lev.font1"])
      {
        int spacing = 1;
        boost::shared_ptr<color> c;
        font *f = object_cast<font *>(t["lev.font1"]);
        const char *str = NULL;

        if (t["spacing"]) { spacing = object_cast<int>(t["spacing"]); }
        else if (t["space"]) { spacing = object_cast<int>(t["space"]); }
        else if (t["s"]) { spacing = object_cast<int>(t["s"]); }
        else if (t["lua.number3"]) { spacing = object_cast<int>(t["lua.number3"]); }

        if (t["color"]) { c = object_cast<boost::shared_ptr<color> >(t["color"]); }
        else if (t["c"]) { c = object_cast<boost::shared_ptr<color> >(t["c"]); }
        else if (t["lev.color1"]) { c = object_cast<boost::shared_ptr<color> >(t["lev.color1"]); }

        if (t["lua.string1"]) { str = object_cast<const char *>(t["lua.string1"]); }

        if (! str) { throw -1; }

        boost::shared_ptr<bitmap> bmp = f->rasterize(str, c);
        if (! bmp) { throw -2; }
        img->draw(bmp.get(), x, y, c->get_a());
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
      lev::debug_print("error on drawing");
      lua_pushboolean(L, false);
      return 1;
    }
  }

  int bitmap::get_h() const
  {
    return al_get_bitmap_height(cast_bmp(_obj));
  }

  bool bitmap::fill_circle(int cx, int cy, int radius, boost::shared_ptr<color> filling)
  {
    if (! filling) { return false; }
    if (filling->get_a() == 0) { return true; }

    set_as_target();
    al_draw_filled_circle(cx, cy, radius, map_color(*filling));
    return true;
  }

  bool bitmap::fill_rectangle(int x1, int y1, int x2, int y2, boost::shared_ptr<color> filling)
  {
    if (! filling) { return false; }
    if (filling->get_a() == 0) { return true; }

    set_as_target();
    al_draw_filled_rectangle(x1, y1, x2, y2, map_color(*filling));
    return true;
  }

  boost::shared_ptr<color> bitmap::get_pixel(int x, int y)
  {
    boost::shared_ptr<color> c;
    try {
      unsigned char r, g, b, a;
      if (x < 0 || x >= get_w() || y < 0 || y >= get_h()) { throw -1; }
      ALLEGRO_COLOR col = al_get_pixel(cast_bmp(_obj), x, y);
      al_unmap_rgba(col, &r, &g, &b, &a);
      c = color::create(r, g, b, a);
      if (! c) { throw -2; }
    }
    catch (...) {
      c.reset();
    }
    return c;
  }

  boost::shared_ptr<rect> bitmap::get_rect() const
  {
    return rect::create(0, 0, get_w(), get_h());
  }

  boost::shared_ptr<size> bitmap::get_size() const
  {
    return size::create(get_w(), get_h());
  }

  boost::shared_ptr<bitmap> bitmap::get_sub_bitmap(int x, int y, int w, int h)
  {
    boost::shared_ptr<bitmap> sub;
    try {
      sub.reset(new bitmap);
      if (! sub) { throw -1; }
      sub->_obj = al_create_sub_bitmap(cast_bmp(_obj), x, y, w, h);
      if (! sub->_obj) { throw -2; }
    }
    catch (...) {
      sub.reset();
      lev::debug_print("error on sub bitmap instance creation");
    }
    return sub;
  }

  int bitmap::get_sub_bitmap_l(lua_State *L)
  {
    using namespace luabind;
    int x = 0, y = 0, w = -1, h = -1;

    luaL_checktype(L, 1, LUA_TUSERDATA);
    bitmap *img = object_cast<bitmap *>(object(from_stack(L, 1)));
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

    object o = globals(L)["lev"]["classes"]["bitmap"]["get_sub_bitmap_c"](img, x, y, w, h);
    o.push(L);
    return 1;
  }

  int bitmap::get_w() const
  {
    return al_get_bitmap_width(cast_bmp(_obj));
  }

//  bitmap* bitmap::levana_icon()
//  {
//    static bitmap *img = NULL;
//    wxBitmap *obj = NULL;
//
//    if (img) { return img; }
//    try {
//      img = new bitmap;
//      img->_obj = obj = new wxBitmap(levana_xpm);
//      img->_status = new myImageStatus;
//      return img;
//    }
//    catch (...) {
//      delete img;
//      return NULL;
//    }
//  }

  boost::shared_ptr<bitmap> bitmap::load(const std::string &filename)
  {
    boost::shared_ptr<bitmap> img;
    try {
      img.reset(new bitmap);
      if (! img) { throw -1; }
      img->_obj = al_load_bitmap(filename.c_str());
      if (! img->_obj) { throw -2; }
    }
    catch (...) {
      img.reset();
      lev::debug_print("error on bitmap data loading");
    }
    return img;
  }

  boost::shared_ptr<bitmap> bitmap::load_path(boost::shared_ptr<path> p)
  {
    return load(p->get_full_path());
  }

  bool bitmap::reload(const std::string &filename)
  {
    boost::shared_ptr<bitmap> img = bitmap::load(filename);
    if (! img) { return false; }
    this->swap(img);
    return true;
  }

  boost::shared_ptr<bitmap> bitmap::resize(int width, int height)
  {
    boost::shared_ptr<bitmap> img;
    try {
      img = bitmap::create(width, height);
      if (! img) { throw -1; }
      img->set_as_target();
      al_draw_scaled_bitmap(cast_bmp(_obj),
                            0, 0, get_w(), get_h(),
                            0, 0, width,   height,
                            0);
    }
    catch (...) {
      img.reset();
      lev::debug_print("error on resized bitmap creation");
    }
    return img;
  }

  bool bitmap::save(const std::string &filename) const
  {
    return al_save_bitmap(filename.c_str(), cast_bmp(_obj));
  }

  bool bitmap::set_as_target()
  {
    al_set_target_bitmap(cast_bmp(_obj));
    return true;
  }

  bool bitmap::set_pixel(int x, int y, const color &c)
  {
    set_as_target();
    al_put_pixel(x, y, al_map_rgba(c.get_r(), c.get_g(), c.get_b(), c.get_a()));
    return true;
  }


//  boost::shared_ptr<bitmap> bitmap::string(boost::shared_ptr<font> f, const std::string &str,
//                                         boost::shared_ptr<color> fore,
//                                         boost::shared_ptr<color> shade,
//                                         boost::shared_ptr<color> back,
//                                         int spacing)
//  {
//    boost::shared_ptr<bitmap> img;
//    if (! f) { return img; }
//
//    if (! fore) { fore = color::white(); }
//    if (! back) { back = color::transparent(); }
//    if (! fore || ! back) { return img; }
//
//    try {
//      boost::shared_ptr<raster> r = f->rasterize_utf8(str, spacing);
//      if (! r) { throw -1; }
//
//      if (shade)
//      {
//        img = bitmap::create(r->get_w() + 1, r->get_h() + 1);
//        if (! img) { throw -2; }
//        img->clear1(back);
//        img->draw_raster(r.get(), 1, 1, shade);
//        img->draw_raster(r.get(), 0, 0, fore);
//      }
//      else
//      {
//        img = bitmap::create(r->get_w(), r->get_h());
//        if (! img) { throw -2; }
//        img->clear1(back);
//        img->draw_raster(r.get(), 0, 0, fore);
//      }
//    }
//    catch (...) {
//      img.reset();
//      lev::debug_print("error on string bitmap instance creation");
//    }
//    return img;
//  }

//  int bitmap::string_l(lua_State *L)
//  {
//    using namespace luabind;
//
//    try {
//      const char *str = NULL;
//      boost::shared_ptr<font> f;
//      boost::shared_ptr<color> fore = color::white();
//      boost::shared_ptr<color> back = color::transparent();
//      boost::shared_ptr<color> shade = color::black();
//      int spacing = 1;
//
//      object t = util::get_merged(L, 1, -1);
//      if (t["lua.string1"]) { str = object_cast<const char *>(t["lua.string1"]); }
//      else if (t["lev.unicode1"]) { str = object_cast<const char *>(t["lev.unicode1"]["str"]); }
//      else if (t["text"]) { str = object_cast<const char *>(t["text"]); }
//      else if (t["t"]) { str = object_cast<const char *>(t["t"]); }
//      else if (t["string"]) { str = object_cast<const char *>(t["string"]); }
//      else if (t["str"]) { str = object_cast<const char *>(t["str"]); }
//      if (! str)
//      {
//        luaL_error(L, "text (string) is not specified");
//        return 0;
//      }
//
//      if (t["lev.font1"]) { f = object_cast<boost::shared_ptr<font> >(t["lev.font1"]); }
//      else if (t["font"]) { f = object_cast<boost::shared_ptr<font> >(t["font"]); }
//      else if (t["f"]) { f = object_cast<boost::shared_ptr<font> >(t["f"]); }
//      else
//      {
//        f = font::load();
//      }
//      if (! f)
//      {
//        luaL_error(L, "no fonts are found\n");
//        return 0;
//      }
//
//      if (t["lev.prim.color1"]) { fore = object_cast<boost::shared_ptr<color> >(t["lev.prim.color1"]); }
//      else if (t["fg_color"]) { fore = object_cast<boost::shared_ptr<color> >(t["fg_color"]); }
//      else if (t["fg"]) { fore = object_cast<boost::shared_ptr<color> >(t["fg"]); }
//      else if (t["fore"]) { fore = object_cast<boost::shared_ptr<color> >(t["fore"]); }
//      else if (t["f"]) { fore = object_cast<boost::shared_ptr<color> >(t["f"]); }
//      else if (t["color"]) { fore = object_cast<boost::shared_ptr<color> >(t["color"]); }
//      else if (t["c"]) { fore = object_cast<boost::shared_ptr<color> >(t["c"]); }
//
//      if (t["lev.prim.color2"]) { shade = object_cast<boost::shared_ptr<color> >(t["lev.prim.color2"]); }
//      else if (t["shade_color"]) { shade = object_cast<boost::shared_ptr<color> >(t["shade_color"]); }
//      else if (t["shade"]) { shade = object_cast<boost::shared_ptr<color> >(t["shade"]); }
//      else if (t["sh"]) { shade = object_cast<boost::shared_ptr<color> >(t["sh"]); }
//      else if (t["s"]) { shade = object_cast<boost::shared_ptr<color> >(t["s"]); }
//
//      if (t["lev.prim.color3"]) { back = object_cast<boost::shared_ptr<color> >(t["lev.prim.color3"]); }
//      else if (t["bg_color"]) { back = object_cast<boost::shared_ptr<color> >(t["bg_color"]); }
//      else if (t["bg"]) { back = object_cast<boost::shared_ptr<color> >(t["bg"]); }
//      else if (t["back"]) { back = object_cast<boost::shared_ptr<color> >(t["back"]); }
//      else if (t["b"]) { back = object_cast<boost::shared_ptr<color> >(t["b"]); }
//
//      if (t["lua.number1"]) { spacing = object_cast<int>(t["lua.number1"]); }
//      else if (t["spacing"]) { spacing = object_cast<int>(t["spacing"]); }
//      else if (t["space"]) { spacing = object_cast<int>(t["space"]); }
//
//      object o = globals(L)["lev"]["classes"]["bitmap"]["string_c"](f, str, fore, shade, back, spacing);
//      o.push(L);
//      return 1;
//    }
//    catch (...) {
//      lev::debug_print("error on string bitmap creating lua code");
//      lua_pushnil(L);
//      return 1;
//    }
//  }

  bool bitmap::stroke_circle(int cx, int cy, int radius, boost::shared_ptr<color> border, int width)
  {
    if (! border) { return false; }
    if (border->get_a() == 0) { return true; }

    set_as_target();
    al_draw_circle(cx, cy, radius, map_color(*border), width);
    return true;
  }

  bool bitmap::stroke_line(int x1, int y1, int x2, int y2, boost::shared_ptr<color> c, int width)
  {
    if (! c) { return false; }
    if (c->get_a() == 0) { return true; }

    set_as_target();
    al_draw_line(x1, y1, x2, y2, map_color(*c), width);
    return true;
  }

  bool bitmap::stroke_rectangle(int x1, int y1, int x2, int y2, boost::shared_ptr<color> border, int width)
  {
    if (! border) { return false; }
    if (border->get_a() == 0) { return true; }

    set_as_target();
    al_draw_rectangle(x1, y1, x2, y2, map_color(*border), width);
    return true;
  }

  bool bitmap::swap(boost::shared_ptr<bitmap> img)
  {
    if (! img) { return false; }
    // swap the images
    void *tmp  = this->_obj;
    this->_obj = img->_obj;
    img->_obj  = tmp;
    return true;
  }

  class myAnimation
  {
    protected:

      myAnimation(bool repeating = true)
        : imgs(), repeating(repeating) { }

    public:

      ~myAnimation() { }

      static myAnimation* Create(bool repeating = true)
      {
        myAnimation *anim = NULL;
        try {
          anim = new myAnimation(repeating);
          if (! anim) { throw -1; }
          anim->sw = stop_watch::create();
          if (! anim->sw) { throw -2; }
          return anim;
        }
        catch (...) {
          delete anim;
          return NULL;
        }
      }

      bool Append(boost::shared_ptr<drawable> img, double duration)
      {
        if (! img) { return false; }
        if (duration <= 0) { return false; }
        try {
          imgs.push_back(img);
          durations.push_back(duration);
          return true;
        }
        catch (...) {
          return false;
        }
      }

      boost::shared_ptr<drawable> GetCurrent()
      {
        double now = sw->get_time();
        double total = 0;
        if (imgs.size() == 0) { return boost::shared_ptr<drawable>(); }

        for (int i = 0; i < durations.size(); i++)
        {
          if (total <= now && now < total + durations[i])
          {
            return imgs[i];
          }
          total += durations[i];
        }
        if (! repeating) { return imgs[imgs.size() - 1]; }
        sw->start(sw->get_time() - total);
        return GetCurrent();
      }

      bool repeating;
      boost::shared_ptr<stop_watch> sw;
      std::vector<boost::shared_ptr<drawable> > imgs;
      std::vector<double> durations;
  };
  static myAnimation* cast_anim(void *obj) { return (myAnimation *)obj; }

  animation::animation()
    : drawable(), _obj(NULL)
  { }

  animation::~animation()
  {
    if (_obj) { delete cast_anim(_obj); }
  }

  bool animation::append(boost::shared_ptr<drawable> img, double duration)
  {
    return cast_anim(_obj)->Append(img, duration);
  }

  bool animation::append_file(const std::string &filename, double duration)
  {
    return cast_anim(_obj)->Append(bitmap::load(filename), duration);
  }

  bool animation::append_path(boost::shared_ptr<path> p, double duration)
  {
    return animation::append_file(p->to_str(), duration);
  }

  int animation::append_l(lua_State *L)
  {
    using namespace luabind;

    try {
      int x = 0, y = 0;
      double duration = 1;

      luaL_checktype(L, 1, LUA_TUSERDATA);
      animation* anim = object_cast<animation *>(object(from_stack(L, 1)));
      object t = util::get_merged(L, 2, -1);

      if (t["duration"]) { duration = object_cast<double>(t["duration"]); }
      else if (t["d"]) { duration = object_cast<double>(t["d"]); }
      else if (t["interval"]) { duration = object_cast<double>(t["interval"]); }
      else if (t["i"]) { duration = object_cast<double>(t["i"]); }
      else if (t["lua.number1"]) { duration = object_cast<double>(t["lua.number1"]); }

      if (t["lev.drawable1"])
      {
        object obj = t["lev.drawable1"];
        boost::shared_ptr<drawable> img;
        img = object_cast<boost::shared_ptr<drawable> >(obj["as_drawable"](obj));
        lua_pushboolean(L, anim->append(img, duration));
      }
      else if (t["lua.string1"])
      {
        const char *path = object_cast<const char *>(t["lua.string1"]);
        lua_pushboolean(L, anim->append_file(path, duration));
      }
      else if (t["lua.path1"])
      {
        boost::shared_ptr<path> p = object_cast<boost::shared_ptr<path> >(t["lua.path1"]);
        lua_pushboolean(L, anim->append_path(p, duration));
      }
      else
      {
        lua_pushboolean(L, false);
      }
    }
    catch (...) {
      lev::debug_print(lua_tostring(L, -1));
      lev::debug_print("error on animation item appending");
      lua_pushboolean(L, false);
    }
    return 1;
  }

  boost::shared_ptr<animation> animation::create(bool repeating)
  {
    boost::shared_ptr<animation> anim;
    try {
      anim.reset(new animation);
      if (! anim) { throw -1; }
      anim->_obj = myAnimation::Create(repeating);
      if (! anim->_obj) { throw -2; }
    }
    catch (...) {
      anim.reset();
      lev::debug_print("error on animation instance creation\n");
    }
    return anim;
  }

  bool animation::draw_on(bitmap *dst, int x, int y, unsigned char alpha)
  {
    boost::shared_ptr<drawable> img = get_current();
    if (! img) { return false; }
    return img->draw_on(dst, x, y, alpha);
  }

  boost::shared_ptr<drawable> animation::get_current()
  {
    return cast_anim(_obj)->GetCurrent();
  }

  int animation::get_h() const
  {
    boost::shared_ptr<drawable> img = cast_anim(_obj)->GetCurrent();
    if (img) { return img->get_h(); }
    else { return 0; }
  }

  int animation::get_w() const
  {
    boost::shared_ptr<drawable> img = cast_anim(_obj)->GetCurrent();
    if (img) { return img->get_w(); }
    else { return 0; }
  }


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

      myTransition() : imgs(), sw() { }

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

      bool DrawOn(bitmap *dst, int x, int y, unsigned char alpha)
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
          if (modes.size() >= 1)
          {
            if (modes[0] == LEV_TRAN_FADE_OUT)
            {
              imgs[0]->draw_on(dst, x, y, (1 - grad) * alpha);
            }
            else if (modes[0] == LEV_TRAN_CROSS_FADE)
            {
              imgs[0]->draw_on(dst, x, y, (1 - grad) * alpha);
            }
          }
          else { imgs[0]->draw_on(dst, x, y, alpha); }
        }

        if (imgs.size() == 1) { return true; }
        if (imgs[1])
        {
          if (modes[0] == LEV_TRAN_CROSS_FADE)
          {
//printf("IMGS[1]: %p, %d\n", imgs[1].get(), (int)imgs.size());
//printf("1 FADE: alpha: %d, glad: %lf\n", (int)alpha, grad);
            imgs[1]->draw_on(dst, x, y, alpha * grad);
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
        return true;
      }

      bool SetNext(boost::shared_ptr<drawable> img, int duration, const std::string &type)
      {
        if (duration < 0) { return false; }

        try {
          imgs.push_back(img);
          durations.push_back(duration);

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

      std::vector<boost::shared_ptr<drawable> > imgs;
      std::vector<double> durations;
      std::vector<transition_mode> modes;
      boost::shared_ptr<stop_watch> sw;
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
    transition::create_with_path(boost::shared_ptr<path> p)
  {
    return transition::create_with_string(p->get_full_path());
  }

  boost::shared_ptr<transition> transition::create_with_string(const std::string &bitmap_path)
  {
    return transition::create(bitmap::load(bitmap_path));
  }

  bool transition::draw_on(bitmap *dst, int x, int y, unsigned char alpha)
  {
    return cast_tran(_obj)->DrawOn(dst, x, y, alpha);
  }

  boost::shared_ptr<drawable> transition::get_current()
  {
    return cast_tran(_obj)->imgs[0];
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

  bool transition::set_current(const std::string &bitmap_path)
  {
    return cast_tran(_obj)->SetCurrent(bitmap::load(bitmap_path));
  }

  int transition::set_current_l(lua_State *L)
  {
    using namespace luabind;

    try {
      bool result = false;

      luaL_checktype(L, 1, LUA_TUSERDATA);
      transition *tran = object_cast<transition *>(object(from_stack(L, 1)));
      object t = util::get_merged(L, 2, -1);

      if (t["lev.drawable1"])
      {
        object obj = t["lev.drawable1"];
        boost::shared_ptr<drawable> img = object_cast<boost::shared_ptr<drawable> >(obj["as_drawable"](obj));
        result = tran->set_current(img);
      }
      else if (t["lev.path1"])
      {
        path* p = object_cast<path *>(t["lev.path1"]);
        result = tran->set_current(p->to_str());
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
      lev::debug_print("error on transition current bitmap setting");
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
    return cast_tran(_obj)->SetNext(bitmap::load(image_path), duration, mode);
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

      if (t["lev.drawable1"])
      {
        object obj = t["lev.drawable1"];
        boost::shared_ptr<drawable> img = object_cast<boost::shared_ptr<drawable> >(obj["as_drawable"](obj));
        result = tran->set_next(img, duration, mode);
      }
      else if (t["lev.path1"])
      {
        boost::shared_ptr<path> p =
          object_cast<boost::shared_ptr<path> >(t["lev.path1"]);
        result = tran->set_next(p->to_str(), duration, mode);
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
      lev::debug_print("error on transition next bitmap setting");
      lua_pushnil(L);
    }
    return 1;
  }


  class myLayout
  {
    protected:

      struct myItem
      {
        public:
          myItem() : x(-1), y(-1), fixed(false),
            img(), img_hover(), img_showing(),
            func_hover(), func_lclick() { }
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
          items()
      {
        font_text = font::load0();
        font_ruby = font::load0();
//        if (font_ruby) { font_ruby->set_size(font_ruby->get_size() / 2); }
        if (font_ruby) { font_ruby->set_size(font_ruby->get_size() / 2 + 5); }
        color_fg = color::white();
        color_shade = color::black();
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

      bool DrawOn(bitmap *dst, int x, int y, unsigned char alpha)
      {
//boost::shared_ptr<stop_watch> sw = stop_watch::create();
        for (int i = 0; i < items.size(); i++)
        {
//printf("DRAW ELEMENT %d: %d %lf\n", i, (int)alpha, sw->get_time());
          myItem &item = items[i];
          if (item.img_showing)
          {
            CalcPosition(i);
            item.img_showing->draw_on(dst, x + item.x, y + item.y, alpha);
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

//        for (int i = 0; i < items.size(); i++)
        for (int i = items.size() - 1; i >= 0; i--)
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

      bool ReserveClickable(boost::shared_ptr<drawable> normal,
                            boost::shared_ptr<drawable> hover,
                            luabind::object lclick_func,
                            luabind::object hover_func)
      {
        try {
          if (! normal) { throw -1; }
          if (! hover) { hover = normal; }

          items.push_back(myItem());
          myItem &i = items[items.size() - 1];
          i.img = normal;
          i.img_hover = hover;
          i.func_hover = hover_func;
          i.func_lclick = lclick_func;
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
          boost::shared_ptr<word> img;
          boost::shared_ptr<word> hover_img;

          img = word::create(font_text, color_fg, color::empty(), color_shade, text, " ");
          img->set_under_line();
          hover_img = word::create(font_text, hover_fg, hover_bg, color::empty(), text, " ");
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
          return true;
        }
        catch (...) {
          return false;
        }
      }

      bool ReserveNewLine()
      {
        // height spacing by bitmap
        items.push_back(myItem());
//        (items.end() - 1)->img = bitmap::create(1, font_text->get_pixel_size() * 1.25);
        (items.end() - 1)->img = spacer::create(0, font_text->get_height());
        // adding new line
        items.push_back(myItem());
        return true;
      }

      bool ReserveWord(const std::string &str_word, const std::string &str_ruby = "")
      {
        if (! font_text) { return false; }
        if (str_word.empty()) { return false; }
        try {
          boost::shared_ptr<drawable> w;
          if (str_ruby.empty())
          {
            w = word::create(font_text, color_fg, color::empty(), color_shade, str_word, " ");
          }
          else
          {
            w = word::create(font_text, color_fg, color::empty(), color_shade, str_word, str_ruby);
          }
//printf("WORD: %p\n", w.get());
//printf("W: %d, H: %d\n", w->get_w(), w->get_h());
          return ReserveImage(w);
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

      // format properties
      boost::shared_ptr<color> color_fg;
      boost::shared_ptr<color> color_shade;
      boost::shared_ptr<color> hover_bg;
      boost::shared_ptr<color> hover_fg;
      boost::shared_ptr<font> font_text;
      boost::shared_ptr<font> font_ruby;
      int spacing;
      int width_stop;

      // all items
      std::vector<myItem> items;
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
      lev::debug_print("error on bitmap layout instance creation");
    }
    return lay;
  }

  bool layout::draw_on(bitmap *dst, int x, int y, unsigned char alpha)
  {
    return cast_lay(_obj)->DrawOn(dst, x, y, alpha);
  }

  boost::shared_ptr<color> layout::get_fg_color()
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

  boost::shared_ptr<font> layout::get_ruby_font()
  {
    return cast_lay(_obj)->font_ruby;
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

  bool layout::reserve_clickable(boost::shared_ptr<bitmap> normal,
                                 boost::shared_ptr<bitmap> hover,
                                 luabind::object lclick_func,
                                 luabind::object hover_func)
  {
    return cast_lay(_obj)->ReserveClickable(boost::shared_ptr<bitmap>(normal),
                                            boost::shared_ptr<bitmap>(hover),
                                            lclick_func,
                                            hover_func);
  }

  bool layout::reserve_clickable_text(const std::string &text,
                                      luabind::object lclick_func,
                                      luabind::object hover_func)
  {
    return cast_lay(_obj)->ReserveClickableText(text, lclick_func, hover_func);
  }

  bool layout::reserve_bitmap(boost::shared_ptr<bitmap> img)
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

//  bool layout::set_fg_color(const color &fg)
  bool layout::set_fg_color(boost::shared_ptr<color> fg)
  {
    if (! fg) { return false; }
    cast_lay(_obj)->color_fg = fg;
    return true;
  }

  bool layout::set_font(boost::shared_ptr<font> f)
  {
    if (! f) { return false; }
    cast_lay(_obj)->font_text = f;
    return true;
  }

  bool layout::set_ruby_font(boost::shared_ptr<font> f)
  {
    if (! f) { return false; }
    cast_lay(_obj)->font_ruby = f;
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

}

