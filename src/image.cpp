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
#include "lev/util.hpp"
#include "lev/screen.hpp"
#include "lev/system.hpp"
#include "lev/timer.hpp"
//#include "resource/levana.xpm"

// libraries
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <GL/glu.h>
#include <luabind/adopt_policy.hpp>
#include <luabind/luabind.hpp>

#include "stb_image.c"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


namespace lev
{

  static bool blend_pixel(unsigned char *dst, const unsigned char *src)
  {
    if (src[3] == 0) { return true; }

    unsigned char dst_r = dst[0];
    unsigned char dst_g = dst[1];
    unsigned char dst_b = dst[2];
    unsigned char dst_a = dst[3];
    unsigned char src_r = src[0];
    unsigned char src_g = src[1];
    unsigned char src_b = src[2];
    unsigned char src_a = src[3];

    if (dst_a == 0 || src_a == 255)
    {
      dst[0] = src_r;
      dst[1] = src_g;
      dst[2] = src_b;
      dst[3] = src_a;
    }
    else if (dst_a == 255)
    {
      unsigned char base_alpha = 255 - src_a;
      dst[0] = ((unsigned short)src_r * src_a + (unsigned short)dst_r * base_alpha) / 255;
      dst[1] = ((unsigned short)src_g * src_a + (unsigned short)dst_g * base_alpha) / 255;
      dst[2] = ((unsigned short)src_b * src_a + (unsigned short)dst_b * base_alpha) / 255;
      // dst[3] = 255;
    }
    else
    {
      unsigned char base_alpha = (unsigned short)dst_a * (255 - src_a) / 255;
      dst[3] = src_a + base_alpha;
      dst[0] = ((unsigned short)src_r * src_a + (unsigned short)dst_r * base_alpha) / dst[3];
      dst[1] = ((unsigned short)src_g * src_a + (unsigned short)dst_g * base_alpha) / dst[3];
      dst[2] = ((unsigned short)src_b * src_a + (unsigned short)dst_b * base_alpha) / dst[3];
    }
    return true;
  }

  static bool blend_pixel(unsigned char *dst, const color &c)
  {
    if (c.get_a() == 0) { return true; }

    unsigned char dst_r = dst[0];
    unsigned char dst_g = dst[1];
    unsigned char dst_b = dst[2];
    unsigned char dst_a = dst[3];
    unsigned char src_r = c.get_r();
    unsigned char src_g = c.get_g();
    unsigned char src_b = c.get_b();
    unsigned char src_a = c.get_a();

    if (dst_a == 0 || src_a == 255)
    {
      dst[0] = src_r;
      dst[1] = src_g;
      dst[2] = src_b;
      dst[3] = src_a;
    }
    else if (dst_a == 255)
    {
      unsigned char base_alpha = 255 - src_a;
      dst[0] = ((unsigned short)src_r * src_a + (unsigned short)dst_r * base_alpha) / 255;
      dst[1] = ((unsigned short)src_g * src_a + (unsigned short)dst_g * base_alpha) / 255;
      dst[2] = ((unsigned short)src_b * src_a + (unsigned short)dst_b * base_alpha) / 255;
      // dst[3] = 255;
    }
    else
    {
      unsigned char base_alpha = (unsigned short)dst_a * (255 - src_a) / 255;
      dst[3] = src_a + base_alpha;
      dst[0] = ((unsigned short)src_r * src_a + (unsigned short)dst_r * base_alpha) / dst[3];
      dst[1] = ((unsigned short)src_g * src_a + (unsigned short)dst_g * base_alpha) / dst[3];
      dst[2] = ((unsigned short)src_b * src_a + (unsigned short)dst_b * base_alpha) / dst[3];
    }
    return true;
  }

  class impl_bitmap : public bitmap
  {
    public:
      typedef boost::shared_ptr<impl_bitmap> ptr;
    protected:
      impl_bitmap(int w, int h) :
        bitmap(),
        w(w), h(h), descent(0),
        tex()
      { }
    public:

      virtual ~impl_bitmap()
      {
        if (buf) { delete [] buf; }
      }

      virtual bool blit(int dst_x, int dst_y, bitmap::ptr src,
                        int src_x, int src_y, int w, int h, unsigned char alpha)
      {
        if (src == NULL) { return false; }

        unsigned char *dst_buf = get_buffer();
        const unsigned char *src_buf = src->get_buffer();
        int dst_h = get_h();
        int dst_w = get_w();
        int src_h = src->get_h();
        int src_w = src->get_w();
        if (w < 0) { w = src_w; }
        if (h < 0) { h = src_h; }
        for (int y = 0; y < h; y++)
        {
          for (int x = 0; x < w; x++)
          {
            int real_src_x = src_x + x;
            int real_src_y = src_y + y;
            if (real_src_x < 0 || real_src_x >= src_w || real_src_y < 0 || real_src_y >= src_h)
            { continue; }
            int real_dst_x = dst_x + x;
            int real_dst_y = dst_y + y;
            if (real_dst_x < 0 || real_dst_x >= dst_w || real_dst_y < 0 || real_dst_y >= dst_h)
            { continue; }
            const unsigned char *src_pixel = &src_buf[4 * (real_src_y * src_w + real_src_x)];
            blend_pixel(&dst_buf[4 * (real_dst_y * dst_w + real_dst_x)], src_pixel);
          }
        }
        return on_change();
      }

