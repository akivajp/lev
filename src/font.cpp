/////////////////////////////////////////////////////////////////////////////
// Name:        src/font.cpp
// Purpose:     source for font management classes
// Author:      Akiva Miura <akiva.miura@gmail.com>
// Created:     16/08/2010
// Copyright:   (C) 2010-2011 Akiva Miura
// Licence:     MIT License
/////////////////////////////////////////////////////////////////////////////

// pre-compiled header
#include "prec.h"

// delcarations
#include "lev/font.hpp"

// dependencies
#include "lev/debug.hpp"
#include "lev/entry.hpp"
#include "lev/image.hpp"
#include "lev/package.hpp"
#include "lev/prim.hpp"
#include "lev/system.hpp"
#include "lev/util.hpp"

// libraries
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_ttf.h>

int luaopen_lev_font(lua_State *L)
{
  using namespace lev;
  using namespace luabind;

  open(L);
  globals(L)["package"]["loaded"]["lev.font"] = true;
  globals(L)["require"]("lev.base");
  globals(L)["require"]("lev.string");

  module_(L, "lev")
  [
    namespace_("font"),
    namespace_("classes")
    [
//      class_<raster, base>("raster")
//        .def("get_pixel", &raster::get_pixel)
//        .property("h", &raster::get_h)
//        .property("height", &raster::get_h)
//        .def("set_pixel", &raster::set_pixel)
//        .property("w", &raster::get_w)
//        .property("width", &raster::get_w),
      class_<font, base>("font")
        .property("ascent", &font::get_ascent)
        .def("calc_width", &font::calc_width)
        .property("descent", &font::get_descent)
        .property("path", &font::get_path)
        .def("clone", &font::clone)
        .property("h", &font::get_height)
        .property("height", &font::get_height)
        .property("pixel_size", &font::get_size, &font::set_size)
        .property("px_size", &font::get_size, &font::set_size)
//        .def("rasterize", &font::rasterize1)
//        .def("rasterize", &font::rasterize_utf8)
//        .def("rasterize", &font::rasterize_utf8_1)
//        .def("rasterize", &font::rasterize_utf16)
//        .def("rasterize", &font::rasterize_utf16_1)
        .property("size", &font::get_size, &font::set_size)
        .property("sz", &font::get_size, &font::set_size)
        .scope
        [
          def("clone", &font::clone),
          def("load", &font::load),
          def("load", &font::load0),
          def("load", &font::load1),
          def("rasterize_c", &font::rasterize)
        ]
    ]
  ];
  object lev = globals(L)["lev"];
  object classes = lev["classes"];
  lev["font"] = classes["font"]["load"];
//  object font = lev["font"];

  register_to(classes["font"], "rasterize", &font::rasterize_l);

//  font["clone"]  = classes["font"]["clone"];
//  font["load"]   = classes["font"]["load"];

  globals(L)["package"]["loaded"]["lev.font"] = true;
  return 0;
}

namespace lev
{

  word::word() :
    drawable(), w(0), h(0),
    ft_text(), ft_ruby(),
    underlined(false)
  { }

  word::~word() { }

  boost::shared_ptr<word>
    word::create(boost::shared_ptr<font> f,
                 boost::shared_ptr<color> arg_fg,
                 boost::shared_ptr<color> arg_bg,
                 boost::shared_ptr<color> arg_shade,
                 const std::string &arg_text,
                 const std::string &arg_ruby)
  {
    boost::shared_ptr<word> w;
    if (! f) { return w; }
    if (! arg_fg) { return w; }
    try {
      w.reset(new word);
      if (! w) { throw -1; }
      w->ft_text = f->clone();
      if (! w->ft_text) { throw -2; }
      w->fg = arg_fg;
      w->bg = arg_bg;
      w->shade = arg_shade;
      w->h = f->get_height();
      w->w = f->calc_width(arg_text);
      w->text = arg_text;
      w->ruby = arg_ruby;
      if (arg_shade)
      {
        w->h += 1;
        w->w += 1;
      }
      if (! arg_ruby.empty())
      {
        w->ft_ruby = f->clone();
        if (! w->ft_ruby) { throw -1; }
        w->ft_ruby->set_size(f->get_size() / 2);
        w->h += w->ft_ruby->get_height();
      }
      return w;
    }
    catch (...) {
      w.reset();
      lev::debug_print("error on word instance creation");
    }
    return w;
  }

