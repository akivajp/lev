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

      struct item_type
      {
        public:
          item_type() : hovered(false), alpha(255),
                        func_hover(), func_lsingle() { }
          bool hovered;
          drawable::ptr img;
          drawable::ptr img_hover;
          rect::ptr r;
          luabind::object func_hover;
          luabind::object func_lsingle;
          unsigned char alpha;
      };
    protected:
      impl_map() :
        map(),
        items(), texturized(false)
      { }
    public:
      virtual ~impl_map() { }

      virtual bool clear()
      {
        items.clear();
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
        for (int i = 0; i < items.size(); i++)
        {
          item_type &item = items[i];
          unsigned char a = (short(alpha) * item.alpha) / 255;
          if (item.hovered)
          {
            item.img_hover->draw_on(dst, item.r->get_x() + x, item.r->get_y() + y, a);
          }
          else
          {
            item.img->draw_on(dst, item.r->get_x() + x, item.r->get_y() + y, a);
          }
        }
        return true;
      }

      virtual int get_h() const
      {
        int max = 0;
        for (int i = 0; i < items.size(); i++)
        {
          const item_type &item = items[i];
          int h = item.r->get_y() + item.r->get_h();
          if (h > max) { max = h; }
        }
        return max;
      }

      virtual int get_w() const
      {
        int max = 0;
        for (int i = 0; i < items.size(); i++)
        {
          const item_type &item = items[i];
          int w = item.r->get_x() + item.r->get_w();
          if (w > max) { max = w; }
        }
        return max;
      }

      virtual bool map_image(drawable::ptr img, int x, int y, unsigned char a)
      {
        if (! img) { return false; }

        rect::ptr r = rect::create(x, y, img->get_w(), img->get_h());
        if (! r) { return false; }
        items.push_back(item_type());
        item_type &item = items[items.size() - 1];
        item.img = img;
        item.img_hover = img;
        item.r = r;
        item.alpha = a;
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
                            int x, int y, luabind::object on_lsingle, luabind::object on_hover,
                            unsigned char alpha = 255)
      {
        if (! img) { return false; }
        if (! hover_img) { hover_img = img; }

        rect::ptr r = rect::create(x, y, img->get_w(), img->get_h());
        if (! r) { return false; }
        items.push_back(item_type());
        item_type &item = items[items.size() - 1];
        item.img = img;
        item.img_hover = hover_img;
        item.r = r;
        item.alpha = alpha;
        item.func_hover = on_hover;
        item.func_lsingle = on_lsingle;
        texturized = false;
        return true;
      }

      static int map_link_l(lua_State *L)
      {
        using namespace luabind;

        try {
          int x = 0, y = 0;
          object lsingle_func, hover_func;
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

          if (t["left_click_func"]) { lsingle_func = t["left_click_func"]; }
          else if (t["lsingle_func"]) { lsingle_func = t["lsingle_func"]; }
          else if (t["on_left_click"]) { lsingle_func = t["on_left_click"]; }
          else if (t["on_lclick"]) { lsingle_func = t["on_lclick"]; }
          else if (t["on_lsingle"]) { lsingle_func = t["on_lsingle"]; }
          else if (t["lua.function1"]) { lsingle_func = t["lua.function1"]; }

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
          result = m->map_link(img1, img2, x, y, lsingle_func, hover_func, alpha);
          lua_pushboolean(L, result);
        }
        catch (...) {
          lev::debug_print("error on map link setting");
          lua_pushboolean(L, false);
        }
        return 1;
      }

      virtual bool on_left_down(int x, int y)
      {
        try {
          for (int i = items.size() - 1; i >= 0; i--)
          {
            item_type &item = items[i];
            if (item.r->include(x, y))
            {
              item.hovered = true;
              return true;
            }
          }
        }
        catch (...) {
          lev::debug_print("error on left down process on image map");
          return false;
        }
        return false;
      }

      virtual bool on_left_up(int x, int y)
      {
        lua_State *L = NULL;
        try {
          for (int i = items.size() - 1; i >= 0; i--)
          {
            item_type &item = items[i];
            if (item.r->include(x, y) && item.hovered)
            {
printf("MAP HIT! ITEM(%d), X:%d Y:%d\n", i, x, y);
              if (item.func_lsingle && luabind::type(item.func_lsingle) == LUA_TFUNCTION)
              {
                L = item.func_lsingle.interpreter();
                item.func_lsingle(x, y);
                return true;
              }
            }
          }
        }
        catch (...) {
          lev::debug_print("error on left up process on image map");
          if (L) { lev::debug_print(lua_tostring(L, -1)); }
          return false;
        }
        return false;
      }

      virtual bool on_motion(int x, int y)
      {
        bool hovered_any = false;
        try {
          for (int i = items.size() - 1; i >= 0; i--)
          {
            item_type &item = items[i];
            if (item.r->include(x, y))
            {
              if (! item.hovered)
              {
                if (item.func_hover && luabind::type(item.func_hover) == LUA_TFUNCTION)
                {
                  item.func_hover(x, y);
                }
                item.hovered = true;
                hovered_any = true;
              }
            }
            else
            {
              item.hovered = false;
            }
          }
        }
        catch (...) {
          lev::debug_print("error on hovering process on image map");
          return false;
        }
        return hovered_any;
      }

      virtual bool pop_back()
      {
        if (items.size() == 0) { return false; }
        items.pop_back();
        texturized = false;
        return true;
      }

      virtual bool texturize(bool force)
      {
        if (texturized && !force) { return false; }
        for (int i = 0; i < items.size(); i++)
        {
          item_type &item = items[i];
          item.img->texturize(force);
          item.img_hover->texturize(force);
        }
        texturized = true;
        return true;
      }

      virtual drawable::ptr to_drawable()
      {
        return drawable::ptr(wptr);
      }

      boost::weak_ptr<impl_map> wptr;
      std::vector<item_type> items;
//      std::vector<bool> hovered;
//      std::vector<boost::shared_ptr<drawable> > imgs;
//      std::vector<boost::shared_ptr<drawable> > hover_imgs;
//      std::vector<boost::shared_ptr<rect> > rects;
//      std::vector<luabind::object> funcs_hover;
//      std::vector<luabind::object> funcs_lsingle;
//      std::vector<unsigned char> alphas;
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
      class_<map, clickable, clickable::ptr>("map")
        .def("clear", &map::clear)
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

