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
#include "lev/image.hpp"
#include "lev/package.hpp"
#include "lev/system.hpp"
#include "lev/util.hpp"
#include "register.hpp"

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
    namespace_("font"),
    namespace_("classes")
    [
      class_<raster, base>("raster")
        .def("get_pixel", &raster::get_pixel)
        .property("h", &raster::get_h)
        .property("height", &raster::get_h)
        .def("set_pixel", &raster::set_pixel)
        .property("w", &raster::get_w)
        .property("width", &raster::get_w),
      class_<font, base>("font")
        .def("clone", &font::clone)
        .property("family", &font::get_family)
        .property("family_name", &font::get_family)
        .property("index", &font::get_index, &font::set_index)
        .property("name", &font::get_family)
        .property("style", &font::get_style)
        .property("style_name", &font::get_style)
        .property("pixel_size", &font::get_pixel_size, &font::set_pixel_size)
        .property("px_size", &font::get_pixel_size, &font::set_pixel_size)
        .def("rasterize", &font::rasterize)
        .def("rasterize", &font::rasterize1)
        .def("rasterize", &font::rasterize_utf8)
        .def("rasterize", &font::rasterize_utf8_1)
        .def("rasterize", &font::rasterize_utf16)
        .def("rasterize", &font::rasterize_utf16_1)
        .property("size", &font::get_pixel_size, &font::set_pixel_size)
        .property("sz", &font::get_pixel_size, &font::set_pixel_size)
        .scope
        [
          def("clone", &font::clone),
          def("load", &font::load),
          def("load", &font::load0),
          def("load", &font::load1)
        ]
    ]
  ];
  object lev = globals(L)["lev"];
  object classes = lev["classes"];
  object font = lev["font"];

  font["clone"]  = classes["font"]["clone"];
  font["load"]   = classes["font"]["load"];

  globals(L)["package"]["loaded"]["lev.font"] = font;
  return 0;
}

namespace lev
{

  raster::raster() : base(), w(-1), h(-1), bitmap(NULL) { }

  raster::~raster()
  {
    if (bitmap)
    {
      delete [] bitmap;
      bitmap = NULL;
    }
  }

  boost::shared_ptr<raster> raster::concat(const std::vector<boost::shared_ptr<raster> > &array)
  {
    boost::shared_ptr<raster> r;
    try {
      int w = 0, h = 0;
      for (int i = 0; i < array.size(); i++)
      {
        if (array[i])
        {
          w += array[i]->get_w();
          if (array[i]->get_h() > h) { h = array[i]->get_h(); }
        }
      }

      r = raster::create(w, h);
      if (! r) { throw -1; }

      int pos_x = 0;
      for (int i = 0; i < array.size(); i++)
      {
        if (array[i])
        {
          for (int y = 0; y < array[i]->get_h(); y++)
          {
            for (int x = 0; x < array[i]->get_w(); x++)
            {
              unsigned char gray = array[i]->get_pixel(x, y);
              int pos_y = h - array[i]->get_h();
              r->set_pixel(pos_x + x, pos_y + y, gray);
            }
          }
          pos_x += array[i]->get_w();
        }
      }
    }
    catch (...) {
      r.reset();
      lev::debug_print("error on raster instance concatination");
    }
    return r;
  }

  boost::shared_ptr<raster> raster::create(int width, int height)
  {
    boost::shared_ptr<raster> r;
    try {
      if (width <= 0 || height <= 0) { throw -1; }
      r.reset(new raster);
      if (! r) { throw -2; }
      r->bitmap = new unsigned char [width * height];
      if (! r->bitmap) { throw -3; }
      memset(r->bitmap, 0, width * height);
      r->w = width;
      r->h = height;
    }
    catch (...) {
      r.reset();
      lev::debug_print("error on raster instance creation");
    }
    return r;
  }

  unsigned char raster::get_pixel(int x, int y) const
  {
    if (x < 0 || x >= w || y < 0 || y >= h) { return 0; }
    return bitmap[y * w + x];
  }

  bool raster::set_pixel(int x, int y, unsigned char gray)
  {
    if (x < 0 || x >= w || y < 0 || y >= h) { return false; }
    bitmap[y * w + x] = gray;
    return true;
  }


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

  int font::get_pixel_size()
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
    if (file_system::file_exists("default.ttf")) { f = font::load("default.ttf"); }
    return f;
  }

  boost::shared_ptr<raster> font::rasterize(unsigned long code, int spacing)
  {
    boost::shared_ptr<raster> r;
    try {
      FT_Face face = cast_font(_obj)->face;
      if (FT_Load_Char(face, code, 0)) { throw -1; }
      if (FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL)) { throw -2; }

      FT_Bitmap &bmp = face->glyph->bitmap;
      int h = get_pixel_size();
//      int h = get_pixel_size() + (bmp.rows - face->glyph->bitmap_top);
//      int h = bmp.rows;
//      int h = bmp.rows + get_pixel_size();
//      int w = bmp.width + spacing;
      int w = face->glyph->advance.x >> 6;
      int offset_x = face->glyph->bitmap_left;
//      int offset_y = h - bmp.rows;
//      int offset_y = h - face->glyph->bitmap_top - bmp.rows;;
//      int offset_y = face->glyph->bitmap_top;
      int offset_y = h - face->glyph->bitmap_top;
      if (bmp.width <= 0) { w = get_pixel_size() / 2; }

//      r = raster::create(w, h);
//      r = raster::create(w, h * 1.3);
      r = raster::create(w, h * 1.25);
      if (! r) { throw -3; }

      for (int y = 0; y < bmp.rows; y++)
      {
        for (int x = 0; x < bmp.width; x++)
        {
          r->set_pixel(offset_x + x, offset_y + y, bmp.buffer[y * bmp.pitch + x]);
        }
      }
    }
    catch (...) {
      r.reset();
      lev::debug_print("error on rasterized character image creation");
    }
    return r;
  }

  boost::shared_ptr<raster> font::rasterize_utf8(const std::string &str, int spacing)
  {
    return font::rasterize_utf16(unicode(str), spacing);
  }

  boost::shared_ptr<raster> font::rasterize_utf16(const unicode &str, int spacing)
  {
    boost::shared_ptr<raster> r;
    try {
      if (str.empty()) { throw -1; }
      std::vector<boost::shared_ptr<raster> > array;
      for (int i = 0; i < str.length(); i++)
      {
        array.push_back(boost::shared_ptr<raster>(rasterize(str.index(i), spacing)));
      }
      r = raster::concat(array);
      if (! r) { throw -2; }
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

  bool font::set_pixel_size(int size)
  {
    return cast_font(_obj)->SetSize(size);
  }

}