  bool word::draw_on(bitmap *dst, int x, int y, unsigned char alpha)
  {
    if (! dst) { return false; }
    dst->set_as_target();
    ALLEGRO_COLOR col_fg = al_map_rgba(fg->get_r(), fg->get_g(), fg->get_b(), fg->get_a());
    ALLEGRO_FONT *raw_font = (ALLEGRO_FONT *)ft_text->get_rawobj();
    if (bg)
    {
      ALLEGRO_COLOR col_bg = al_map_rgba(bg->get_r(), bg->get_g(), bg->get_b(), bg->get_a());
      al_draw_filled_rectangle(x, y, x + w, y + h, col_bg);
    }
    if (ft_ruby)
    {
      // draw with ruby
      ALLEGRO_FONT *raw_ruby = (ALLEGRO_FONT *)ft_ruby->get_rawobj();
      if (shade)
      {
        ALLEGRO_COLOR col_sh = al_map_rgba(shade->get_r(), shade->get_g(), shade->get_b(), shade->get_a());
        al_draw_text(raw_ruby, col_sh, x + (w / 2) + 1, y + 1, ALLEGRO_ALIGN_CENTRE, ruby.c_str());
        al_draw_text(raw_font, col_sh, x + (w / 2) + 1, y + ft_ruby->get_height() + 1,
                     ALLEGRO_ALIGN_CENTRE, text.c_str());
      }
      al_draw_text(raw_ruby, col_fg, x + (w / 2), y, ALLEGRO_ALIGN_CENTRE, ruby.c_str());
      al_draw_text(raw_font, col_fg, x + (w / 2), y + ft_ruby->get_height(),
                   ALLEGRO_ALIGN_CENTRE, text.c_str());
    }
    else
    {
      // draw without ruby
      if (shade)
      {
        ALLEGRO_COLOR col_sh = al_map_rgba(shade->get_r(), shade->get_g(), shade->get_b(), shade->get_a());
        al_draw_text(raw_font, col_sh, x + (w / 2) + 1, y + 1, ALLEGRO_ALIGN_CENTRE, text.c_str());
      }
      al_draw_text(raw_font, col_fg, x + (w / 2), y, ALLEGRO_ALIGN_CENTRE, text.c_str());
    }
    if (underlined)
    {
      al_draw_line(x, y + h, x + w - 1, y + h, col_fg, 0);
    }
    return true;
  }

  class myFont
  {
    protected:
      myFont(int size) : file(), f(NULL), size(size) { }

    public:
      typedef boost::shared_ptr<myFont> Pointer;

      ~myFont()
      {
        Clear();
      }

      bool Clear()
      {
        if (f && system::get_interpreter())
        {
//printf("MY FONT CLEAR: %s %d\n", file.c_str(), size);
          al_destroy_font(f);
          f = NULL;
        }
      }

//      static myFont* Clone(const myFont *orig)
//      {
//        myFont *f = NULL;
//        if (! orig) { return NULL; }
//        try {
//          f = Load(orig->file, orig->size);
//          return f;
//        }
//        catch (...) {
//          delete f;
//          return NULL;
//        }
//      }

      static Pointer Load(const std::string &file, int size)
      {
        Pointer f;
//printf("MY FONT LOAD: %s %d\n", file.c_str(), size);
        try {
          f.reset(new myFont(size));
          if (! f) { throw -1; }
          f->f = al_load_ttf_font(file.c_str(), f->size, 0);
//          f->f = al_load_ttf_font(file.c_str(), f->size, ALLEGRO_TTF_MONOCHROME);
          if (! f->f) { throw -2; }
          f->file = file;
        }
        catch (...) {
          f.reset();
        }
        return f;
      }

//      bool SetSize(int sz)
//      {
//        if (sz <= 0) { return false; }
//        ALLEGRO_FONT *new_font = al_load_ttf_font(file.c_str(), sz, 0);
//        if (! new_font) { return false; }
//        Clear();
//        f = new_font;
//        size = sz;
//        return true;
//      }

