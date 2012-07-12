#ifndef _SOUND_HPP
#define _SOUND_HPP

/////////////////////////////////////////////////////////////////////////////
// Name:        lev/sound.hpp
// Purpose:     header for sound control
// Author:      Akiva Miura <akiva.miura@gmail.com>
// Modified by:
// Created:     05/13/2011
// Copyright:   (C) 2010-2011 Akiva Miura
// Licence:     MIT License
/////////////////////////////////////////////////////////////////////////////

#include "base.hpp"
#include "fs.hpp"

#include <boost/shared_ptr.hpp>
#include <luabind/luabind.hpp>
#include <map>
#include <string>
#include <vector>

extern "C" { int luaopen_lev_sound(lua_State *L); }

namespace lev
{

  class sound : public base
  {
    public:
      typedef boost::shared_ptr<sound> ptr;
    protected:
      sound() : base() { }
    public:
      virtual ~sound() { }
      virtual bool clear() = 0;
      virtual double get_length() = 0;
      virtual float get_pan() const = 0;
      virtual double get_position() = 0;
      virtual double get_volume() const = 0;
      virtual type_id get_type_id() const { return LEV_TSOUND; }
      virtual bool is_playing() const = 0;

      virtual bool open(const std::string &filename) = 0;
      virtual bool open_file(file::ptr src) = 0;
      virtual bool open_path(filepath::ptr path) = 0;
      virtual bool open_and_play(const std::string &filename, bool repeat = false) = 0;
      bool open_and_play1(const std::string &filename) { return open_and_play(filename); }
      virtual bool open_and_play_file(file::ptr src, bool repeat = false) = 0;
      bool open_and_play_file1(file::ptr src)
      {
        return open_and_play_file(src);
      }

      bool pause() { return set_playing(false); }
      bool play(bool repeat = false) { return set_playing(true, repeat); }
      bool play0() { return play(); }
      virtual bool set_pan(float pan) = 0;
      virtual bool set_playing(bool play, bool repeat = false) = 0;
      bool set_playing1(bool play) { return set_playing(play); }
      virtual bool set_position(double pos) = 0;
      virtual bool set_volume(double vol) = 0;
  };

  class mixer : public base
  {
    public:
      typedef boost::shared_ptr<mixer> ptr;
    protected:
      mixer() { }
    public:
      virtual ~mixer() { }
      virtual bool activate(bool active = true) = 0;
      bool activate0() { return activate(); }
      virtual bool clear_slot(int slot_num) = 0;
      static mixer::ptr get();
      virtual int get_channels() const = 0;
      virtual int get_freq() const = 0;

      virtual sound::ptr get_slot(int slot_num = 0) = 0;
      sound::ptr get_slot0() { return get_slot(); }

      virtual type_id get_type_id() const { return LEV_TMIXER; }
      static mixer::ptr init();
      virtual bool is_active() const = 0;

      bool start() { return activate(true); }
      bool stop() { return activate(false); }
  };

}

#endif // _SOUND_HPP

