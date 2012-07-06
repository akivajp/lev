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
#include "lev/font.hpp"
#include "lev/fs.hpp"
#include "lev/map.hpp"
#include "lev/util.hpp"
#include "lev/screen.hpp"
#include "lev/system.hpp"
#include "lev/timer.hpp"
#include "register.hpp"
//#include "resource/levana.xpm"

// libraries
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
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

//      virtual bool draw_raster(const raster *r, int offset_x, int offset_y, color::ptr c)
//      {
//        if (! r) { return false; }
//
//        if (! c) { c = color::white(); }
//        if (! c) { return false; }
//        color copy = *c;
//
//        for (int y = 0; y < r->get_h(); y++)
//        {
//          for (int x = 0; x < r->get_w(); x++)
//          {
//            copy.set_a((unsigned short)c->get_a() * r->get_pixel(x, y) / 255);
//            draw_pixel(offset_x + x, offset_y + y, copy);
//          }
//        }
//        return on_change();
//      }

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
        bitmap::ptr bmp;
        try {
          if (! file_system::file_exists(filename)) { throw -1; }
          int w, h;
          boost::shared_ptr<unsigned char> buf(stbi_load(filename.c_str(), &w, &h, NULL, 4), stbi_image_free);
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

      static bitmap::ptr load_path(boost::shared_ptr<file_path> path)
      {
        return load(path->get_full_path());
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

  bitmap::ptr bitmap::load_path(boost::shared_ptr<file_path> path)
  {
    return impl_bitmap::load_path(path);
  }

  class myTexture
  {
    public:
      myTexture(int w, int h) : img_w(w), img_h(h), tex_w(1), tex_h(1)
      {
        while(tex_w < w) { tex_w <<= 1; }
        while(tex_h < h) { tex_h <<= 1; }
        coord_x = (double)w / tex_w;
        coord_y = (double)h / tex_h;
      }

      ~myTexture()
      {
        if (index > 0)
        {
//printf("Rel: %d\n", index);
          glDeleteTextures(1, &index);
          index = 0;
        }
      }

      static myTexture* Create(int w, int h)
      {
        if (w <= 0 || h <= 0) { return NULL; }
        myTexture *tex = NULL;
        try {
          tex = new myTexture(w, h);
          glGenTextures(1, &tex->index);
//printf("Gen: %d\n", tex->index);
          if (tex->index == 0) { throw -1; }
          return tex;
        }
        catch (...) {
//printf("ERROR!\n");
          delete tex;
          return NULL;
        }
      }

      int img_w, img_h;
      int tex_w, tex_h;
      double coord_x, coord_y;
      GLuint index;
  };
  static myTexture* cast_tex(void *obj) { return (myTexture *)obj; }

  texture::texture() : _obj(NULL) { }

  texture::~texture()
  {
    if (_obj) { delete cast_tex(_obj); }
  }

  bool texture::blit_on(boost::shared_ptr<class screen> dst,
                        int dst_x, int dst_y,
                        int src_x, int src_y,
                        int w, int h,
                        unsigned char alpha) const
  {
    if (! dst) { return NULL; }

    myTexture *tex = cast_tex(_obj);
    if (w < 0) { w = get_w(); }
    if (h < 0) { h = get_h(); }
    double tex_x = src_x * tex->coord_x / get_w();
    double tex_y = src_y * tex->coord_y / get_h();
    double tex_w = w * tex->coord_x / get_w();
    double tex_h = h * tex->coord_y / get_h();

    glBindTexture(GL_TEXTURE_2D, tex->index);
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

  texture::ptr texture::create(bitmap::ptr src)
  {
    boost::shared_ptr<texture> tex;
    if (! src) { return tex; }
    try {
      myTexture *obj = NULL;
      tex.reset(new texture);
      if (! tex) { throw -1; }
      tex->_obj = obj = myTexture::Create(src->get_w(), src->get_h());
      if (! tex->_obj) { throw -2; }

      glBindTexture(GL_TEXTURE_2D, obj->index);
      glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexImage2D(GL_TEXTURE_2D, 0 /* level */, GL_RGBA, obj->tex_w, obj->tex_h, 0 /* border */,
                   GL_RGBA, GL_UNSIGNED_BYTE, NULL /* only buffer reservation */);
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0 /* x offset */, 0 /* y offset */,
                      obj->img_w, obj->img_h, GL_RGBA, GL_UNSIGNED_BYTE,
                      src->get_buffer());
    }
    catch (...) {
      tex.reset();
      lev::debug_print("error on texture instance creation");
    }
    return tex;
  }

//  bool texture::draw_on_screen(boost::shared_ptr<screen> dst, int x, int y, unsigned char alpha) const
//  {
//    if (! dst) { return NULL; }
//
//    myTexture *tex = cast_tex(_obj);
//    glBindTexture(GL_TEXTURE_2D, tex->index);
//    glEnable(GL_TEXTURE_2D);
//    glBegin(GL_QUADS);
//      glColor4ub(255, 255, 255, alpha);
//      glTexCoord2d(0, 0);
//      glVertex2i(x, y);
//      glTexCoord2d(0, tex->coord_y);
//      glVertex2i(x, y + tex->img_h);
//      glTexCoord2d(tex->coord_x, tex->coord_y);
//      glVertex2i(x + tex->img_w, y + tex->img_h);
//      glTexCoord2d(tex->coord_x, 0);
//      glVertex2i(x + tex->img_w, y);
//    glEnd();
//    glDisable(GL_TEXTURE_2D);
//    return true;
//  }

  int texture::get_h() const
  {
    return cast_tex(_obj)->img_h;
  }

  int texture::get_w() const
  {
    return cast_tex(_obj)->img_w;
  }

  boost::shared_ptr<texture> texture::load(const std::string &file)
  {
    try {
      boost::shared_ptr<bitmap> img(bitmap::load(file));
      if (! img) { throw -1; }
      return texture::create(img);
    }
    catch (...) {
      lev::debug_print("error on texture bitmap loading");
      return boost::shared_ptr<texture>();
    }
  }


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

      virtual bool append_path(const file_path *path, double duration)
      {
        return append_file(path->get_full_path(), duration);
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
            img = object_cast<boost::shared_ptr<drawable> >(obj["to_drawable"](obj));
            lua_pushboolean(L, anim->append(img, duration));
          }
          else if (t["lua.string1"])
          {
            const char *path = object_cast<const char *>(t["lua.string1"]);
            lua_pushboolean(L, anim->append_file(path, duration));
          }
          else if (t["lua.filepath1"])
          {
            boost::shared_ptr<file_path> path = object_cast<boost::shared_ptr<file_path> >(t["lua.filepath1"]);
            lua_pushboolean(L, anim->append_path(path.get(), duration));
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
          anim.reset(new impl_animation);
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
printf("ANIMATION DRAW ON\n");
        drawable::ptr img = get_current();
printf("ANIMATION DRAW ON\n");
        if (! img) { return false; }
printf("ANIMATION DRAW ON\n");
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


  class myTransition
  {
    public:

      enum transition_mode
      {
        LEV_TRAN_NONE = 0,
        LEV_TRAN_CROSS_FADE,
        LEV_TRAN_FADE_OUT,
      };

    protected:

      myTransition() : imgs(), sw(), texturized(false) { }

    public:

      ~myTransition() { }

      static myTransition *Create(boost::shared_ptr<drawable> img)
      {
        myTransition *tran = NULL;
        try {
          tran = new myTransition;
          if (! tran) { throw -1; }
          tran->sw = stop_watch::create();
          if (! tran->sw) { throw -2; }

          tran->sw->start();
          tran->imgs.push_back(img);
          return tran;
        }
        catch (...) {
          delete tran;
          return NULL;
        }
      }

      bool DrawOn(canvas::ptr dst, int x, int y, unsigned char alpha)
      {
        double grad = 1.0;

        if (durations.size() >= 1 && durations[0] > 0)
        {
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

      bool Rewind()
      {
        return sw->start(0);
      }

      bool SetCurrent(boost::shared_ptr<drawable> img)
      {
        imgs.clear();
        durations.clear();
        modes.clear();
        imgs.push_back(img);
        texturized = false;
        return true;
      }

      bool SetNext(boost::shared_ptr<drawable> img, int duration, const std::string &type)
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

      bool TexturizeAll(bool force)
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

      std::vector<boost::shared_ptr<drawable> > imgs;
      std::vector<double> durations;
      std::vector<transition_mode> modes;
      boost::shared_ptr<stop_watch> sw;
      bool texturized;
  };
  static myTransition* cast_tran(void *obj) { return (myTransition *)obj; }

  transition::transition() : drawable(), _obj(NULL) { }

  transition::~transition()
  {
    if (_obj) { delete cast_tran(_obj); }
  }

  boost::shared_ptr<transition> transition::create(boost::shared_ptr<drawable> img)
  {
    boost::shared_ptr<transition> tran;
    try {
      tran.reset(new transition);
      if (! tran) { throw -1; }
      tran->_obj = myTransition::Create(img);
      if (! tran->_obj) { throw -2; }
    }
    catch (...) {
      tran.reset();
      lev::debug_print("error on transition instance creation");
    }
    return tran;
  }

  boost::shared_ptr<transition>
    transition::create_with_path(boost::shared_ptr<file_path> path)
  {
    return transition::create_with_string(path->get_full_path());
  }

  boost::shared_ptr<transition> transition::create_with_string(const std::string &image_path)
  {
    return transition::create(bitmap::load(image_path));
  }

  bool transition::draw_on(canvas::ptr dst, int x, int y, unsigned char alpha)
  {
    return cast_tran(_obj)->DrawOn(dst, x, y, alpha);
  }

  int transition::get_h() const
  {
    if (cast_tran(_obj)->imgs[0]) { return cast_tran(_obj)->imgs[0]->get_h(); }
    else { return 0; }
  }

  int transition::get_w() const
  {
    if (cast_tran(_obj)->imgs[0]) { return cast_tran(_obj)->imgs[0]->get_w(); }
    else { return 0; }
  }

  bool transition::is_running()
  {
    if (cast_tran(_obj)->imgs.size() <= 1) { return false; }
    else { return true; }
  }

  bool transition::rewind()
  {
    return cast_tran(_obj)->Rewind();
  }

  bool transition::set_current(boost::shared_ptr<drawable> current)
  {
    return cast_tran(_obj)->SetCurrent(current);
  }

  bool transition::set_current(const std::string &image_path)
  {
    return cast_tran(_obj)->SetCurrent(bitmap::load(image_path));
  }

  int transition::set_current_l(lua_State *L)
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
        boost::shared_ptr<drawable> img = object_cast<boost::shared_ptr<drawable> >(obj["cast"](obj));
        result = tran->set_current(img);
      }
      else if (t["lev.fs.file_path1"])
      {
        file_path* path = object_cast<file_path *>(t["lev.fs.file_path1"]);
        result = tran->set_current(path->get_full_path());
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

  bool transition::set_next(boost::shared_ptr<drawable> next, double duration, const std::string &mode)
  {
    return cast_tran(_obj)->SetNext(next, duration, mode);
  }

  bool transition::set_next(const std::string &image_path, double duration,
                                        const std::string &mode)
  {
    return cast_tran(_obj)->SetNext(bitmap::load(image_path), duration, mode);
  }

  int transition::set_next_l(lua_State *L)
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

      if (t["lev.bitmap1"])
      {
        bitmap* img = object_cast<bitmap *>(t["lev.bitmap1"]);
        result = tran->set_next(img->clone(), duration, mode);
      }
      else if (t["lev.fs.file_path1"])
      {
        boost::shared_ptr<file_path> path =
          object_cast<boost::shared_ptr<file_path> >(t["lev.fs.file_path1"]);
        result = tran->set_next(path->get_full_path(), duration, mode);
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

  bool transition::texturize(bool force)
  {
    return cast_tran(_obj)->TexturizeAll(force);
  }

  class myLayout
  {
    protected:

      struct myItem
      {
        public:
          myItem() : x(-1), y(-1), fixed(false),
            img(), img_hover(), img_showing(),
            func_hover(), func_lclick() { }
          boost::shared_ptr<drawable> img;
          boost::shared_ptr<drawable> img_hover;
          boost::shared_ptr<drawable> img_showing;
          luabind::object func_hover;
          luabind::object func_lclick;
          int x, y;
          bool fixed;
      };

      myLayout(int width_stop = -1)
        : width_stop(width_stop),
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

      ~myLayout() { }

      static myLayout* Create(int width_stop)
      {
        myLayout *lay = NULL;
        try {
          lay = new myLayout(width_stop);
          if (! lay) { throw -1; }
          return lay;
        }
        catch (...) {
          delete lay;
          return NULL;
        }
      }

      int CalcMaxWidth()
      {
        int max_w = 0;
        int x = 0;
        for (int i = 0; i < items.size(); i++)
        {
          myItem &item = items[i];
          if (! item.img || (width_stop > 0 && x > 0 && x + item.img->get_w() > width_stop))
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

      bool CalcPosition(int index)
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
          myItem &item = items[i];
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
          myItem &item = items[i];
          if (! item.img || (width_stop > 0 && x > 0 && x + item.img->get_w() > width_stop))
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
//                items[j].y = y - items[j].img->get_h();
                items[j].y = y - items[j].img->get_ascent();
                items[j].fixed = true;
              }
            }
            if (i > index)
            {
              return true;
            }
          }
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
        return true;
      }

      int CalcTotalHeight()
      {
        int x = 0;
        int y = 0;
        int max_h = 0;
        for (int i = 0; i < items.size(); i++)
        {
          myItem &item = items[i];
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

      bool Clear()
      {
        items.clear();
        texturized = false;
        return true;
      }

      bool Complete()
      {
        for ( ; ; )
        {
          int i = GetNextIndex();
          if (i < 0) { return true; }
          ShowIndex(i);
        }
      }

      bool DrawOn(canvas::ptr dst, int x, int y, unsigned char alpha)
      {
        for (int i = 0; i < items.size(); i++)
        {
          myItem &item = items[i];
          if (item.img_showing)
          {
            CalcPosition(i);
            item.img_showing->draw_on(dst, x + item.x, y + item.y, alpha);
          }
        }
//printf("\n");
        return true;
      }

      int GetNextIndex()
      {
        for (int i = 0; i < items.size(); i++)
        {
          if (! items[i].img) { continue; }
          if (! items[i].img_showing) { return i; }
        }
        return -1;
      }

      bool OnHover(int x, int y)
      {
        using namespace luabind;

        for (int i = 0; i < items.size(); i++)
        {
          myItem &item = items[i];

          if (! item.img_hover) { continue; }
          if (! item.img_showing) { continue; }

          CalcPosition(i);
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

      bool OnLeftClick(int x, int y)
      {
        using namespace luabind;

        for (int i = 0; i < items.size(); i++)
        {
          myItem &item = items[i];

          if (! item.img_hover) { continue; }
          if (! item.img_showing) { continue; }

          CalcPosition(i);
          rect r(item.x, item.y, item.img->get_w(), item.img->get_h());
          if (r.include(x, y))
          {
            if (item.func_lclick && type(item.func_lclick) == LUA_TFUNCTION)
            {
              item.func_lclick(x, y);
            }
            return true;
          }
        }
        return false;
      }

      bool RecalcAll()
      {
        for (int i = 0; i < items.size(); i++)
        {
          items[i].fixed = false;
        }
        for (int i = 0; i < items.size(); i++)
        {
          CalcPosition(i);
        }
        return true;
      }

      bool ReserveClickable(boost::shared_ptr<bitmap> normal,
                            boost::shared_ptr<bitmap> hover,
                            luabind::object lclick_func,
                            luabind::object hover_func)
      {
        try {
          if (! normal) { throw -1; }
          if (! hover) { hover = normal->clone(); }

          items.push_back(myItem());
          myItem &i = items[items.size() - 1];
          i.img = normal;
          i.img_hover = hover;
          i.func_hover = hover_func;
          i.func_lclick = lclick_func;
          texturized = false;
          return true;
        }
        catch (...) {
          return false;
        }
      }

      bool ReserveClickableText(const std::string &text,
                                luabind::object lclick_func,
                                luabind::object hover_func)
      {
        if (! font_text) { return false; }
        if (text.empty()) { return false; }
        try {
          boost::shared_ptr<bitmap> img;
          boost::shared_ptr<bitmap> hover_img;

          img = font_text->rasterize(text, color_fg, color::ptr(), color_shade);
          img->stroke_line(0, img->get_h() - 1,
                           img->get_w() - 1, img->get_h() - 1, color_fg, 1, "dot");
          hover_img = font_text->rasterize(text, hover_fg, hover_bg, color::ptr());
          return ReserveClickable(img, hover_img, lclick_func, hover_func);
        }
        catch (...) {
          return false;
        }
      }

      bool ReserveImage(boost::shared_ptr<drawable> img)
      {
        try {
          if (! img) { throw -1; }

          items.push_back(myItem());
          (items.end() - 1)->img = img;
          texturized = false;
          return true;
        }
        catch (...) {
          return false;
        }
      }

      bool ReserveNewLine()
      {
        // height spacing
        items.push_back(myItem());
        (items.end() - 1)->img = spacer::create(0, font_text->get_size(), 0);
        // adding new line
        items.push_back(myItem());
        texturized = false;
        return true;
      }

      bool ReserveWord(const std::string &word, const std::string &ruby = "")
      {
        if (! font_text) { return false; }
        if (! ruby.empty() && ! font_ruby) { return false; }
        if (word.empty()) { return false; }
        try {
          boost::shared_ptr<bitmap> img;
          if (ruby.empty()) {
            img = font_text->rasterize(word, color_fg, color::ptr(), color_shade);
            return ReserveImage(img);
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
            return ReserveImage(img);
          }
        }
        catch (...) {
          return false;
        }
      }

      bool ShowIndex(int index)
      {
        if (index >= items.size()) { return false; }
        myItem &item = items[index];
        item.img_showing = item.img;
        return true;
      }

      bool TexturizeAll(bool force)
      {
        if (texturized && !force) { return false; }
        for (int i = 0; i < items.size(); i++)
        {
          myItem &item = items[i];
          if (item.img) { item.img->texturize(force); }
          if (item.img_hover) { item.img_hover->texturize(force); }
        }
        texturized = true;
        return true;
      }

      // format properties
//      color color_fg;
      color::ptr color_fg;
      color::ptr color_shade;
//      color hover_bg;
      color::ptr hover_bg;
//      color hover_fg;
      color::ptr hover_fg;
      boost::shared_ptr<font> font_text;
      boost::shared_ptr<font> font_ruby;
      int width_stop;

      // all items
      std::vector<myItem> items;

      bool texturized;
  };
  static myLayout* cast_lay(void *obj) { return (myLayout *)obj; }

  layout::layout()
    : drawable(), _obj(NULL)
  {
  }

  layout::~layout()
  {
    if (_obj) { delete cast_lay(_obj); }
  }

  bool layout::clear()
  {
    return cast_lay(_obj)->Clear();
  }

  bool layout::complete()
  {
    if (is_done()) { return false; }
    return cast_lay(_obj)->Complete();
  }

  boost::shared_ptr<layout> layout::create(int width_stop)
  {
    boost::shared_ptr<layout> lay;
    try {
      lay.reset(new layout);
      if (! lay) { throw -1; }
      lay->_obj = myLayout::Create(width_stop);
      if (! lay->_obj) { throw -2; }
    }
    catch (...) {
      lay.reset();
      lev::debug_print("error on image layout instance creation");
    }
    return lay;
  }

  bool layout::draw_on(canvas::ptr dst, int x, int y, unsigned char alpha)
  {
    return cast_lay(_obj)->DrawOn(dst, x, y, alpha);
  }

  boost::shared_ptr<color> layout::get_fg_color()
  {
    return cast_lay(_obj)->color_fg;
  }

  boost::shared_ptr<font> layout::get_font()
  {
    return cast_lay(_obj)->font_text;
  }

  int layout::get_h() const
  {
    return cast_lay(_obj)->CalcTotalHeight();
  }

  boost::shared_ptr<font> layout::get_ruby_font()
  {
    return cast_lay(_obj)->font_ruby;
  }

  boost::shared_ptr<color> layout::get_shade_color()
  {
    return cast_lay(_obj)->color_shade;
  }

  int layout::get_w() const
  {
    if (cast_lay(_obj)->width_stop > 0) { return cast_lay(_obj)->width_stop; }
    return cast_lay(_obj)->CalcMaxWidth();
  }

  bool layout::is_done()
  {
    myLayout *lay = cast_lay(_obj);
    return lay->GetNextIndex() < 0;
  }

  bool layout::on_hover(int x, int y)
  {
    return cast_lay(_obj)->OnHover(x, y);
  }

  bool layout::on_left_click(int x, int y)
  {
    return cast_lay(_obj)->OnLeftClick(x, y);
  }

  bool layout::rearrange()
  {
    return cast_lay(_obj)->RecalcAll();
  }

  bool layout::reserve_clickable(boost::shared_ptr<bitmap> normal,
                                 boost::shared_ptr<bitmap> hover,
                                 luabind::object lclick_func,
                                 luabind::object hover_func)
  {
    return cast_lay(_obj)->ReserveClickable(boost::shared_ptr<bitmap>(normal),
                                            boost::shared_ptr<bitmap>(hover),
                                            lclick_func,
                                            hover_func);
  }

  bool layout::reserve_clickable_text(const std::string &text,
                                      luabind::object lclick_func,
                                      luabind::object hover_func)
  {
    return cast_lay(_obj)->ReserveClickableText(text, lclick_func, hover_func);
  }

  bool layout::reserve_image(boost::shared_ptr<bitmap> img)
  {
    return cast_lay(_obj)->ReserveImage(img);
  }

  bool layout::reserve_new_line()
  {
    return cast_lay(_obj)->ReserveNewLine();
  }

  bool layout::reserve_word(const std::string &word, const std::string &ruby)
  {
    return cast_lay(_obj)->ReserveWord(word, ruby);
  }

  bool layout::reserve_word_lua(luabind::object word, luabind::object ruby)
  {
    return cast_lay(_obj)->ReserveWord(util::tostring(word), util::tostring(ruby));
  }

  bool layout::reserve_word_lua1(luabind::object word)
  {
    return cast_lay(_obj)->ReserveWord(util::tostring(word));
  }

//  bool layout::set_fg_color(const color &fg)
  bool layout::set_fg_color(boost::shared_ptr<color> fg)
  {
    if (! fg) { return false; }
    cast_lay(_obj)->color_fg = fg;
    return true;
  }

  bool layout::set_font(boost::shared_ptr<font> f)
  {
    if (! f) { return false; }
    cast_lay(_obj)->font_text = f;
    return true;
  }

  bool layout::set_ruby_font(boost::shared_ptr<font> f)
  {
    if (! f) { return false; }
    cast_lay(_obj)->font_ruby = f;
    return true;
  }

  bool layout::set_shade_color(const color *c)
  {
    try {
      if (c) { cast_lay(_obj)->color_shade.reset(new color(*c)); }
      else { cast_lay(_obj)->color_shade.reset(); }
      return true;
    }
    catch (...) {
      return false;
    }
  }

  bool layout::show_next()
  {
    if (is_done()) { return false; }
    myLayout *lay = cast_lay(_obj);
    return lay->ShowIndex(lay->GetNextIndex());
  }

  bool layout::texturize(bool force)
  {
    return cast_lay(_obj)->TexturizeAll(force);
  }

}

int luaopen_lev_image(lua_State *L)
{
  using namespace luabind;
  using namespace lev;

  open(L);
  globals(L)["package"]["loaded"]["lev.image"] = true;
  globals(L)["require"]("lev.base");
  globals(L)["require"]("lev.draw");
  globals(L)["require"]("lev.font");
  globals(L)["require"]("lev.prim");

  module(L, "lev")
  [
//    namespace_("image"),
    namespace_("classes")
    [
      class_<bitmap, canvas, boost::shared_ptr<canvas> >("bitmap")
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
          def("create",  &bitmap::load_path),
//          def("draw_text_c", &bitmap::draw_text),
//          def("levana_icon", &bitmap::levana_icon),
          def("load",    &bitmap::load),
          def("load",    &bitmap::load_path),
          def("sub_c", &bitmap::sub)
        ],
      class_<texture, drawable, boost::shared_ptr<base> >("texture")
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
      class_<transition, drawable, boost::shared_ptr<base> >("transition")
        .property("is_running", &transition::is_running)
        .def("rewind", &transition::rewind)
        .scope
        [
          def("create", &transition::create),
          def("create", &transition::create0),
          def("create", &transition::create_with_path),
          def("create", &transition::create_with_string)
        ],
      class_<layout, drawable, boost::shared_ptr<base> >("layout")
        .def("clear", &layout::clear)
        .property("color",  &layout::get_fg_color, &layout::set_fg_color)
        .def("complete", &layout::complete)
        .property("fg",  &layout::get_fg_color, &layout::set_fg_color)
        .property("fg_color", &layout::get_fg_color, &layout::set_fg_color)
        .property("font",  &layout::get_font, &layout::set_font)
        .property("fore",  &layout::get_fg_color, &layout::set_fg_color)
        .property("is_done", &layout::is_done)
        .def("on_hover", &layout::on_hover)
        .def("on_lclick", &layout::on_left_click)
        .def("on_left_click", &layout::on_left_click)
        .def("rearrange", &layout::rearrange)
        .def("reserve_clickable", &layout::reserve_clickable)
        .def("reserve_clickable", &layout::reserve_clickable_text)
        .def("reserve_image", &layout::reserve_image)
        .def("reserve_new_line", &layout::reserve_new_line)
        .def("reserve_word", &layout::reserve_word_lua)
        .def("reserve_word", &layout::reserve_word_lua1)
        .property("ruby",  &layout::get_ruby_font, &layout::set_ruby_font)
        .property("ruby_font",  &layout::get_ruby_font, &layout::set_ruby_font)
        .property("shade", &layout::get_shade_color, &layout::set_shade_color)
        .property("shade_color", &layout::get_shade_color, &layout::set_shade_color)
        .def("show_next", &layout::show_next)
        .property("text_font",  &layout::get_font, &layout::set_font)
        .scope
        [
          def("create", &layout::create)
        ],
      class_<map, drawable, boost::shared_ptr<base> >("map")
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
//  object image = lev["image"];
//  register_to(classes["image"], "draw_text", &image::draw_text_l);
  register_to(classes["bitmap"], "get_sub", &impl_bitmap::sub_l);
  register_to(classes["bitmap"], "sub", &impl_bitmap::sub_l);
  register_to(classes["animation"], "append", &impl_animation::append_l);
  register_to(classes["map"], "map_image", &map::map_image_l);
  register_to(classes["map"], "map_link", &map::map_link_l);
  register_to(classes["transition"], "set_current", &transition::set_current_l);
  register_to(classes["transition"], "set_next", &transition::set_next_l);

  lev["animation"]     = classes["animation"]["create"];
  lev["bitmap"]        = classes["bitmap"]["create"];
  lev["layout"]        = classes["layout"]["create"];
  lev["map"]           = classes["map"]["create"];
  lev["texture"]       = classes["texture"]["create"];
  lev["transition"]    = classes["transition"]["create"];

//  image["animation"]   = classes["animation"]["create"];
//  image["create"]      = classes["bitmap"]["create"];
//  image["layout"]      = classes["layout"]["create"];
//  image["levana_icon"] = classes["image"]["levana_icon"];
//  image["load"]        = classes["bitmap"]["load"];
//  image["map"]         = classes["map"]["create"];
//  image["tex2d"]       = classes["texture"]["create"];
//  image["texture"]     = classes["texture"]["create"];
//  image["transition"]  = classes["transition"]["create"];

  globals(L)["package"]["loaded"]["lev.image"] = true;
  return 0;
}