      virtual bool clear(unsigned char r = 0, unsigned char g = 0,
                         unsigned char b = 0, unsigned char a = 0)
      {
        unsigned char *pixel = get_buffer();
        int length = get_w() * get_h();
        if (a > 0)
        {
          for (int i = 0; i < length; i++)
          {
            pixel[0] = r;
            pixel[1] = g;
            pixel[2] = b;
            pixel[3] = a;
            pixel += 4;
          }
        }
        else
        {
          for (int i = 0; i < length; i++)
          {
            pixel[3] = 0;
            pixel += 4;
          }
        }
        return on_change();
      }

      virtual bitmap::ptr clone()
      {
        boost::shared_ptr<bitmap> bmp;
        try {
          bmp = bitmap::create(get_w(), get_h());
          if (! bmp) { throw -1; }
          unsigned char *src_buf = get_buffer();
          unsigned char *new_buf = bmp->get_buffer();
          long length = 4 * get_w() * get_h();
          for (int i = 0; i < length; i++)
          {
            new_buf[i] = src_buf[i];
          }
        }
        catch (...) {
          bmp.reset();
          lev::debug_print("error on bitmap memory cloning");
        }
        return bmp;
      }

      static impl_bitmap::ptr create(int w, int h)
      {
        impl_bitmap::ptr bmp;
        if (w <= 0 || h <= 0) { return bmp; }
        try {
          bmp.reset(new impl_bitmap(w, h));
          if (! bmp) { throw -1; }
          bmp->wptr = bmp;
          bmp->buf = new unsigned char [w * h * 4];
          if (! bmp->buf) { throw -2; }
          bmp->clear();
        }
        catch (...) {
          bmp.reset();
          lev::debug_print("error on bitmap memory allocation");
        }
        return bmp;
      }

      virtual bool draw(drawable::ptr src, int x, int y, unsigned char alpha)
      {
        if (! src) { return false; }
        src->draw_on(to_canvas(), x, y, alpha);
        return on_change();
      }

      virtual bool draw_on(canvas::ptr dst, int offset_x, int offset_y, unsigned char alpha)
      {
        if (! dst) { return false; }
        return dst->blit(offset_x, offset_y, to_bitmap(), 0, 0, -1, -1, alpha);
      }

      virtual bool draw_pixel(int x, int y, const color &c)
      {
        if (x < 0 || x >= get_w() || y < 0 || y >= get_h()) { return false; }
        unsigned char *buf = get_buffer();
        unsigned char *pixel = &buf[4 * (y * get_w() + x)];
        blend_pixel(pixel, c);
        return on_change();
      }

      unsigned char *get_buffer()
      {
        return buf;
      }
      const unsigned char *get_buffer() const
      {
        return buf;
      }

      virtual int get_descent() const
      {
        return descent;
      }

      virtual int get_h() const
      {
        return h;
      }

      virtual color::ptr get_pixel(int x, int y) const
      {
        if (x < 0 || x >= get_w() || y < 0 || y >= get_h()) { color::ptr(); }
        const unsigned char *buf = get_buffer();
        const unsigned char *pixel = &buf[4 * (y * get_w() + x)];
        return color::create(pixel[0], pixel[1], pixel[2], pixel[3]);
      }

      virtual rect::ptr get_rect() const
      {
        return rect::create(0, 0, get_w(), get_h());
      }

      virtual size::ptr get_size() const
      {
        return size::create(get_w(), get_h());
      }

      virtual texture::ptr get_texture() const
      {
        return tex;
      }

      virtual int get_w() const
      {
        return w;
      }

      virtual bool is_compiled() const
      {
        return false;
      }

      virtual bool is_texturized() const
      {
        if (tex) { return true; }
        return false;
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

      static bitmap::ptr load(const std::string &filename)
      {
        file::ptr f = file::open(filename);
        if (! f) { bitmap::ptr(); }
        return bitmap::load_file(f);
      }

      static bitmap::ptr load_file(file::ptr f)
      {
        bitmap::ptr bmp;
        if (! f) { return bmp; }
        try {
          int w, h;
          std::string data;
          if (! f->read_all(data)) { throw -1; }
          boost::shared_ptr<unsigned char> buf;
          buf.reset(stbi_load_from_memory((unsigned char *)data.c_str(),
                                          data.length(), &w, &h, NULL, 4),
                    stbi_image_free);
          if (! buf) { throw -2; }
          bmp = bitmap::create(w, h);
          if (! bmp) { throw -3; }

          for (int y = 0; y < h; y++)
          {
            for (int x = 0; x < w; x++)
            {
              unsigned char *pixel = buf.get() + (y * w + x) * 4;
              unsigned char r, g, b, a;
              r = pixel[0];
              g = pixel[1];
              b = pixel[2];
              a = pixel[3];
              bmp->set_pixel(x, y, color(r, g, b, a));
            }
          }
        }
        catch (...) {
          bmp.reset();
          lev::debug_print("error on bitmap data loading");
        }
        return bmp;
      }

      bool on_change()
      {
        if (tex)
        {
          tex.reset();
        }
        return true;
      }

      virtual bitmap::ptr resize(int width, int height)
      {
        bitmap::ptr bmp;
        try {
          bmp = bitmap::create(width, height);
          if (! bmp) { throw -1; }
          for (int y = 0; y < height; y++)
          {
            for (int x = 0; x < width; x++)
            {
              color::ptr c;
              c = get_pixel(long(x) * get_w() / width, long(y) * get_h() / height);
              if (c) { bmp->set_pixel(x, y, *c); }
            }
          }
        }
        catch (...) {
          bmp.reset();
          lev::debug_print("error on resized bitmap creation");
        }
        return bmp;
      }

      virtual bool save(const std::string &filename) const
      {
        const unsigned char *buf = get_buffer();
        if (stbi_write_png(filename.c_str(), get_w(), get_h(), 4, buf, 4 * get_w()) != 0)
        { return true; }
        else { return false; }
      }

      virtual bool set_descent(int d)
      {
        descent = d;
        return true;
      }

      virtual bool set_pixel(int x, int y, const color &c)
      {
        if (x < 0 || x >= get_w() || y < 0 || y >= get_h()) { return false; }
        unsigned char *buf = get_buffer();
        unsigned char *pixel = &buf[4 * (y * get_w() + x)];
        pixel[0] = c.get_r();
        pixel[1] = c.get_g();
        pixel[2] = c.get_b();
        pixel[3] = c.get_a();
        return on_change();
      }

      virtual bitmap::ptr sub(int x, int y, int w, int h)
      {
        bitmap::ptr bmp;
        try {
          bmp = bitmap::create(w, h);
          if (! bmp) { throw -1; }
          bmp->blit(0, 0, this->to_bitmap(), x, y, w, h);
        }
        catch (...) {
          bmp.reset();
          lev::debug_print("error on sub bitmap instance creation");
        }
        return bmp;
      }

      static int sub_l(lua_State *L)
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

        object o = globals(L)["lev"]["classes"]["bitmap"]["sub_c"](img, x, y, w, h);
        o.push(L);
        return 1;
      }

