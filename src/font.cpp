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
#include "lev/system.hpp"
#include "lev/util.hpp"

// libraries
#include <ft2build.h>
#include FT_FREETYPE_H

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
//    namespace_("font"),
    namespace_("classes")
    [
      class_<font, base>("font")
        .def("clone", &font::clone)
        .property("family", &font::get_family)
        .property("family_name", &font::get_family)
        .property("index", &font::get_index, &font::set_index)
        .property("name", &font::get_family)
        .property("style", &font::get_style)
        .property("style_name", &font::get_style)
        .property("pixel_size", &font::get_size, &font::set_size)
        .property("px_size", &font::get_size, &font::set_size)
//          def("rasterize", &font::rasterize)
//        .def("rasterize", &font::rasterize_raw)
//        .def("rasterize", &font::rasterize_utf8)
//        .def("rasterize", &font::rasterize_utf16)
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
//  object font = lev["font"];
  register_to(classes["font"], "rasterize", &font::rasterize_l);

  lev["font"] = classes["font"]["load"];
//  font["clone"]  = classes["font"]["clone"];
//  font["load"]   = classes["font"]["load"];

  globals(L)["package"]["loaded"]["lev.font"] = true;
  return 0;
}

namespace lev
{

  class myFontManager
  {
    protected:
      myFontManager() : lib(NULL) { }

      ~myFontManager() { }

    public:

      static myFontManager* Get()
      {
        return Init();
      }

      static myFontManager* Init()
      {
        static myFontManager man;
        if (man.lib) { return &man; }
        if (FT_Init_FreeType(&man.lib)) { return NULL; }
        return &man;
      }

      FT_Library lib;
  };

  class myFont
  {
    protected:
      myFont() : file(), face(NULL), size(20) { }

    public:

      ~myFont()
      {
        Clear();
      }

      bool Clear()
      {
        if (face)
        {
          FT_Done_Face(face);
          face = NULL;
        }
      }

      static myFont* Clone(const myFont *orig)
      {
        myFont *f = NULL;
        myFontManager *man = myFontManager::Get();
        if (! man || ! orig) { return NULL; }
        try {
          f = new myFont;
          if (FT_New_Face(man->lib, orig->file.c_str(),
              orig->face->face_index, &f->face)) { throw -1; }
          f->file = orig->file;
          f->SetSize(orig->size);
          return f;
        }
        catch (...) {
          delete f;
          return NULL;
        }
      }

      static myFont* Load(const std::string &file, int index)
      {
        myFont *f = NULL;
        myFontManager *man = myFontManager::Get();
        if (! man) { return NULL; }
        try {
          f = new myFont;
          if (FT_New_Face(man->lib, file.c_str(), index, &f->face)) { throw -1; }
          f->file = file;
          f->SetSize(20);
          return f;
        }
        catch (...) {
          delete f;
          return NULL;
        }
      }

      bool SetIndex(int index)
      {
        myFontManager *man = myFontManager::Get();
        try {
          FT_Face f;
          if (FT_New_Face(man->lib, file.c_str(), index, &f)) { throw -1; }
          Clear();
          face = f;
          return true;
        }
        catch (...) {
          return false;
        }
      }

      bool SetSize(int sz)
      {
        if (size <= 0) { return false; }
        if (FT_Set_Pixel_Sizes(face, 0, sz)) { return false; }
        size = sz;
        return true;
      }

      std::string file;
      FT_Face face;
      int size;
  };


  static myFont* cast_font(void *obj) { return (myFont *)obj; }

  font::font() : base(), _obj(NULL) { }

  font::~font()
  {
    if (_obj) { delete cast_font(_obj); }
    _obj = NULL;
  }


  boost::shared_ptr<font> font::clone()
  {
    boost::shared_ptr<font> f;
    try {
      f.reset(new font);
      if (! f) { throw -1; }
      f->_obj = myFont::Clone(cast_font(_obj));
      if (! f->_obj) { throw -2; }
    }
    catch (...) {
      f.reset();
      lev::debug_print("error on font instance cloning");
    }
    return f;
  }


