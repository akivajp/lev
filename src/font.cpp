/////////////////////////////////////////////////////////////////////////////
// Name:        src/font.cpp
// Purpose:     source for font management classes
// Author:      Akiva Miura <akiva.miura@gmail.com>
// Created:     16/08/2010
// Copyright:   (C) 2010-2011 Akiva Miura
// Licence:     MIT License
/////////////////////////////////////////////////////////////////////////////

#include "prec.h"

#include "lev/font.hpp"
#include "lev/util.hpp"
#include "register.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H

int luaopen_lev_font(lua_State *L)
{
  using namespace lev;
  using namespace luabind;

  open(L);
  globals(L)["require"]("lev");

  module_(L, "lev")
  [
    namespace_("font"),
    namespace_("classes")
    [
      class_<font, base>("font")
        .def("clone", &font::clone, adopt(result))
        .property("family", &font::get_family)
        .property("family_name", &font::get_family)
        .property("index", &font::get_index, &font::set_index)
        .property("name", &font::get_family)
        .property("style", &font::get_style)
        .property("style_name", &font::get_style)
        .property("pixel_size", &font::get_pixel_size, &font::set_pixel_size)
        .property("px_size", &font::get_pixel_size, &font::set_pixel_size)
        .property("size", &font::get_pixel_size, &font::set_pixel_size)
        .property("sz", &font::get_pixel_size, &font::set_pixel_size)
        .scope
        [
//          def("clone", &font::clone, adopt(result)),
          def("load", &font::load, adopt(result)),
          def("load", &font::load0, adopt(result)),
          def("load", &font::load1, adopt(result))
        ]
    ]
  ];
  object lev = globals(L)["lev"];
  object classes = lev["classes"];
  object font = lev["font"];
//
//  font["clone"]  = classes["font"]["clone"];
  font["load"]   = classes["font"]["load"];

  globals(L)["package"]["loaded"]["lev.font"] = font;
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


  font* font::clone()
  {
    font *f = NULL;
    try {
      f = new font;
      f->_obj = myFont::Clone(cast_font(_obj));
      if (! f->_obj) { throw -1; }
      return f;
    }
    catch (...) {
      delete f;
      return NULL;
    }
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

  font* font::load(const std::string &file, int index)
  {
    font *f = NULL;
    try {
      f = new font;
      f->_obj = myFont::Load(file, index);
      if (! f->_obj) { throw -1; }
      return f;
    }
    catch (...) {
      delete f;
      return NULL;
    }
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