      virtual bool texturize(bool force)
      {
        if (tex && !force) { return false; }
        tex = texture::create(to_bitmap());
        if (! tex) { return false; }
        return true;
      }

      virtual bitmap::ptr to_bitmap()
      {
        return bitmap::ptr(wptr);
      }

      virtual canvas::ptr to_canvas()
      {
        return canvas::ptr(wptr);
      }

      virtual drawable::ptr to_drawable()
      {
        return drawable::ptr(wptr);
      }

      boost::weak_ptr<impl_bitmap> wptr;
      int w, h, descent;
      unsigned char *buf;
      boost::shared_ptr<texture> tex;
  };

  bitmap::ptr bitmap::create(int w, int h)
  {
    return impl_bitmap::create(w, h);
  }

  bitmap::ptr bitmap::load(const std::string &filename)
  {
    return impl_bitmap::load(filename);
  }

  bitmap::ptr bitmap::load_file(file::ptr f)
  {
    return impl_bitmap::load_file(f);
  }

  bitmap::ptr bitmap::load_path(boost::shared_ptr<filepath> path)
  {
    if (! path) { return bitmap::ptr(); }
    return impl_bitmap::load(path->to_str());
  }


  // texture class implementation
  class impl_texture : public texture
  {
    public:
      typedef boost::shared_ptr<impl_texture> ptr;
    protected:
      impl_texture(int w, int h) :
        texture(),
        descent(0),
        img_w(w), img_h(h), tex_w(1), tex_h(1)
      {
        while(tex_w < w) { tex_w <<= 1; }
        while(tex_h < h) { tex_h <<= 1; }
        coord_x = double(w) / tex_w;
        coord_y = double(h) / tex_h;
      }
    public:
      virtual ~impl_texture()
      {
        if (index > 0)
        {
//printf("Rel: %d\n", index);
          glDeleteTextures(1, &index);
          index = 0;
        }
      }

      virtual bool blit_on(screen::ptr dst,
                           int dst_x = 0, int dst_y = 0,
                           int src_x = 0, int src_y = 0,
                           int w = -1, int h = -1,
                           unsigned char alpha = 255) const
      {
        if (! dst) { return NULL; }
//printf("TEXTURE BLIT ON!\n");

        if (w < 0) { w = img_w; }
        if (h < 0) { h = img_h; }
        double tex_x = coord_x * src_x / img_w;
        double tex_y = coord_y * src_y / img_h;
        double tex_w = coord_x * w / img_w;
        double tex_h = coord_y * h / img_h;

        dst->set_current();
        glBindTexture(GL_TEXTURE_2D, index);
        glEnable(GL_TEXTURE_2D);
        glBegin(GL_QUADS);
          glColor4ub(255, 255, 255, alpha);
          glTexCoord2d(tex_x, tex_y);
          glVertex2i(dst_x, dst_y);
          glTexCoord2d(tex_x, tex_y + tex_h);
          glVertex2i(dst_x, dst_y + h);
          glTexCoord2d(tex_x + tex_w, tex_y + tex_h);
          glVertex2i(dst_x + w, dst_y + h);
          glTexCoord2d(tex_x + tex_w, tex_y);
          glVertex2i(dst_x + w, dst_y);
        glEnd();
        glDisable(GL_TEXTURE_2D);
        return true;
      }

      static impl_texture::ptr create(bitmap::ptr src)
      {
        impl_texture::ptr tex;
        if (! src) { return tex; }
        try {
          tex.reset( new impl_texture(src->get_w(), src->get_h()) );
          if (! tex) { throw -1; }
          tex->wptr = tex;
          glGenTextures(1, &tex->index);
//printf("Gen: %d\n", tex->index);
          if (tex->index == 0) { throw -2; }
          tex->descent = src->get_descent();

          glBindTexture(GL_TEXTURE_2D, tex->index);
          glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
          glTexImage2D(GL_TEXTURE_2D, 0 /* level */, GL_RGBA, tex->tex_w, tex->tex_h, 0 /* border */,
                       GL_RGBA, GL_UNSIGNED_BYTE, NULL /* only buffer reservation */);
          glTexSubImage2D(GL_TEXTURE_2D, 0, 0 /* x offset */, 0 /* y offset */,
                          tex->img_w, tex->img_h, GL_RGBA, GL_UNSIGNED_BYTE,
                          src->get_buffer());
        }
        catch (...) {
          tex.reset();
          lev::debug_print("error on texture instance creation");
        }
        return tex;
      }