    ALLEGRO_FONT *f;
    std::string file;
    int size;
  };

//  static myFont* cast_font(void *obj) { return (myFont *)obj; }
  static myFont* cast_font(boost::shared_ptr<void> obj)
  {
    return (myFont *)obj.get();
  }

  class myFontCache
  {
    public:
      typedef boost::shared_ptr<myFontCache> CachePointer;
      typedef std::map<int, myFont::Pointer> MapBySize;
      typedef std::map<std::string, MapBySize> MapByName;
      static CachePointer singleton;

    protected:
      myFontCache() : cache(), num_fonts(0) { }

    public:
      ~myFontCache() { }

      static bool Destroy()
      {
        if (singleton)
        {
//printf("DESTROY FONT CACHE\n");
          singleton.reset();
          return true;
        }
        return false;
      }

      static CachePointer Get()
      {
        return singleton;
      }

      static CachePointer Init()
      {
        if (singleton) { return singleton; }
//printf("INIT FONT CACHE\n");
        try {
          singleton.reset(new myFontCache);
          if (! singleton) { throw -1; }
        }
        catch (...) {
          singleton.reset();
        }
        return singleton;
      }

      myFont::Pointer Load(const std::string &file, int size)
      {
//        printf("FONT CACHE SIZE: %d\n", num_fonts);
        if (cache[file][size])
        {
//          printf("CACHE HIT! %s %d\n", file.c_str(), size);
          return cache[file][size];
        }
        myFont::Pointer ptr(myFont::Load(file, size));
        if (ptr)
        {
//          printf("CACHE REGISTER! %s %d\n", file.c_str(), size);
          cache[file][size] = ptr;
          num_fonts++;
        }
        return ptr;
      }

      MapByName cache;
      int num_fonts;
  };
  myFontCache::CachePointer myFontCache::singleton;

  font::font() : base(), _obj() { }

  font::~font()
  {
//    if (_obj) { delete cast_font(_obj); }
//    _obj = NULL;
  }

  int font::calc_width(const std::string &str) const
  {
    if (! _obj) { return 0; }
    return al_get_text_width(cast_font(_obj)->f, str.c_str());
  }

  boost::shared_ptr<font> font::clone()
  {
    boost::shared_ptr<font> f;
    try {
      f.reset(new font);
      if (! f) { throw -1; }
      f->_obj = _obj;
//      f->_obj = myFont::Clone(cast_font(_obj));
      if (! f->_obj) { throw -2; }
    }
    catch (...) {
      f.reset();
      lev::debug_print("error on font instance cloning");
    }
    return f;
  }

  bool font::destroy_cache()
  {
    return myFontCache::Destroy();
  }

  int font::get_ascent() const
  {
    return al_get_font_ascent(cast_font(_obj)->f);
  }

  int font::get_height() const
  {
    return al_get_font_line_height(cast_font(_obj)->f);
  }

  int font::get_descent() const
  {
    return al_get_font_descent(cast_font(_obj)->f);
  }

  std::string font::get_path() const
  {
    return cast_font(_obj)->file;
  }

  void* font::get_rawobj()
  {
    return cast_font(_obj)->f;
  }

  int font::get_size()
  {
    return cast_font(_obj)->size;
  }

  boost::shared_ptr<font> font::load(const std::string &file, int size)
  {
    boost::shared_ptr<font> f;
    try {
      if (! myFontCache::Init()) { return f; }
      f.reset(new font);
      if (! f) { throw -1; }
      f->_obj = myFontCache::Get()->Load(file, size);
//      f->_obj = myFont::Load(file, size);
      if (! f->_obj) { throw -2; }
    }
    catch (...) {
      f.reset();
      lev::debug_print("error on font instance loading");
    }
    return f;
  }

