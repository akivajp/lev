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
#include "lev/debug.hpp"
#include "lev/image.hpp"
#include "lev/prim.hpp"
#include "lev/system.hpp"
#include "lev/timer.hpp"
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
        hovered.clear();
        imgs.clear();
        hover_imgs.clear();
        rects.clear();
        alphas.clear();
        funcs_hover.clear();
        funcs_lclick.clear();
        return true;
      }

      bool DrawOn(bitmap *dst, int x, int y, unsigned char alpha)
      {
        for (int i = 0; i < imgs.size(); i++)
        {
          unsigned char a = (short(alpha) * alphas[i]) / 255;
          if (hovered[i])
          {
            hover_imgs[i]->draw_on(dst, rects[i]->get_x() + x, rects[i]->get_y() + y, a);
          }
          else
          {
            imgs[i]->draw_on(dst, rects[i]->get_x() + x, rects[i]->get_y() + y, a);
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

      bool MapImage(boost::shared_ptr<drawable> img, int x, int y, unsigned char a)
      {
        if (! img) { return false; }

        boost::shared_ptr<rect> r = rect::create(x, y, img->get_w(), img->get_h());
        if (! r) { return false; }
        imgs.push_back(img);
        hovered.push_back(false);
        hover_imgs.push_back(img);
        rects.push_back(r);
        alphas.push_back(a);

        funcs_hover.push_back(luabind::object());
        funcs_lclick.push_back(luabind::object());
        return true;
      }

      bool MapLink(boost::shared_ptr<drawable> img, boost::shared_ptr<drawable> hover_img,
                   int x, int y, luabind::object on_lclick, luabind::object on_hover,
                   unsigned char alpha = 255)
      {
        if (! img) { return false; }
        if (! hover_img) { hover_img = img; }
        boost::shared_ptr<rect> r = rect::create(x, y, img->get_w(), img->get_h());
        if (! r) { return false; }
        imgs.push_back(img);
        hovered.push_back(false);
        hover_imgs.push_back(hover_img);
        rects.push_back(r);
        alphas.push_back(alpha);

        funcs_hover.push_back(on_hover);
        funcs_lclick.push_back(on_lclick);
        return true;
      }

      bool OnHover(int x, int y)
      {
        bool hovered_any = false;
        try {
          for (int i = 0; i < rects.size(); i++)
          {
            if (rects[i]->include(x, y))
            {
              if (! hovered[i])
              {
                if (funcs_hover[i] && luabind::type(funcs_hover[i]) == LUA_TFUNCTION)
                {
                  funcs_hover[i](x, y);
                }
                hovered[i] = true;
                hovered_any = true;
              }
            }
            else
            {
              hovered[i] = false;
            }
          }
        }
        catch (...) {
          lev::debug_print("error on hovering process on image map");
          lev::debug_print(lua_tostring(system::get_interpreter(), -1));
          return false;
        }
        return hovered_any;
      }

      bool OnLeftClick(int x, int y)
      {
        lua_State *L = NULL;
        try {
//          for (int i = 0; i < rects.size(); i++)
          for (int i = rects.size() - 1; i >= 0; i--)
          {
            if (rects[i]->include(x, y))
            {
              if (funcs_lclick[i] && luabind::type(funcs_lclick[i]) == LUA_TFUNCTION)
              {
                L = funcs_lclick[i].interpreter();
                funcs_lclick[i](x, y);
                return true;
              }
            }
          }
        }
        catch (...) {
          lev::debug_print("error on left click process on image map");
          if (L) { lev::debug_print(lua_tostring(L, -1)); }
          return false;
        }
        return false;
      }

      bool PopBack()
      {
        if (imgs.size() == 0) { return false; }
        hovered.pop_back();
        imgs.pop_back();
        hover_imgs.pop_back();
        rects.pop_back();
        funcs_hover.pop_back();
        funcs_lclick.pop_back();
        alphas.pop_back();
        return true;
      }

      std::vector<bool> hovered;
      std::vector<boost::shared_ptr<drawable> > imgs;
      std::vector<boost::shared_ptr<drawable> > hover_imgs;
      std::vector<boost::shared_ptr<rect> > rects;
      std::vector<luabind::object> funcs_hover;
      std::vector<luabind::object> funcs_lclick;
      std::vector<unsigned char> alphas;
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
      lev::debug_print("error on image map instance creation");
    }
    return m;
  }

  bool map::draw_on(bitmap *dst, int x, int y, unsigned char alpha)
  {
    return cast_map(_obj)->DrawOn(dst, x, y, alpha);
  }

  int map::get_h() const
  {
    return cast_map(_obj)->GetHeight();
  }

  int map::get_w() const
  {
    return cast_map(_obj)->GetWidth();
  }

  bool map::map_image(boost::shared_ptr<drawable> img, int x, int y, unsigned char alpha)
  {
    return cast_map(_obj)->MapImage(img, x, y, alpha);
  }

  int map::map_image_l(lua_State *L)
  {
    using namespace luabind;

    try {
      int x = 0, y = 0;
      unsigned char alpha = 255;
      bool result = false;

      luaL_checktype(L, 1, LUA_TUSERDATA);
      map* m = object_cast<map *>(object(from_stack(L, 1)));
      object t = util::get_merged(L, 2, -1);

      if (t["x"]) { x = object_cast<int>(t["x"]); }
      else if (t["lua.number1"]) { x = object_cast<int>(t["lua.number1"]); }

      if (t["y"]) { y = object_cast<int>(t["y"]); }
      else if (t["lua.number2"]) { y = object_cast<int>(t["lua.number2"]); }

      if (t["alpha"]) { alpha = object_cast<unsigned char>(t["alpha"]); }
      else if (t["a"]) { alpha = object_cast<unsigned char>(t["a"]); }
      else if (t["lua.number3"]) { alpha = object_cast<unsigned char>(t["lua.number3"]); }

      if (t["lev.drawable1"])
      {
        object obj = t["lev.drawable1"];
        boost::shared_ptr<drawable> img = object_cast<boost::shared_ptr<drawable> >(obj["as_drawable"](obj));
        result = m->map_image(img, x, y, alpha);
      }
      lua_pushboolean(L, result);
    }
    catch (...) {
      lev::debug_print("error on mapping");
      lua_pushboolean(L, false);
    }
    return 1;
  }

  bool map::map_link(boost::shared_ptr<drawable> img, boost::shared_ptr<drawable> hover_img,
                     int x, int y, luabind::object on_lclick, luabind::object on_hover,
                     unsigned char alpha)
  {
    return cast_map(_obj)->MapLink(img, hover_img, x, y, on_lclick, on_hover, alpha);
  }

  int map::map_link_l(lua_State *L)
  {
    using namespace luabind;

    try {
      int x = 0, y = 0;
      object lclick_func, hover_func;
      boost::shared_ptr<drawable> img1, img2;
      unsigned char alpha = 255;
      bool result = false;

      luaL_checktype(L, 1, LUA_TUSERDATA);
      map* m = object_cast<map *>(object(from_stack(L, 1)));
      object t = util::get_merged(L, 2, -1);

      if (t["x"]) { x = object_cast<int>(t["x"]); }
      else if (t["lua.number1"]) { x = object_cast<int>(t["lua.number1"]); }

      if (t["y"]) { y = object_cast<int>(t["y"]); }
      else if (t["lua.number2"]) { y = object_cast<int>(t["lua.number2"]); }

      if (t["alpha"]) { alpha = object_cast<unsigned char>(t["alpha"]); }
      else if (t["a"]) { alpha = object_cast<unsigned char>(t["a"]); }
      else if (t["lua.number3"]) { y = object_cast<int>(t["lua.number3"]); }

      if (t["left_click_func"]) { lclick_func = t["left_click_func"]; }
      else if (t["lclick_func"]) { lclick_func = t["lclick_func"]; }
      else if (t["on_left_click"]) { lclick_func = t["on_left_click"]; }
      else if (t["on_lclick"]) { lclick_func = t["on_lclick"]; }
      else if (t["lua.function1"]) { lclick_func = t["lua.function1"]; }

      if (t["hover_func"]) { hover_func = t["hover_func"]; }
      if (t["on_hover"]) { hover_func = t["on_hover"]; }
      else if (t["lua.function2"]) { hover_func = t["lua.function2"]; }

      if (t["lev.drawable1"])
      {
        object obj = t["lev.drawable1"];
        img1 = object_cast<boost::shared_ptr<drawable> >(obj["as_drawable"](obj));
      }
      if (t["lev.drawable2"])
      {
        object obj = t["lev.drawable2"];
        img2 = object_cast<boost::shared_ptr<drawable> >(obj["as_drawable"](obj));
      }
      result = m->map_link(img1, img2, x, y, lclick_func, hover_func, alpha);
      lua_pushboolean(L, result);
    }
    catch (...) {
      lev::debug_print("error on map link setting");
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

}