      virtual int get_descent() const
      {
        return descent;
      }

      virtual int get_h() const
      {
        return img_h;
      }

      virtual int get_w() const
      {
        return img_w;
      }

      virtual bool is_texturized() const
      {
        return true;
      }

      static impl_texture::ptr load(const std::string &file)
      {
        impl_texture::ptr tex;
        try {
          bitmap::ptr img = bitmap::load(file);
          if (! img) { throw -1; }
          tex = impl_texture::create(img);
        }
        catch (...) {
          tex.reset();
          lev::debug_print("error on texture bitmap loading");
        }
        return tex;
      }

      virtual bool set_descent(int d)
      {
        descent = d;
        return true;
      }

      virtual drawable::ptr to_drawable()
      {
        return drawable::ptr(wptr);
      }

      boost::weak_ptr<impl_texture> wptr;
      int img_w, img_h;
      int tex_w, tex_h;
      int descent;
      double coord_x, coord_y;
      GLuint index;
  };

  texture::ptr texture::create(bitmap::ptr src)
  {
    return impl_texture::create(src);
  }

  texture::ptr texture::load(const std::string &file)
  {
    return impl_texture::load(file);
  }


  // animation class implementation
  class impl_animation : public animation
  {
    public:
      typedef boost::shared_ptr<impl_animation> ptr;
    protected:
      impl_animation(bool repeating = true) :
        animation(),
        imgs(), repeating(repeating), texturized(false)
      { }
    public:
      virtual ~impl_animation() { }

      virtual bool append(drawable::ptr img, double duration)
      {
        if (! img) { return false; }
        if (duration <= 0) { return false; }
        texturized = false;
        try {
          imgs.push_back(img);
          durations.push_back(duration);
          return true;
        }
        catch (...) {
          return false;
        }
      }

      virtual bool append_file(const std::string &filename, double duration)
      {
        return append(bitmap::load(filename), duration);
      }

      static int append_l(lua_State *L)
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
            img = object_cast<boost::shared_ptr<drawable> >(obj["drawable"]);
            lua_pushboolean(L, anim->append(img, duration));
          }
          else if (t["lua.string1"])
          {
            const char *path = object_cast<const char *>(t["lua.string1"]);
            lua_pushboolean(L, anim->append(bitmap::load(path), duration));
          }
          else if (t["lev.filepath1"])
          {
            filepath::ptr path = object_cast<filepath::ptr>(t["lev.filepath1"]);
            lua_pushboolean(L, anim->append(bitmap::load(path->to_str()), duration));
          }
          else if (t["lev.file1"])
          {
            object obj = t["lev.file1"];
            file::ptr f = object_cast<file::ptr>(obj["file"]);
            lua_pushboolean(L, anim->append(bitmap::load_file(f), duration));
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

      virtual bool compile(bool force)
      {
        for (int i = 0; i < imgs.size(); i++)
        {
          if (imgs[i]) { imgs[i]->compile(force); }
        }
        return true;
      }

      static impl_animation::ptr create(bool repeating = true)
      {
        impl_animation::ptr anim;
        try {
          anim.reset(new impl_animation(repeating));
          if (! anim) { throw -1; }
          anim->wptr = anim;
          anim->sw = stop_watch::create();
          if (! anim->sw) { throw -2; }
        }
        catch (...) {
          anim.reset();
          lev::debug_print("error on animation instance creation");
        }
        return anim;
      }

      virtual bool draw_on(canvas::ptr dst, int x, int y, unsigned char alpha)
      {
        drawable::ptr img = get_current();
//printf("ANIMATION SIZE: %d\n", (int)imgs.size());
        if (! img) { return false; }
//printf("ANIMATION DRAW ON: %p\n", img.get());
        return img->draw_on(dst, x, y, alpha);
      }

      virtual drawable::ptr get_current() const
      {
        double now = sw->get_time();
        double total = 0;
        if (imgs.size() == 0) { return drawable::ptr(); }

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
        return get_current();
      }

      virtual int get_h() const
      {
        drawable::ptr img = get_current();
        if (img) { return img->get_h(); }
        else { return 0; }
      }

      virtual int get_w() const
      {
        drawable::ptr img = get_current();
        if (img) { return img->get_w(); }
        else { return 0; }
      }

      virtual bool texturize(bool force)
      {
        if (texturized && ! force) { return false; }
        for (int i = 0; i < imgs.size(); i++)
        {
          if (imgs[i]) { imgs[i]->texturize(force); }
        }
        texturized = true;
        return true;
      }

      virtual drawable::ptr to_drawable()
      {
        return drawable::ptr(wptr);
      }

      bool repeating;
      boost::shared_ptr<stop_watch> sw;
      std::vector<boost::shared_ptr<drawable> > imgs;
      std::vector<double> durations;
      bool texturized;
      boost::weak_ptr<impl_animation> wptr;
  };

  animation::ptr animation::create(bool repeating)
  {
    return impl_animation::create(repeating);
  }