  std::string font::get_family()
  {
    return cast_font(_obj)->face->family_name;
  }

  std::string font::get_style()
  {
    return cast_font(_obj)->face->style_name;
  }

  int font::get_index()
  {
    return cast_font(_obj)->face->face_index;
  }

  int font::get_size()
  {
    return cast_font(_obj)->size;
  }

  boost::shared_ptr<font> font::load(const std::string &file, int index)
  {
    boost::shared_ptr<font> f;
    try {
      f.reset(new font);
      if (! f) { throw -1; }
      f->_obj = myFont::Load(file, index);
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
    if (system::get())
    {
      f = package::find_font0(system::get()->get_interpreter());
      if (f) { return f; }
    }
    if (fs::is_file("default.ttf")) { f = font::load("default.ttf"); }
    return f;
  }

  bitmap::ptr
    font::rasterize(const std::string &str, boost::shared_ptr<color> fg,
                    boost::shared_ptr<color> bg, boost::shared_ptr<color> shade)
  {
     boost::shared_ptr<bitmap> bmp, bmp_fg, bmp_shade;
     if (! fg) { return bmp; }
     bmp_fg = rasterize_utf8(str, fg);
 //    if (shade && shade->get_a() > 0) { bmp = bitmap::create(w + 2, get_height() + 2); }
     if (shade && shade->get_a() > 0)
     {
       bmp_shade = rasterize_utf8(str, shade);
       bmp = bitmap::create(bmp_shade->get_w() + 1, bmp_shade->get_h() + 1);
     }
     else
     {
       bmp = bitmap::create(bmp_fg->get_w(), bmp_fg->get_h());
     }
     if (! bmp) { return bmp; }
     bmp->set_descent(bmp_fg->get_descent());
     if (bg) { bmp->clear_color(bg); }
     if (bmp_shade)
     {
       bmp->draw(bmp_shade, 1, 1);
     }
     bmp->draw(bmp_fg, 0, 0);
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
      else if (t["lev.ustring1"]) { str = object_cast<const char *>(t["lev.ustring1"]["str"]); }
      else if (t["text"]) { str = object_cast<const char *>(t["text"]); }
      else if (t["t"]) { str = object_cast<const char *>(t["t"]); }
      else if (t["string"]) { str = object_cast<const char *>(t["string"]); }
      else if (t["str"]) { str = object_cast<const char *>(t["str"]); }
      if (! str)
      {
        luaL_error(L, "text (string) is not specified");
        return 0;
      }

      if (t["lev.color1"]) { fore = object_cast<boost::shared_ptr<color> >(t["lev.color1"]); }
      else if (t["fg_color"]) { fore = object_cast<boost::shared_ptr<color> >(t["fg_color"]); }
      else if (t["fg"]) { fore = object_cast<boost::shared_ptr<color> >(t["fg"]); }
      else if (t["fore"]) { fore = object_cast<boost::shared_ptr<color> >(t["fore"]); }
      else if (t["f"]) { fore = object_cast<boost::shared_ptr<color> >(t["f"]); }
      else if (t["color"]) { fore = object_cast<boost::shared_ptr<color> >(t["color"]); }
      else if (t["c"]) { fore = object_cast<boost::shared_ptr<color> >(t["c"]); }

      if (t["lev.color2"]) { back = object_cast<boost::shared_ptr<color> >(t["lev.color2"]); }
      else if (t["bg_color"]) { back = object_cast<boost::shared_ptr<color> >(t["bg_color"]); }
      else if (t["bg"]) { back = object_cast<boost::shared_ptr<color> >(t["bg"]); }
      else if (t["back"]) { back = object_cast<boost::shared_ptr<color> >(t["back"]); }
      else if (t["b"]) { back = object_cast<boost::shared_ptr<color> >(t["b"]); }

      if (t["lev.color3"]) { shade = object_cast<boost::shared_ptr<color> >(t["lev.color3"]); }
      else if (t["shade_color"]) { shade = object_cast<boost::shared_ptr<color> >(t["shade_color"]); }
      else if (t["shade"]) { shade = object_cast<boost::shared_ptr<color> >(t["shade"]); }
      else if (t["sh"]) { shade = object_cast<boost::shared_ptr<color> >(t["sh"]); }
      else if (t["s"]) { shade = object_cast<boost::shared_ptr<color> >(t["s"]); }

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

  bitmap::ptr font::rasterize_raw(unsigned long code, color::ptr fg)
  {
    bitmap::ptr r;
    try {
      FT_Face face = cast_font(_obj)->face;
      if (FT_Load_Char(face, code, 0)) { throw -1; }
      if (FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL)) { throw -2; }

      FT_Bitmap &bmp = face->glyph->bitmap;
      int h = bmp.rows;
      int w = face->glyph->advance.x >> 6;
      int offset_x = face->glyph->bitmap_left;
//      int offset_y = h - face->glyph->bitmap_top;
      if (bmp.width <= 0) { return bitmap::create(get_size() / 2, 1); }
      // printf("TOP: %d, H: %d\n", face->glyph->bitmap_top, bmp.rows)    ;
      // printf("LEFT: %d, WIDTH: %d,  ADVANCE X: %d\n", face->glyph->    bitmap_left, bmp.width, w);
      // printf("WIDTH: %d\n", w);
      // al_set_new_bitmap_flags(ALLEGRO_MEMORY_BITMAP || ALLEGRO_NO_P    RESERVE_TEXTURE);

      r = bitmap::create(w, h);
      if (! r) { throw -3; }
      r->set_descent(bmp.rows - face->glyph->bitmap_top);

      color c(*fg);
      unsigned char a = c.get_a();
      for (int y = 0; y < bmp.rows; y++)
      {
        for (int x = 0; x < bmp.width; x++)
        {
          unsigned char d = bmp.buffer[y * bmp.pitch + x];
          c.set_a(a * d / 255.0);
          r->set_pixel(offset_x + x, y, c);
        }
      }
    }
    catch (...) {
      r.reset();
      lev::debug_print("error on rasterized character image creation");
    }
    return r;
  }

  boost::shared_ptr<bitmap> font::rasterize_utf8(const std::string &str, color::ptr fg)
  {
    return font::rasterize_utf16(ustring(str), fg);
  }

  boost::shared_ptr<bitmap> font::rasterize_utf16(const ustring &str, color::ptr fg)
  {
     boost::shared_ptr<bitmap> r;
     if (! fg) { return r; }
     try {
       if (str.empty()) { throw -1; }
       std::vector<boost::shared_ptr<bitmap> > array;
       int max_a = 0, max_d = 0, total_w = 0;
       max_a = get_size();
       max_d = get_size() * 0.2;
       int current_x = 0;
       for (int i = 0; i < str.length(); i++)
       {
         boost::shared_ptr<bitmap> b = rasterize_raw(str.index(i), fg);
         if (b)
         {
           if (b->get_ascent() > max_a) { max_a = b->get_ascent(); }
           if (b->get_descent() > max_d) { max_d = b->get_descent(); }
           total_w += b->get_w();
           array.push_back(b);
         }
       }
//printf("MAX A: %d, MAX D: %d\n", max_a, max_d);
       r = bitmap::create(total_w, max_a + max_d);
       if (! r) { throw -2; }
       r->set_descent(max_d);
       for (int i = 0; i < array.size(); i++)
       {
         boost::shared_ptr<bitmap> b = array[i];
         r->draw(b, current_x, max_a - b->get_ascent());
         current_x += b->get_w();
       }
     }
     catch (...) {
       r.reset();
       lev::debug_print("error on rasterized string image creation");
     }
     return r;
  }

  bool font::set_index(int index)
  {
    return cast_font(_obj)->SetIndex(index);
  }

  bool font::set_size(int size)
  {
    return cast_font(_obj)->SetSize(size);
  }

}