  boost::shared_ptr<font> font::load0()
  {
    boost::shared_ptr<font> f;
    if (system::get_interpreter())
    {
      f = package::find_font0(system::get_interpreter());
      if (f) { return f; }
    }
    f = font::load("default.ttf");
    return f;
  }

  boost::shared_ptr<bitmap>
    font::rasterize(const std::string &str, boost::shared_ptr<color> fg,
                    boost::shared_ptr<color> bg, boost::shared_ptr<color> shade)
  {
    boost::shared_ptr<bitmap> bmp;
    if (! _obj) { return bmp; }
    int w = calc_width(str);

    if (! fg) { return bmp; }
    ALLEGRO_COLOR col_fg = al_map_rgba(fg->get_r(), fg->get_g(), fg->get_b(), fg->get_a());

//    if (shade && shade->get_a() > 0) { bmp = bitmap::create(w + 2, get_height() + 2); }
    if (shade && shade->get_a() > 0) { bmp = bitmap::create(w + 1, get_height() + 1); }
    else { bmp = bitmap::create(w, get_height()); }
    if (! bmp) { return bmp; }

    bmp->set_as_target();
    if (bg) { bmp->clear_color(bg); }
    else { bmp->clear(); }

    if (shade && shade->get_a() > 0)
    {
      ALLEGRO_COLOR col_shade = al_map_rgba(shade->get_r(), shade->get_g(), shade->get_b(), shade->get_a());
//      al_draw_text(cast_font(_obj)->f, col_shade, 2, 2, ALLEGRO_ALIGN_LEFT, str.c_str());
      al_draw_text(cast_font(_obj)->f, col_shade, 1, 1, ALLEGRO_ALIGN_LEFT, str.c_str());
    }
//    al_draw_text(cast_font(_obj)->f, col_fg, 0, 0, ALLEGRO_ALIGN_LEFT, str.c_str());
    al_draw_text(cast_font(_obj)->f, col_fg, 0, 0, ALLEGRO_ALIGN_LEFT, str.c_str());
    return bmp;
  }

