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

#include <boost/shared_ptr.hpp>
#include <luabind/luabind.hpp>
#include <map>
#include <string>
#include <vector>

extern "C" { int luaopen_lev_sound(lua_State *L); }

namespace lev
{

  class system;

  class sound : public base
  {
    protected:
      sound();
    public:
      ~sound();
      bool clear();
      static sound *create();
      double get_length();
      float get_pan();
      double get_position();
      void* get_rawobj() { return _obj; }
      virtual type_id get_type_id() const { return LEV_TSOUND; }
      virtual const char *get_type_name() const { return "lev.sound"; }
      bool is_playing();
      bool load(const std::string &filename);
      bool load_and_play(const std::string &filename, bool repeat = false);
      bool load_and_play1(const std::string &filename) { return load_and_play(filename); }
      bool open(const std::string &filename);
      bool pause() { return set_playing(false); }
      bool play(bool repeat = false) { return set_playing(true, repeat); }
      bool play0() { return play(); }
      bool set_pan(float pan);
      bool set_playing(bool play, bool repeat = false);
      bool set_playing1(bool play) { return set_playing(play); }
      bool set_position(double pos);
    protected:
      void *_obj;
  };

  class mixer : public base
  {
    protected:
      mixer();
    public:
      ~mixer();
      bool activate(bool active = true);
      bool activate0() { return activate(); }
      bool clear_slot(int slot_num);
//      static mixer* get() { return init(); }
      static boost::shared_ptr<mixer> get() { return singleton; }
      int get_channels();
      int get_freq();
      void *get_rawobj() { return _obj; }
      sound *get_slot(int slot_num = 0);
      sound *get_slot0() { return get_slot(); }
//      static int get_field(lua_State *L);
      bool get_playing();
      virtual type_id get_type_id() const { return LEV_TMIXER; }
      virtual const char *get_type_name() const { return "lev.sound.mixer"; }
//      static mixer* init();
      static boost::shared_ptr<mixer> init(boost::shared_ptr<system> sys);
      bool is_active();
      bool start() { return activate(true); }
      bool stop() { return activate(false); }
    protected:
      void *_obj;
      static boost::shared_ptr<mixer> singleton;
  };

}

#endif // _SOUND_HPP

