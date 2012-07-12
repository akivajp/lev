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
#include "lev/entry.hpp"
#include "lev/fs.hpp"
#include "lev/image.hpp"
#include "lev/prim.hpp"
#include "lev/util.hpp"

// libraries
#include <boost/weak_ptr.hpp>
#include <vector>

namespace lev
{

  // map implementation
  class impl_map : public map
  {
    public:
      typedef boost::shared_ptr<impl_map> ptr;
    protected:
      impl_map() :
        map(),
        rects(), imgs(), texturized(false)
      { }
    public:
      virtual ~impl_map() { }

      virtual bool clear()
      {
        hovered.clear();
        imgs.clear();
        hover_imgs.clear();
        rects.clear();
        alphas.clear();
        funcs_hover.clear();
        funcs_lclick.clear();
        texturized = false;
        return true;
      }

      static impl_map::ptr create()
      {
        impl_map::ptr m;
        try {
          m.reset(new impl_map);
          if (! m) { throw -1; }
          m->wptr = m;
        }
        catch (...) {
          m.reset();
          lev::debug_print("error on image map instance creation");
        }
        return m;
      }

      virtual bool draw_on(canvas::ptr dst, int x, int y, unsigned char alpha)
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

      virtual int get_h() const
      {
        int max = 0;
        for (int i = 0; i < rects.size(); i++)
        {
          int h = rects[i]->get_y() + rects[i]->get_h();
          if (h > max) { max = h; }
        }
        return max;
      }

      virtual int get_w() const
      {
        int max = 0;
        for (int i = 0; i < rects.size(); i++)
        {
          int w = rects[i]->get_x() + rects[i]->get_w();
          if (w > max) { max = w; }
        }
        return max;
      }

      virtual bool map_image(drawable::ptr img, int x, int y, unsigned char a)
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
        texturized = false;
        return true;
      }

      static int map_image_l(lua_State *L)
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
            drawable::ptr img = object_cast<drawable::ptr>(obj["drawable"]);
            result = m->map_image(img, x, y, alpha);
          }
          else if (t["lev.file1"])
          {
            object obj = t["lev.file1"];
            file::ptr f = object_cast<file::ptr>(obj["file"]);
            result = m->map_image(bitmap::load_file(f), x, y, alpha);
          }
          lua_pushboolean(L, result);
        }
        catch (...) {
          lua_pushboolean(L, false);
        }
        return 1;
      }

      virtual bool map_link(drawable::ptr img, drawable::ptr hover_img,
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
        texturized = false;
        return true;
      }

      static int map_link_l(lua_State *L)
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
            img1 = object_cast<drawable::ptr>(obj["drawable"]);
          }
          if (t["lev.drawable2"])
          {
            object obj = t["lev.drawable2"];
            img2 = object_cast<drawable::ptr>(obj["drawable"]);
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

      virtual bool on_hover(int x, int y)
      {
        bool hovered_any = false;
        try {
          for (int i = rects.size() - 1; i >= 0; i--)
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
          return false;
        }
        return hovered_any;
      }

      virtual bool on_left_click(int x, int y)
      {
        lua_State *L = NULL;
        try {
          for (int i = rects.size() - 1; i >= 0; i--)
          {
            if (rects[i]->include(x, y))
            {
//printf("MAP HIT! ITEM(%d), X:%d Y:%d\n", i, x, y);
              if (funcs_lclick[i] && luabind::type(funcs_lclick[i]) == LUA_TFUNCTION)
              {
                L = funcs_lclick[i].interpreter();
//printf("DO ON LCICK! ITEM(%d), X:%d Y:%d\n", i, x, y);
                funcs_lclick[i](x, y);
//printf("DONE ON LCICK! ITEM(%d), X:%d Y:%d\n\n", i, x, y);
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

      virtual bool pop_back()
      {
        if (imgs.size() == 0) { return false; }
        hovered.pop_back();
        imgs.pop_back();
        hover_imgs.pop_back();
        rects.pop_back();
        funcs_hover.pop_back();
        funcs_lclick.pop_back();
        alphas.pop_back();
        texturized = false;
        return true;
      }

      virtual bool texturize(bool force)
      {
        if (texturized && !force) { return false; }
        for (int i = 0; i < imgs.size(); i++)
        {
          imgs[i]->texturize(force);
          hover_imgs[i]->texturize(force);
        }
        texturized = true;
        return true;
      }

      virtual drawable::ptr to_drawable()
      {
        return drawable::ptr(wptr);
      }

      boost::weak_ptr<impl_map> wptr;
      std::vector<bool> hovered;
      std::vector<boost::shared_ptr<drawable> > imgs;
      std::vector<boost::shared_ptr<drawable> > hover_imgs;
      std::vector<boost::shared_ptr<rect> > rects;
      std::vector<luabind::object> funcs_hover;
      std::vector<luabind::object> funcs_lclick;
      std::vector<unsigned char> alphas;
      bool texturized;
  };

  map::ptr map::create()
  {
    return impl_map::create();
  }

}

int luaopen_lev_map(lua_State *L)
{
  using namespace luabind;
  using namespace lev;
  open(L);
  globals(L)["package"]["loaded"]["lev.map"] = true;
  globals(L)["require"]("lev.draw");

  module(L, "lev")
  [
    namespace_("classes")
    [
      class_<map, drawable, boost::shared_ptr<drawable> >("map")
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

  register_to(classes["map"], "map_image", &impl_map::map_image_l);
  register_to(classes["map"], "map_link", &impl_map::map_link_l);

  lev["map"]           = classes["map"]["create"];
  globals(L)["package"]["loaded"]["lev.map"] = true;
  return 0;
}

