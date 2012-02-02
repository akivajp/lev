/////////////////////////////////////////////////////////////////////////////
// Name:        src/map.cpp
// Purpose:     source for image mapping class
// Author:      Akiva Miura <akiva.miura@gmail.com>
// Modified by:
// Created:     02/01/2012
// Copyright:   (C) 2010-2012 Akiva Miura
// Licence:     MIT License
/////////////////////////////////////////////////////////////////////////////

// pre-compiled header
#include "prec.h"

// delcarations
#include "lev/map.hpp"

// dependencies
#include "lev/image.hpp"
#include "lev/prim.hpp"
#include "lev/util.hpp"

// libraries
#include <vector>

namespace lev
{

  class myMap
  {
    protected:
      myMap() : rects(), imgs() { }
    public:
      ~myMap() { }

      static myMap* Create()
      {
        myMap *m = NULL;
        try {
          m = new myMap;
          if (! m) { throw -1; }
          return m;
        }
        catch (...) {
          delete m;
          return NULL;
        }
      }

      bool Clear()
      {
        drawing.clear();
        imgs.clear();
        hover_imgs.clear();
        rects.clear();
        return true;
      }

      bool DrawOnImage(image *img, int x, int y, unsigned char alpha)
      {
        for (int i = 0; i < drawing.size(); i++)
        {
          if (drawing[i])
          {
            drawing[i]->draw_on_image(img, rects[i]->get_x() + x, rects[i]->get_y() + y, alpha);
          }
        }
        return true;
      }

      bool DrawOnScreen(screen *s, int x, int y, unsigned char alpha)
      {
        for (int i = 0; i < drawing.size(); i++)
        {
          if (drawing[i])
          {
            drawing[i]->draw_on_screen(s, rects[i]->get_x() + x, rects[i]->get_y() + y, alpha);
          }
        }
        return true;
      }

      int GetHeight()
      {
        int max = 0;
        for (int i = 0; i < rects.size(); i++)
        {
          int h = rects[i]->get_y() + rects[i]->get_h();
          if (h > max) { max = h; }
        }
        return max;
      }

      int GetWidth()
      {
        int max = 0;
        for (int i = 0; i < rects.size(); i++)
        {
          int w = rects[i]->get_x() + rects[i]->get_w();
          if (w > max) { max = w; }
        }
        return max;
      }

      bool MapImage(boost::shared_ptr<drawable> img, int x, int y)
      {
        if (! img) { return false; }

        boost::shared_ptr<rect> r = rect::create(x, y, img->get_w(), img->get_h());
        if (! r) { return false; }
        imgs.push_back(img);
        drawing.push_back(img);
        hover_imgs.push_back(img);
        rects.push_back(r);

        funcs_hover.push_back(luabind::object());
        funcs_lclick.push_back(luabind::object());
        return true;
      }

      bool MapLink(boost::shared_ptr<drawable> img, boost::shared_ptr<drawable> hover_img,
                   int x, int y, luabind::object on_lclick, luabind::object on_hover)
      {
        if (! img) { return false; }
        boost::shared_ptr<rect> r = rect::create(x, y, img->get_w(), img->get_h());
        if (! r) { return false; }
        imgs.push_back(img);
        drawing.push_back(img);
        hover_imgs.push_back(hover_img);
        rects.push_back(r);

        funcs_hover.push_back(on_hover);
        funcs_lclick.push_back(on_lclick);
        return true;
      }

      bool OnHover(int x, int y)
      {
        try {
          for (int i = 0; i < rects.size(); i++)
          {
            if (rects[i]->include(x, y))
            {
              if (drawing[i] != hover_imgs[i] && funcs_hover[i] &&
                  luabind::type(funcs_hover[i]) == LUA_TFUNCTION)
              {
                funcs_hover[i]();
              }
              drawing[i] = hover_imgs[i];
            }
            else
            {
              drawing[i] = imgs[i];
            }
          }
        }
        catch (...) {
          fprintf(stderr, "error on hovering process on image map\n");
          return false;
        }
        return true;
      }

      bool OnLeftClick(int x, int y)
      {
        lua_State *L = NULL;
        try {
          for (int i = 0; i < rects.size(); i++)
          {
            if (rects[i]->include(x, y))
            {
              if (funcs_lclick[i] && luabind::type(funcs_lclick[i]) == LUA_TFUNCTION)
              {
                L = funcs_lclick[i].interpreter();
                funcs_lclick[i]();
              }
            }
          }
        }
        catch (...) {
          fprintf(stderr, "error on left click process on image map\n");
          if (L) { fprintf(stderr, "error message: %s\n", lua_tostring(L, -1)); }
          return false;
        }
        return true;
      }

      bool TexturizeAll(bool force)
      {
        bool result_final = false;
        for (int i = 0; i < imgs.size(); i++)
        {
          bool result = imgs[i]->texturize(force);
          if (result) { result_final = true; }
          result = hover_imgs[i]->texturize(force);
          if (result) { result_final = true; }
        }
        return result_final;
      }

      bool PopBack()
      {
        if (imgs.size() == 0) { return false; }
        drawing.pop_back();
        imgs.pop_back();
        hover_imgs.pop_back();
        rects.pop_back();
        funcs_hover.pop_back();
        funcs_lclick.pop_back();
        return true;
      }

