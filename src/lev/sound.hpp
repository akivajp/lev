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

#include <luabind/luabind.hpp>
#include <map>
#include <string>
#include <vector>

extern "C" { int luaopen_lev_sound(lua_State *L); }

namespace lev
{

  class channel : public base
  {
    private:
      channel();
    public:
      ~channel();
      bool clean();
      static channel *create(int id);
      int  get_id() { return _id; }
      double get_length();
      float get_pan();
      bool get_playing();
      double get_position();
//      virtual type_id get_type_id() const { return LEV_TCHANNEL; }
      virtual const char *get_type_name() const { return "lev.sound.channel"; }
      bool load(const char *filename);
      bool open(const char *filename);
      bool pause() { return set_playing(false); }
      bool play() { return set_playing(true); }
      bool play_with(const char *);
      bool set_pan(float pan);
      bool set_playing(bool play);
      bool set_position(double pos);

      friend class mixer;
    private:
      int _id;
      void *_obj;
  };

  class mixer : public base
  {
    private:
      mixer();
    public:
      ~mixer();
      static mixer *create();
      static int create_l(lua_State *L);
      bool clean_channel(int channel_num);
      channel *get_channel(int channel_num = 0);
      channel *get_channel0() { return get_channel(); }
      static int get_field(lua_State *L);
      bool get_playing();
//      virtual type_id get_type_id() const { return LEV_TMIXER; }
      virtual const char *get_type_name() const { return "lev.sound.mixer"; }
      bool pause() { return set_playing(false); }
      bool play() { return set_playing(true); }
      bool set_playing(bool play);
    private:
      void *_obj;
  };

  class sound : public base
  {
    private:
      sound();
      ~sound();
      static sound* get();
    public:
      static bool init();
      static bool play(const char *filename);
      static bool stop_all();
    private:
      void *_obj;
  };

}

#endif // _SOUND_HPP