  int font::rasterize_l(lua_State *L)
  {
    using namespace luabind;
    try {
      const char *str = NULL;
      boost::shared_ptr<font> f;
      boost::shared_ptr<color> fore = color::white();
      boost::shared_ptr<color> back = color::transparent();
      boost::shared_ptr<color> shade = color::black();
      int spacing = 1;

      luaL_checktype(L, 1, LUA_TUSERDATA);
      f = object_cast<boost::shared_ptr<font> >(object(from_stack(L, 1)));
      object t = util::get_merged(L, 2, -1);
      if (t["lua.string1"]) { str = object_cast<const char *>(t["lua.string1"]); }
      else if (t["lev.unicode1"]) { str = object_cast<const char *>(t["lev.unicode1"]["str"]); }
      else if (t["text"]) { str = object_cast<const char *>(t["text"]); }
      else if (t["t"]) { str = object_cast<const char *>(t["t"]); }
      else if (t["string"]) { str = object_cast<const char *>(t["string"]); }
      else if (t["str"]) { str = object_cast<const char *>(t["str"]); }
      if (! str)
      {
        luaL_error(L, "text (string) is not specified");
        return 0;
      }

      if (t["lev.prim.color1"]) { fore = object_cast<boost::shared_ptr<color> >(t["lev.prim.color1"]); }
      else if (t["fg_color"]) { fore = object_cast<boost::shared_ptr<color> >(t["fg_color"]); }
      else if (t["fg"]) { fore = object_cast<boost::shared_ptr<color> >(t["fg"]); }
      else if (t["fore"]) { fore = object_cast<boost::shared_ptr<color> >(t["fore"]); }
      else if (t["f"]) { fore = object_cast<boost::shared_ptr<color> >(t["f"]); }
      else if (t["color"]) { fore = object_cast<boost::shared_ptr<color> >(t["color"]); }
      else if (t["c"]) { fore = object_cast<boost::shared_ptr<color> >(t["c"]); }

      if (t["lev.prim.color2"]) { shade = object_cast<boost::shared_ptr<color> >(t["lev.prim.color2"]); }
      else if (t["shade_color"]) { shade = object_cast<boost::shared_ptr<color> >(t["shade_color"]); }
      else if (t["shade"]) { shade = object_cast<boost::shared_ptr<color> >(t["shade"]); }
      else if (t["sh"]) { shade = object_cast<boost::shared_ptr<color> >(t["sh"]); }
      else if (t["s"]) { shade = object_cast<boost::shared_ptr<color> >(t["s"]); }

      if (t["lev.prim.color3"]) { back = object_cast<boost::shared_ptr<color> >(t["lev.prim.color3"]); }
      else if (t["bg_color"]) { back = object_cast<boost::shared_ptr<color> >(t["bg_color"]); }
      else if (t["bg"]) { back = object_cast<boost::shared_ptr<color> >(t["bg"]); }
      else if (t["back"]) { back = object_cast<boost::shared_ptr<color> >(t["back"]); }
      else if (t["b"]) { back = object_cast<boost::shared_ptr<color> >(t["b"]); }

      object o = globals(L)["lev"]["classes"]["font"]["rasterize_c"](f, str, fore, back, shade);
      o.push(L);
      return 1;
    }
    catch (...) {
      lev::debug_print("error on string bitmap creating lua code");
      lua_pushnil(L);
      return 1;
    }
  }

//  boost::shared_ptr<raster> font::rasterize(unsigned long code, int spacing)
//  {
//    boost::shared_ptr<raster> r;
//    try {
//      FT_Face face = cast_font(_obj)->face;
//      if (FT_Load_Char(face, code, 0)) { throw -1; }
//      if (FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL)) { throw -2; }
//
//      FT_Bitmap &bmp = face->glyph->bitmap;
//      int h = get_size();
////      int h = get_size() + (bmp.rows - face->glyph->bitmap_top);
////      int h = bmp.rows;
////      int h = bmp.rows + get_size();
////      int w = bmp.width + spacing;
//      int w = face->glyph->advance.x >> 6;
//      int offset_x = face->glyph->bitmap_left;
////      int offset_y = h - bmp.rows;
////      int offset_y = h - face->glyph->bitmap_top - bmp.rows;;
////      int offset_y = face->glyph->bitmap_top;
//      int offset_y = h - face->glyph->bitmap_top;
//      if (bmp.width <= 0) { w = get_size() / 2; }
//
////      r = raster::create(w, h);
////      r = raster::create(w, h * 1.3);
//      r = raster::create(w, h * 1.25);
//      if (! r) { throw -3; }
//
//      for (int y = 0; y < bmp.rows; y++)
//      {
//        for (int x = 0; x < bmp.width; x++)
//        {
//          r->set_pixel(offset_x + x, offset_y + y, bmp.buffer[y * bmp.pitch + x]);
//        }
//      }
//    }
//    catch (...) {
//      r.reset();
//      lev::debug_print("error on rasterized character image creation");
//    }
//    return r;
//  }

//  boost::shared_ptr<raster> font::rasterize_utf8(const std::string &str, int spacing)
//  {
//    return font::rasterize_utf16(ustring(str), spacing);
//  }
//
//  boost::shared_ptr<raster> font::rasterize_utf16(const ustring &str, int spacing)
//  {
//    boost::shared_ptr<raster> r;
//    try {
//      if (str.empty()) { throw -1; }
//      std::vector<boost::shared_ptr<raster> > array;
//      for (int i = 0; i < str.length(); i++)
//      {
//        array.push_back(boost::shared_ptr<raster>(rasterize(str.index(i), spacing)));
//      }
//      r = raster::concat(array);
//      if (! r) { throw -2; }
//    }
//    catch (...) {
//      r.reset();
//      lev::debug_print("error on rasterized string image creation");
//    }
//    return r;
//  }

  bool font::set_size(int size)
  {
    if (myFontCache::Get())
    {
      myFont::Pointer ptr = myFontCache::Get()->Load(get_path(), size);
      if (ptr)
      {
        _obj = ptr;
      }
    }
    return false;
  }

}