      std::vector<boost::shared_ptr<drawable> > drawing;
      std::vector<boost::shared_ptr<drawable> > imgs;
      std::vector<boost::shared_ptr<drawable> > hover_imgs;
      std::vector<boost::shared_ptr<rect> > rects;
      std::vector<luabind::object> funcs_hover;
      std::vector<luabind::object> funcs_lclick;
  };
  static myMap* cast_map(void *obj) { return (myMap *)obj; }

  map::map() : _obj(NULL) { }

  map::~map()
  {
    if (_obj)
    {
      delete cast_map(_obj);
      _obj = NULL;
    }
  }

  bool map::clear()
  {
    return cast_map(_obj)->Clear();
  }

  boost::shared_ptr<map> map::create()
  {
    boost::shared_ptr<map> m;
    try {
      m.reset(new map);
      if (! m) { throw -1; }
      m->_obj = myMap::Create();
      if (! m->_obj) { throw -2; }
    }
    catch (...) {
      m.reset();
      fprintf(stderr, "error on image map instance creation\n");
    }
    return m;
  }

  bool map::draw_on_image(image *dst, int x, int y, unsigned char alpha)
  {
    return cast_map(_obj)->DrawOnImage(dst, x, y, alpha);
  }

  bool map::draw_on_screen(screen *dst, int x, int y, unsigned char alpha)
  {
    return cast_map(_obj)->DrawOnScreen(dst, x, y, alpha);
  }

  int map::get_h() const
  {
    return cast_map(_obj)->GetHeight();
  }

  int map::get_w() const
  {
    return cast_map(_obj)->GetWidth();
  }

  bool map::map_image(boost::shared_ptr<drawable> img, int x, int y)
  {
    return cast_map(_obj)->MapImage(img, x, y);
  }

  int map::map_image_l(lua_State *L)
  {
    using namespace luabind;

    try {
      int x = 0, y = 0;
      bool result = false;

      luaL_checktype(L, 1, LUA_TUSERDATA);
      map* m = object_cast<map *>(object(from_stack(L, 1)));
      object t = util::get_merged(L, 2, -1);

      if (t["x"]) { x = object_cast<int>(t["x"]); }
      else if (t["lua.number1"]) { x = object_cast<int>(t["lua.number1"]); }

      if (t["y"]) { y = object_cast<int>(t["y"]); }
      else if (t["lua.number2"]) { y = object_cast<int>(t["lua.number2"]); }

      if (t["lev.image1"])
      {
        boost::shared_ptr<image> img = object_cast<boost::shared_ptr<image> >(t["lev.image1"]);
        result = m->map_image(img, x, y);
      }
      lua_pushboolean(L, result);
    }
    catch (...) {
      lua_pushboolean(L, false);
    }
    return 1;
  }

  bool map::map_link(boost::shared_ptr<drawable> img, boost::shared_ptr<drawable> hover_img,
                     int x, int y, luabind::object on_lclick, luabind::object on_hover)
  {
    return cast_map(_obj)->MapLink(img, hover_img, x, y, on_lclick, on_hover);
  }

  int map::map_link_l(lua_State *L)
  {
    using namespace luabind;

    try {
      int x = 0, y = 0;
      object lclick_func, hover_func;
      boost::shared_ptr<image> img1, img2;
      bool result = false;

      luaL_checktype(L, 1, LUA_TUSERDATA);
      map* m = object_cast<map *>(object(from_stack(L, 1)));
      object t = util::get_merged(L, 2, -1);

      if (t["x"]) { x = object_cast<int>(t["x"]); }
      else if (t["lua.number1"]) { x = object_cast<int>(t["lua.number1"]); }

      if (t["y"]) { y = object_cast<int>(t["y"]); }
      else if (t["lua.number2"]) { y = object_cast<int>(t["lua.number2"]); }

      if (t["left_click_func"]) { lclick_func = t["left_click_func"]; }
      else if (t["lclick_func"]) { lclick_func = t["lclick_func"]; }
      else if (t["on_left_click"]) { lclick_func = t["on_left_click"]; }
      else if (t["on_lclick"]) { lclick_func = t["on_lclick"]; }
      else if (t["lua.function1"]) { lclick_func = t["lua.function1"]; }

      if (t["hover_func"]) { hover_func = t["hover_func"]; }
      if (t["on_hover"]) { hover_func = t["on_hover"]; }
      else if (t["lua.function2"]) { hover_func = t["lua.function2"]; }

      if (t["lev.image1"])
      {
        img1 = object_cast<boost::shared_ptr<image> >(t["lev.image1"]);
      }
      if (t["lev.image2"])
      {
        img2 = object_cast<boost::shared_ptr<image> >(t["lev.image2"]);
      }
      result = m->map_link(img1, img2, x, y, lclick_func, hover_func);
      lua_pushboolean(L, result);
    }
    catch (...) {
      fprintf(stderr, "error on map link setting\n");
      lua_pushboolean(L, false);
    }
    return 1;
  }

  bool map::on_hover(int x, int y)
  {
    return cast_map(_obj)->OnHover(x, y);
  }

  bool map::on_left_click(int x, int y)
  {
    return cast_map(_obj)->OnLeftClick(x, y);
  }

  bool map::pop_back()
  {
    return cast_map(_obj)->PopBack();
  }

  bool map::texturize(bool force)
  {
    return cast_map(_obj)->TexturizeAll(force);
  }

}