  // transition implementation
  class impl_transition : public transition
  {
    public:
      typedef boost::shared_ptr<impl_transition> ptr;

      enum transition_mode
      {
        LEV_TRAN_NONE = 0,
        LEV_TRAN_CROSS_FADE,
        LEV_TRAN_FADE_OUT,
      };

    protected:
      impl_transition() :
        transition(),
        imgs(), sw(), texturized(false)
      { }
    public:
      virtual ~impl_transition() { }

      static impl_transition::ptr create(drawable::ptr img)
      {
        impl_transition::ptr tran;
        try {
          tran.reset(new impl_transition);
          if (! tran) { throw -1; }
          tran->wptr = tran;
          tran->sw = stop_watch::create();
          if (! tran->sw) { throw -2; }

          tran->sw->start();
          tran->imgs.push_back(img);
        }
        catch (...) {
          tran.reset();
          lev::debug_print("error on image transition instance creation");
        }
        return tran;
      }

      virtual bool draw_on(canvas::ptr dst, int x, int y, unsigned char alpha)
      {
        double grad = 1.0;

        if (durations.size() >= 1 && durations[0] > 0)
        {
//printf("DURATION: %lf\n", durations[0]);
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

      virtual drawable::ptr get_current()
      {
        return imgs[0];
      }

      virtual int get_h() const
      {
        if (imgs[0]) { return imgs[0]->get_h(); }
        else { return 0; }
      }

      virtual int get_w() const
      {
        if (imgs[0]) { return imgs[0]->get_w(); }
        else { return 0; }
      }

      virtual bool is_running() const
      {
        if (imgs.size() <= 1) { return false; }
        else { return true; }
      }

      virtual bool rewind()
      {
        return sw->start(0);
      }

      virtual bool set_current(drawable::ptr img)
      {
        imgs.clear();
        durations.clear();
        modes.clear();
        imgs.push_back(img);
        texturized = false;
        return true;
      }

      virtual bool set_current(const std::string &image_path)
      {
        return set_current(bitmap::load(image_path));
      }

      static int set_current_l(lua_State *L)
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
            drawable::ptr img = object_cast<drawable::ptr>(obj["drawable"]);
            result = tran->set_current(img);
          }
          else if (t["lev.file1"])
          {
            object obj = t["lev.file1"];
            file::ptr f = object_cast<file::ptr>(obj["file"]);
            result = tran->set_current(bitmap::load_file(f));
          }
          else if (t["lev.filepath1"])
          {
            filepath::ptr path = object_cast<filepath::ptr>(t["lev.filepath1"]);
            result = tran->set_current(path->to_str());
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

      virtual bool set_next(drawable::ptr img, double duration = 1, const std::string &type = "")
      {
        if (duration < 0) { return false; }

        try {
          imgs.push_back(img);
          durations.push_back(duration);
          texturized = false;

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

      virtual bool set_next(const std::string &image_path, double duration = 1,
                            const std::string &mode = "")
      {
        return set_next(bitmap::load(image_path), duration, mode);
      }

      static int set_next_l(lua_State *L)
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
            drawable::ptr img = object_cast<drawable::ptr>(obj["drawable"]);
            result = tran->set_next(img, duration, mode);
          }
          else if (t["lev.file1"])
          {
            object obj = t["lev.file1"];
            file::ptr f = object_cast<file::ptr>(obj["file"]);
            result = tran->set_next(bitmap::load_file(f), duration, mode);
          }
          else if (t["lev.filepath1"])
          {
            filepath::ptr path = object_cast<filepath::ptr>(t["lev.filepath1"]);
            result = tran->set_next(path->to_str(), duration, mode);
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
          lev::debug_print("error on transition next image setting");
          lua_pushnil(L);
        }
        return 1;
      }

      virtual bool texturize(bool force)
      {
        if (texturized && !force) { return false; }
        for (int i = 0; i < imgs.size(); i++)
        {
          if (! imgs[i]) { continue; }
          imgs[i]->texturize(force);
        }
        texturized = true;
        return true;
      }

      virtual drawable::ptr to_drawable()
      {
        return drawable::ptr(wptr);
      }

      boost::weak_ptr<impl_transition> wptr;
      std::vector<boost::shared_ptr<drawable> > imgs;
      std::vector<double> durations;
      std::vector<transition_mode> modes;
      boost::shared_ptr<stop_watch> sw;
      bool texturized;
  };

  transition::ptr transition::create(drawable::ptr img)
  {
    return impl_transition::create(img);
  }

  transition::ptr transition::create_with_file(file::ptr f)
  {
    return impl_transition::create(bitmap::load_file(f));
  }

  transition::ptr transition::create_with_path(filepath::ptr path)
  {
    return create_with_string(path->to_str());
  }

  transition::ptr transition::create_with_string(const std::string &image_path)
  {
    return create(bitmap::load(image_path));
  }


  // layout implementation
  class impl_layout : public layout
  {
    public:
      typedef boost::shared_ptr<impl_layout> ptr;

      struct item_type
      {
        item_type() :
          x(-1), y(-1), fixed(false),
          func_hover(), func_lsingle(),
          auto_fill(true)
        { }

        drawable::ptr img;
        drawable::ptr img_hover;
        drawable::ptr img_showing;
        luabind::object func_hover;
        luabind::object func_lsingle;
        bool auto_fill;
        int x, y;
        bool fixed;
      };

    protected:
      impl_layout(int width_stop = -1) :
        layout(),
        width_stop(width_stop),
        font_text(), font_ruby(),
        items(), texturized(false)
      {
        font_text = font::load0();
        font_ruby = font::load0();
        if (font_ruby) { font_ruby->set_size(font_ruby->get_size() / 2); }
        color_fg = color::white();
        color_shade = color::black();
        hover_bg = color::transparent();
        hover_fg = color::red();
      }
    public:
      virtual ~impl_layout() { }

      int calc_max_width() const
      {
        int max_w = 0;
        int x = 0;
        for (int i = 0; i < items.size(); i++)
        {
          const item_type &item = items[i];
          if (! item.img ||
              (item.auto_fill && width_stop > 0 && x > 0 && x + item.img->get_w() > width_stop))
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

      bool calc_position(int index)
      {
        int x = 0;
        int y = 0;
//        int max_h = 0;
        int max_ascent = 0;
        int max_descent = 0;

        if (index < 0 || index >= items.size()) { return false; }
        if (! items[index].img) { return false; }

        // if position is already fixed, return it
        if (items[index].fixed) { return true; }

        // back scan
        int i = index - 1;
        for (; i >= 0; i--)
        {
          item_type &item = items[i];
          if (item.fixed)
          {
            x = item.x;
            y = item.y;
//            max_h = item.img->get_h();
            max_ascent = item.img->get_ascent();
            max_descent = item.img->get_descent();
            break;
          }
        }
        if (i < 0) { i = 0; }

        // fore scan
        for (; i < items.size(); i++)
        {
          item_type &item = items[i];
          if (! item.img ||
              (item.auto_fill && width_stop > 0 && x > 0 && x + item.img->get_w() > width_stop))
          {
            // newline
            x = 0;
//            y += max_h;
//printf("MAX ASCENT: %d, MAX DESCENT: %d\n", max_ascent, max_descent);
            y += (max_ascent + max_descent);
//            max_h = 0;
            max_ascent = 0;
            max_descent = 0;
            // back scan for position fixing
            for (int j = i - 1; j >= 0; j--)
            {
              if (items[j].fixed) { break; }
              if (items[j].img)
              {
                items[j].y = y - items[j].img->get_h();
//                items[j].y = y - items[j].img->get_ascent();
                items[j].fixed = true;
              }
            }
            if (i > index)
            {
              return true;
            }
          }
//if (! item.auto_fill && width_stop > 0 && x > 0 && x + item.img->get_w() > width_stop)
//{
//  printf("NO AUTO FILL!\n");
//}
          if (item.img)
          {
            // calc by next item
            item.x = x;
            x += item.img->get_w();
//            if (item.img->get_h() > max_h) { max_h = item.img->get_h(); }
            if (item.img->get_ascent() > max_ascent) { max_ascent = item.img->get_ascent(); }
            if (item.img->get_descent() > max_descent) { max_descent = item.img->get_descent(); }
          }
        }
//        y += max_h;
        y += (max_ascent + max_descent);
//        items[index].y = y - items[index].img->get_h();
        items[index].y = y - max_descent - items[index].img->get_ascent();
//        items[index].y = y - items[index].img->get_ascent();
        return true;
      }

      int calc_total_height() const
      {
        int x = 0;
        int y = 0;
        int max_h = 0;
        for (int i = 0; i < items.size(); i++)
        {
          const item_type &item = items[i];
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

      virtual bool clear()
      {
        items.clear();
        texturized = false;
        return true;
      }

      virtual bool complete()
      {
        for ( ; ; )
        {
          int i = get_next_index();
          if (i < 0) { return true; }
          show_index(i);
        }
      }

      static impl_layout::ptr create(int width_stop = -1)
      {
        impl_layout::ptr lay;
        try {
          lay.reset(new impl_layout(width_stop));
          if (! lay) { throw -1; }
          lay->wptr = lay;
        }
        catch (...) {
          lay.reset();
          lev::debug_print("error on layout instance creation");
        }
        return lay;
      }

      virtual bool draw_on(canvas::ptr dst, int x, int y, unsigned char alpha)
      {
        for (int i = 0; i < items.size(); i++)
        {
          item_type &item = items[i];
          if (item.img_showing)
          {
            calc_position(i);
            item.img_showing->draw_on(dst, x + item.x, y + item.y, alpha);
          }
        }
//printf("\n");
        return true;
      }

      virtual color::ptr get_fg_color()
      {
        return color_fg;
      }

      virtual font::ptr get_font()
      {
        return font_text;
      }

      virtual int get_h() const
      {
        return calc_total_height();
      }

      int get_next_index() const
      {
        for (int i = 0; i < items.size(); i++)
        {
          if (! items[i].img) { continue; }
          if (! items[i].img_showing) { return i; }
        }
        return -1;
      }

      virtual font::ptr get_ruby_font()
      {
        return font_ruby;
      }

      virtual color::ptr get_shade_color()
      {
        return color_shade;
      }

      virtual int get_w() const
      {
        if (width_stop > 0) { return width_stop; }
        return calc_max_width();
      }

      virtual bool is_done() const
      {
        return get_next_index() < 0;
      }

      virtual bool on_left_down(int x, int y)
      {
        using namespace luabind;

        for (int i = 0; i < items.size(); i++)
        {
          item_type &item = items[i];

          if (! item.img_hover) { continue; }
          if (! item.img_showing) { continue; }

          calc_position(i);
          rect r(item.x, item.y, item.img->get_w(), item.img->get_h());
          if (r.include(x, y))
          {
//            if (item.func_lsingle && type(item.func_lsingle) == LUA_TFUNCTION)
//            {
//              item.func_lsingle(x, y);
//            }
            return true;
          }
        }
        return false;
      }

      virtual bool on_left_up(int x, int y)
      {
        using namespace luabind;

        for (int i = 0; i < items.size(); i++)
        {
          item_type &item = items[i];

          if (! item.img_hover) { continue; }
          if (! item.img_showing) { continue; }

          calc_position(i);
          rect r(item.x, item.y, item.img->get_w(), item.img->get_h());
          if (r.include(x, y) && item.img_showing == item.img_hover)
          {
            if (item.func_lsingle && type(item.func_lsingle) == LUA_TFUNCTION)
            {
              item.func_lsingle(x, y);
            }
            return true;
          }
        }
        return false;
      }

      virtual bool on_motion(int x, int y)
      {
        using namespace luabind;

        for (int i = 0; i < items.size(); i++)
        {
          item_type &item = items[i];

          if (! item.img_hover) { continue; }
          if (! item.img_showing) { continue; }

          calc_position(i);
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

      virtual bool rearrange()
      {
        for (int i = 0; i < items.size(); i++)
        {
          items[i].fixed = false;
        }
        for (int i = 0; i < items.size(); i++)
        {
          calc_position(i);
        }
        return true;
      }

      virtual bool
        reserve_clickable(drawable::ptr normal,
                          drawable::ptr hover,
                          luabind::object lsingle_func,
                          luabind::object hover_func)
      {
        try {
          if (! normal) { throw -1; }
          if (! hover) { hover = normal; }

          items.push_back(item_type());
          item_type &i = items[items.size() - 1];
          i.img = normal;
          i.img_hover = hover;
          i.func_hover = hover_func;
          i.func_lsingle = lsingle_func;
          texturized = false;
          return true;
        }
        catch (...) {
          return false;
        }
      }

      virtual bool
        reserve_clickable_text(const std::string &text,
                               luabind::object lsingle_func,
                               luabind::object hover_func)
      {
        if (! font_text) { return false; }
        if (text.empty()) { return false; }
        try {
          bitmap::ptr img;
          bitmap::ptr hover_img;

          img = font_text->rasterize(text, color_fg, color::ptr(), color_shade);
          img->stroke_line(0, img->get_h() - 1,
                           img->get_w() - 1, img->get_h() - 1, color_fg, 1, "dot");
          hover_img = font_text->rasterize(text, hover_fg, hover_bg, color::ptr());
          return reserve_clickable(img, hover_img, lsingle_func, hover_func);
        }
        catch (...) {
          return false;
        }
      }

      virtual bool reserve_image(drawable::ptr img, bool auto_filling = true)
      {
        try {
          if (! img) { throw -1; }

          items.push_back(item_type());
          (items.end() - 1)->img = img;
          (items.end() - 1)->auto_fill = auto_filling;
          texturized = false;
          return true;
        }
        catch (...) {
          return false;
        }
      }

      virtual bool reserve_new_line()
      {
        // height spacing
        items.push_back(item_type());
        (items.end() - 1)->img = spacer::create(0, font_text->get_size(), 0);
        // adding new line
        items.push_back(item_type());
        texturized = false;
        return true;
      }

      virtual bool
        reserve_word(const std::string &word, const std::string &ruby = "",
                     bool auto_filling = true)
      {
        if (! font_text) { return false; }
        if (! ruby.empty() && ! font_ruby) { return false; }
        if (word.empty()) { return false; }
        try {
          bitmap::ptr img;
          if (ruby.empty()) {
            img = font_text->rasterize(word, color_fg, color::ptr(), color_shade);
            return reserve_image(img, auto_filling);
          }
          else
          {
            boost::shared_ptr<bitmap> img_ruby;
            boost::shared_ptr<bitmap> img_word;
            img_ruby = font_ruby->rasterize(ruby, color_fg, color::ptr(), color_shade);
            img_word = font_text->rasterize(word, color_fg, color::ptr(), color_shade);
            if (!img_ruby || !img_word) { throw -1; }
            int h = img_ruby->get_h() + img_word->get_h();
            int w = img_ruby->get_w();
            if (img_word->get_w() > w) { w = img_word->get_w(); }

            img = bitmap::create(w, h);
            img->draw(img_ruby, (w - img_ruby->get_w()) / 2, 0);
            img->draw(img_word, (w - img_word->get_w()) / 2, img_ruby->get_h());
            return reserve_image(img, auto_filling);
          }
        }
        catch (...) {
          return false;
        }
      }

      virtual bool set_fg_color(color::ptr fg)
      {
        if (! fg) { return false; }
        color_fg = fg;
        return true;
      }

      virtual bool set_font(font::ptr f)
      {
        if (! f) { return false; }
        font_text = f;
        return true;
      }

      virtual bool set_shade_color(color::ptr sh)
      {
        if (! sh) { return false; }
        color_shade = sh;
        return true;
      }

      virtual bool set_ruby_font(font::ptr f)
      {
        if (! f) { return false; }
        font_ruby = f;
        return true;
      }

      bool show_index(int index)
      {
        if (index >= items.size()) { return false; }
        item_type &item = items[index];
        item.img_showing = item.img;
        return true;
      }

      virtual bool show_next()
      {
        if (is_done()) { return false; }
        return show_index(get_next_index());
      }

      virtual bool texturize(bool force)
      {
        if (texturized && !force) { return false; }
        for (int i = 0; i < items.size(); i++)
        {
          item_type &item = items[i];
          if (item.img) { item.img->texturize(force); }
          if (item.img_hover) { item.img_hover->texturize(force); }
        }
        texturized = true;
        return true;
      }

      virtual drawable::ptr to_drawable()
      {
        return drawable::ptr(wptr);
      }

      boost::weak_ptr<impl_layout> wptr;
      bool texturized;
      // common format properties
      color::ptr color_fg;
      color::ptr color_shade;
      color::ptr hover_bg;
      color::ptr hover_fg;
      boost::shared_ptr<font> font_text;
      boost::shared_ptr<font> font_ruby;
      int width_stop;
      // all items
      std::vector<item_type> items;
  };

  layout::ptr layout::create(int width_stop)
  {
    return impl_layout::create(width_stop);
  }

}

int luaopen_lev_image(lua_State *L)
{
  using namespace luabind;
  using namespace lev;
  open(L);
  globals(L)["package"]["loaded"]["lev.image"] = true;
  globals(L)["require"]("lev.draw");

  module(L, "lev")
  [
    namespace_("classes")
    [
      class_<bitmap, canvas, canvas::ptr >("bitmap")
        .def("clone", &bitmap::clone)
//        .def("load", &bitmap::reload)
        .property("rect",  &bitmap::get_rect)
//        .def("reload", &bitmap::reload)
        .def("resize", &bitmap::resize)
        .def("save", &bitmap::save)
        .def("set_color", &bitmap::set_pixel)
        .def("set_pixel", &bitmap::set_pixel)
        .property("sz",  &bitmap::get_size)
        .property("size",  &bitmap::get_size)
        .scope
        [
          def("create",  &bitmap::create),
          def("create",  &bitmap::load),
          def("create",  &bitmap::load_file),
          def("create",  &bitmap::load_path),
//          def("levana_icon", &bitmap::levana_icon),
          def("sub_c", &bitmap::sub)
        ],
      class_<texture, drawable, boost::shared_ptr<drawable> >("texture")
        .scope
        [
          def("create", &texture::create),
          def("create", &texture::load)
        ],
      class_<animation, drawable, boost::shared_ptr<drawable> >("animation")
        .property("current", &animation::get_current)
        .scope
        [
          def("create", &animation::create),
          def("create", &animation::create0)
        ],
      class_<transition, drawable, boost::shared_ptr<drawable> >("transition")
        .property("current", &transition::get_current)
        .property("is_running", &transition::is_running)
        .def("rewind", &transition::rewind)
        .scope
        [
          def("create", &transition::create),
          def("create", &transition::create0),
          def("create", &transition::create_with_file),
          def("create", &transition::create_with_path),
          def("create", &transition::create_with_string)
        ],
      class_<layout, clickable, clickable::ptr>("layout")
        .def("clear", &layout::clear)
        .property("color",  &layout::get_fg_color, &layout::set_fg_color)
        .def("complete", &layout::complete)
        .property("fg",  &layout::get_fg_color, &layout::set_fg_color)
        .property("fg_color", &layout::get_fg_color, &layout::set_fg_color)
        .property("font",  &layout::get_font, &layout::set_font)
        .property("fore",  &layout::get_fg_color, &layout::set_fg_color)
        .property("is_done", &layout::is_done)
        .def("rearrange", &layout::rearrange)
        .def("reserve_clickable", &layout::reserve_clickable)
        .def("reserve_clickable", &layout::reserve_clickable_text)
        .def("reserve_image", &layout::reserve_image)
        .def("reserve_image", &layout::reserve_image1)
        .def("reserve_new_line", &layout::reserve_new_line)
        .def("reserve_word", &layout::reserve_word)
        .def("reserve_word", &layout::reserve_word_filling)
        .def("reserve_word", &layout::reserve_word1)
        .def("reserve_word", &layout::reserve_word2)
        .property("ruby",  &layout::get_ruby_font, &layout::set_ruby_font)
        .property("ruby_font",  &layout::get_ruby_font, &layout::set_ruby_font)
        .property("shade", &layout::get_shade_color, &layout::set_shade_color)
        .property("shade_color", &layout::get_shade_color, &layout::set_shade_color)
        .def("show_next", &layout::show_next)
        .property("text_font",  &layout::get_font, &layout::set_font)
        .scope
        [
          def("create", &layout::create),
          def("create", &layout::create0)
        ]
    ]
  ];
  object lev = globals(L)["lev"];
  object classes = lev["classes"];

  register_to(classes["bitmap"], "get_sub", &impl_bitmap::sub_l);
  register_to(classes["bitmap"], "sub", &impl_bitmap::sub_l);
  register_to(classes["animation"], "append", &impl_animation::append_l);
  register_to(classes["transition"], "set_current", &impl_transition::set_current_l);
  register_to(classes["transition"], "set_next", &impl_transition::set_next_l);

  lev["animation"]     = classes["animation"]["create"];
  lev["bitmap"]        = classes["bitmap"]["create"];
  lev["layout"]        = classes["layout"]["create"];
  lev["texture"]       = classes["texture"]["create"];
  lev["tex2d"]         = classes["texture"]["create"];
  lev["transition"]    = classes["transition"]["create"];

//  image["levana_icon"] = classes["image"]["levana_icon"];

  globals(L)["require"]("lev.font");
  globals(L)["require"]("lev.prim");
  globals(L)["package"]["loaded"]["lev.image"] = true;
  return 0;
}

