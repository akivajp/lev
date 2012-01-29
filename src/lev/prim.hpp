#ifndef _PRIM_HPP
#define _PRIM_HPP

/////////////////////////////////////////////////////////////////////////////
// Name:        src/lev/prim.hpp
// Purpose:     header for primitive expressions
// Author:      Akiva Miura <akiva.miura@gmail.com>
// Created:     01/05/2011
// Copyright:   (C) 2010-2011 Akiva Miura
// Licence:     MIT License
/////////////////////////////////////////////////////////////////////////////

#include "base.hpp"

#include <boost/shared_ptr.hpp>
#include <luabind/luabind.hpp>
#include <string>

extern "C" {
  int luaopen_lev_prim(lua_State *L);
}

namespace lev
{

  class color : public base
  {
    public:
      color(const color &orig);
      color(unsigned char r = 0, unsigned char g = 0, unsigned char b = 0, unsigned char a = 255);
      color(unsigned long argb_code);
      static boost::shared_ptr<color> create(unsigned char r, unsigned char g,
                                             unsigned char b, unsigned char a);
      static int create_l(lua_State *L);
      unsigned char get_a() const { return a; }
      unsigned char get_b() const { return b; }
      unsigned char get_g() const { return g; }
      unsigned char get_r() const { return r; }
      unsigned long get_code32() const;
      const std::string get_codestr() const;
      bool set_a(unsigned char new_a) { a = new_a; return true; }
      bool set_b(unsigned char new_b) { b = new_b; return true; }
      bool set_g(unsigned char new_g) { g = new_g; return true; }
      bool set_r(unsigned char new_r) { r = new_r; return true; }
      virtual type_id get_type_id() const { return LEV_TCOLOR; }
      virtual const char *get_type_name() const { return "lev.prim.color"; }

      static const color black() { return color(0, 0, 0, 255); }
      static const color blue() { return color(0, 0, 255, 255); }
      static const color green() { return color(0, 255, 0, 255); }
      static const color red() { return color(255, 0, 0, 255); }
      static const color white() { return color(255, 255, 255, 255); }
      static const color transparent() { return color(0, 0, 0, 0); }

    protected:
      unsigned char r, g, b, a;
  };


  class size : public base
  {
    public:
      size(int w, int h, int d = 0) : w(w), h(h), d(d), base() {}
      bool assign(int w, int h, int d = 0);
      bool assign_size(const size &sz) { return assign(sz.w, sz.h, sz.d); }
      static boost::shared_ptr<size> create(int w, int h, int d = 0);
      static int create_l(lua_State *L);
      int get_d() const { return d; }
      int get_h() const { return h; }
      int get_w() const { return w; }
      virtual type_id get_type_id() const { return LEV_TSIZE; }
      virtual const char *get_type_name() const { return "lev.prim.size"; }
      bool set_d(int new_d) { d = new_d; return true; }
      bool set_h(int new_h) { h = new_h; return true; }
      bool set_w(int new_w) { w = new_w; return true; }
    private:
      int w;
      int h;
      int d;
  };


  class vector : public base
  {
    public:
      vector(const vector &orig);
      vector(int x = 0, int y = 0, int z = 0) : x(x), y(y), z(z) { }
      bool assign(int x = 0, int y = 0, int z = 0);
      bool assign_vector(const vector &vec) { return assign(vec.x, vec.y, vec.z); }
      static boost::shared_ptr<vector> create(int x = 0, int y = 0, int z = 0);
      static int create_l(lua_State *L);
      int get_x() const { return x; }
      int get_y() const { return y; }
      int get_z() const { return z; }
      virtual type_id get_type_id() const { return LEV_TVECTOR; }
      virtual const char *get_type_name() const { return "lev.prim.vector"; }
      bool set_x(int new_x) { x = new_x; }
      bool set_y(int new_y) { y = new_y; }
      bool set_z(int new_z) { z = new_z; }
      vector operator+(vector &rhs)
      { return vector(x + rhs.x, y + rhs.y, z + rhs.z); }
    protected:
      int x;
      int y;
      int z;
  };


  class point : public base
  {
    protected:
      point();
    public:
      virtual ~point();
      bool clear_color();
      static boost::shared_ptr<point> create(vector *vec, color *col);
      static int create_l(lua_State *L);
      boost::shared_ptr<color> get_color()  { return col; }
      boost::shared_ptr<vector> get_vertex() { return vertex; }
      virtual type_id get_type_id() const { return LEV_TPOINT; }
      virtual const char *get_type_name() const { return "lev.prim.point"; }
      bool set_color(color *c);
      bool set_vertex(vector *v);
    protected:
      boost::shared_ptr<vector> vertex;
      boost::shared_ptr<color> col;
  };

  class rect : public base
  {
    public:
      rect(int x = 0, int y = 0, int w = 0, int h = 0);
      rect(const vector &pos, const size &sz);
      rect(const rect &r);
      virtual ~rect();
      bool assign(int x, int y, int w, int h);
      bool assign_position_size(const vector &v, const size &sz);
      bool assign_rect(const rect &r);
      static rect* create(int x, int y, int w, int h);
      static int create_l(lua_State *L);
      int get_bottom() const { return get_y() + sz.get_h(); }
      int get_h() const { return sz.get_h(); }
      int get_left() const { return pos.get_x(); }
      vector* get_position() { return &pos; }
      int get_right() const { return get_x() + sz.get_w(); }
      size*   get_size() { return &sz; }
      int get_top() const { return pos.get_y(); }
      int get_x() const { return pos.get_x(); }
      int get_y() const { return pos.get_y(); }
      int get_w() const { return sz.get_w(); }
      virtual type_id get_type_id() const { return LEV_TRECT; }
      virtual const char *get_type_name() const { return "lev.prim.rect"; }
      bool include(int x, int y) const;
      bool include1(const vector &p) const { return include(p.get_x(), p.get_y()); }
      bool set_bottom(int bottom) { return sz.set_h(bottom - get_y()); }
      bool set_h(int h) { return sz.set_h(h); }
      bool set_left(int left) { return pos.set_x(left); }
      bool set_position(const vector &vec);
      bool set_right(int right) { return sz.set_w(right - get_x()); }
      bool set_size(const size &new_size);
      bool set_top(int top) { return pos.set_y(top); }
      bool set_x(int x) { return pos.set_x(x); }
      bool set_y(int y) { return pos.set_y(y); }
      bool set_w(int w) { return sz.set_w(w); }

      const rect& operator=(const rect &rhs);
    protected:
      vector pos;
      size   sz;
  };

}

#endif // _PRIM_HPP

