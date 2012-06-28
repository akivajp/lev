/////////////////////////////////////////////////////////////////////////////
// Name:        src/sound.cpp
// Purpose:     source for sound control
// Author:      Akiva Miura <akiva.miura@gmail.com>
// Modified by:
// Created:     05/13/2011
// Copyright:   (C) 2010-2011 Akiva Miura
// Licence:     MIT License
/////////////////////////////////////////////////////////////////////////////

// pre-compiled header
#include "prec.h"

// declarations
#include "lev/sound.hpp"

// dependencies
#include "lev/debug.hpp"
#include "lev/entry.hpp"
#include "lev/fs.hpp"
#include "lev/system.hpp"

// libraries
#include <allegro5/allegro_audio.h>
#include <boost/shared_array.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <luabind/luabind.hpp>
#include <map>

// static member variable initialization
//boost::shared_ptr<lev::mixer> lev::mixer::singleton;

int luaopen_lev_sound(lua_State *L)
{
  using namespace luabind;
  using namespace lev;

  open(L);
  globals(L)["package"]["loaded"]["lev.sound"] = true;
  globals(L)["require"]("lev");
  globals(L)["require"]("lev.system");

  module(L, "lev")
  [
    namespace_("sound"),
    namespace_("classes")
    [
      class_<sound, base, boost::shared_ptr<base> >("sound")
        .def("clear", &sound::clear)
        .property("gain", &sound::get_volume, &sound::set_volume)
        .property("id", &sound::get_id)
        .property("is_playing", &sound::is_playing, &sound::set_playing)
        .property("len", &sound::get_length)
        .property("length", &sound::get_length)
        .def("load", &sound::load)
        .def("load", &sound::load_path)
        .def("open", &sound::open)
        .def("open", &sound::open_path)
        .property("pan", &sound::get_pan, &sound::set_pan)
        .def("pause", &sound::pause)
        .def("play", &sound::load_and_play)
        .def("play", &sound::load_and_play1)
        .def("play", &sound::load_and_play_path)
        .def("play", &sound::play)
        .def("play", &sound::play0)
        .property("pos", &sound::get_position, &sound::set_position)
        .property("position", &sound::get_position, &sound::set_position)
        .property("vol", &sound::get_volume, &sound::set_volume)
        .property("volume", &sound::get_volume, &sound::set_volume),
      class_<mixer, base, boost::shared_ptr<base> >("mixer")
        .def("activate", &mixer::activate)
        .def("activate", &mixer::activate0)
        .def("clear_slot", &mixer::clear_slot)
        .property("channels", &mixer::get_channels)
        .property("freq", &mixer::get_freq)
        .property("frequency", &mixer::get_freq)
        .property("is_active", &mixer::is_active)
        .def("slot", &mixer::get_slot)
        .def("slot", &mixer::get_slot0)
        .def("start", &mixer::start)
        .def("stop", &mixer::stop)
    ]
  ];
//  object classes = globals(L)["lev"]["classes"];
//  object sound = globals(L)["lev"]["sound"];

  globals(L)["package"]["loaded"]["lev.sound"] = true;
  return 0;
}

namespace lev
{

  class mySound
  {
    private:

      mySound()
        : loop(false),
          id(0),
          gain(0),
          mx(NULL),
          sample(NULL),
          instance(NULL),
          stream(NULL)
      { }

    public:

      ~mySound()
      {
        Clear();
      }

      bool Clear()
      {
        if (system::get_interpreter())
        {
          if (instance)
          {
            al_destroy_sample_instance(instance);
            instance = NULL;
          }
          if (sample)
          {
            al_destroy_sample(sample);
            sample = NULL;
          }
          if (stream)
          {
            al_destroy_audio_stream(stream);
            stream = NULL;
          }
        }
        gain = 0;
        return true;
      }

      static mySound *Create()
      {
        mySound *snd = NULL;
        try {
          snd = new mySound();
          if (! snd) { throw -1; }
          return snd;
        }
        catch (...) {
          delete snd;
          return NULL;
        }
      }

      double GetLength()
      {
        if (instance) { return al_get_sample_instance_time(instance); }
        if (stream) { return al_get_audio_stream_length_secs(stream); }
        return 0;
      }

      float GetPan()
      {
        if (instance) { return al_get_sample_instance_pan(instance); }
        if (stream) { return al_get_audio_stream_pan(stream); }
        return 0;
      }

      bool GetPlaying()
      {
        if (instance) { return al_get_sample_instance_playing(instance); }
        if (stream) { return al_get_audio_stream_playing(stream); }
        return false;
      }

      double GetPosition()
      {
        if (instance)
        {
          unsigned int len = al_get_sample_instance_length(instance);
          unsigned int pos = al_get_sample_instance_position(instance);
          float time = al_get_sample_instance_time(instance);
          return time * pos / len;
        }
        if (stream) { return al_get_audio_stream_position_secs(stream); }
        return 0;
      }

      float GetVolume()
      {
        return gain;
//        if (instance) { al_get_sample_instance_gain(instance); }
//        if (stream) { al_get_audio_stream_gain(stream); }
//        return 0;
      }

      bool LoadSample(const std::string &filename)
      {
        Clear();
        try {
          sample = al_load_sample(filename.c_str());
          if (! sample) { throw -1; }
          instance = al_create_sample_instance(sample);
          if (! instance) { throw -2; }
          if (! al_attach_sample_instance_to_mixer(instance, mx)) { throw -3; }
          gain = 1;
          return true;
        }
        catch (...) {
          Clear();
          return false;
        }
      }

      bool OpenStream(const std::string &filename)
      {
        Clear();
        try {
          stream = al_load_audio_stream(filename.c_str(), 4, 2024);
          if (! stream) { throw -1; }
          if (! al_attach_audio_stream_to_mixer(stream, mx)) { throw -2; }
          gain = 1;
          return true;
        }
        catch (...) {
          Clear();
          return false;
        }
      }

      bool Play(const std::string &filename, bool repeat)
      {
        OpenStream(filename);
        return SetPlaying(true, repeat);
      }

      bool SetPan(float pan)
      {
        if (instance) { return al_set_sample_instance_pan(instance, pan); }
        if (stream) { return al_set_audio_stream_pan(stream, pan); }
        return false;
      }

      bool SetPlaying(bool play, bool repeat)
      {
        ALLEGRO_PLAYMODE mode = ALLEGRO_PLAYMODE_ONCE;
        if (repeat) { mode = ALLEGRO_PLAYMODE_LOOP; }

        if (mx)
        {
          if (instance)
          {
            if (! al_set_sample_instance_playing(instance, play)) { return false; }
            return al_set_sample_instance_playmode(instance, mode);
          }
          if (stream)
          {
            if (! al_set_audio_stream_playing(stream, play)) { return false; }
            return al_set_audio_stream_playmode(stream, mode);
          }
        }
        return false;
      }

      bool SetPosition(double s)
      {
        if (instance)
        {
          unsigned int len = al_get_sample_instance_length(instance);
          float time = al_get_sample_instance_time(instance);
          return al_set_sample_instance_position(instance, len * s / time);
        }
        if (stream) { return al_seek_audio_stream_secs(stream, s); }
        return false;
      }

      bool SetVolume(double vol)
      {
        if (instance)
        {
          if (! al_set_sample_instance_gain(instance, vol)) { return false; }
          gain = vol;
          return true;
        }
        if (stream)
        {
          if (! al_set_audio_stream_gain(stream, vol)) { return false; }
          gain = vol;
          return true;
        }
        return false;
      }

      ALLEGRO_SAMPLE *sample;
      ALLEGRO_SAMPLE_INSTANCE *instance;
      ALLEGRO_AUDIO_STREAM *stream;
      ALLEGRO_MIXER *mx;
      float gain;
      bool loop;
      int id;
  };

  static mySound *cast_snd(void *obj) { return (mySound *)obj; }

  sound::sound() : _obj(NULL) {}

  sound::~sound()
  {
    if (_obj) { delete (mySound *)_obj; }
  }

  boost::shared_ptr<sound> sound::create()
  {
    boost::shared_ptr<sound> snd;
    try {
      snd.reset(new sound);
      if (! snd) { throw -1; }
      snd->_obj = mySound::Create();
      if (! snd->_obj) { throw -2; }
    }
    catch (...) {
      snd.reset();
      lev::debug_print("error on sound instance creation");
    }
    return snd;
  }

  bool sound::clear() { return cast_snd(_obj)->Clear(); }
  int sound::get_id() const { return cast_snd(_obj)->id; }
  double sound::get_length() { return cast_snd(_obj)->GetLength(); }
  float sound::get_pan() { return cast_snd(_obj)->GetPan(); }
  double sound::get_position() { return cast_snd(_obj)->GetPosition(); }
  float sound::get_volume() { return cast_snd(_obj)->GetVolume(); }
  bool sound::is_playing() { return ((mySound *)_obj)->GetPlaying(); }

  bool sound::load(const std::string &filename)
  {
    return cast_snd(_obj)->LoadSample(filename);
  }
  bool sound::load_path(boost::shared_ptr<path> p)
  {
    if (! p) { return false; }
    return cast_snd(_obj)->LoadSample(p->to_str());
  }
  bool sound::load_and_play(const std::string &filename, bool repeat)
  {
    return ((mySound *)_obj)->Play(filename, repeat);
  }
  bool sound::load_and_play_path(boost::shared_ptr<path> p)
  {
    if (! p) { return false; }
    return ((mySound *)_obj)->Play(p->to_str(), false);
  }

  bool sound::open(const std::string &filename)
  {
    return cast_snd(_obj)->OpenStream(filename);
  }
  bool sound::open_path(boost::shared_ptr<path> p)
  {
    if (! p) { return false; }
    return cast_snd(_obj)->OpenStream(p->to_str());
  }
  bool sound::set_pan(float pan) { return ((mySound *)_obj)->SetPan(pan); }
  bool sound::set_playing(bool play, bool repeat)
  {
    return ((mySound *)_obj)->SetPlaying(play, repeat);
  }

  bool sound::set_position(double pos) { return cast_snd(_obj)->SetPosition(pos); }
  bool sound::set_volume(double vol) { return cast_snd(_obj)->SetVolume(vol); }

  // Mixer Implementation

  class myMixer
  {
    private:
      myMixer() :
        active(false),
        mixer(NULL),
        slots(),
        voice(NULL)
      {}

    public:

      ~myMixer()
      {
        std::map<int, boost::shared_ptr<sound> >::iterator i;
        for (i = slots.begin(); i != slots.end(); i++)
        {
          if (i->second != NULL) { i->second->clear(); }
        }
        if (system::get_interpreter())
        {
//printf("DETACH MIXER\n");
//          al_detach_mixer(mixer);
//printf("DESTROY MIXER\n");
          al_destroy_mixer(mixer);
          mixer = NULL;
//printf("DESTROY VOICE\n");
          al_destroy_voice(voice);
          voice = NULL;
        }
//printf("DESTROYED MYMIXER\n");
      }

      bool Activate(bool active)
      {
        return al_set_mixer_playing(mixer, active);
      }

      bool ClearSlot(int slot_num)
      {
        std::map<int, boost::shared_ptr<sound> >::iterator found;
        found = slots.find(slot_num);
        if (found != slots.end())
        {
          slots.erase(found);
          return true;
        }
        else { return false; }
      }

      static myMixer *Create()
      {
        myMixer *mx = NULL;
        try {
          mx = new myMixer;
          if (! mx) { throw -1; }
          mx->voice = al_create_voice(44100, ALLEGRO_AUDIO_DEPTH_INT16, ALLEGRO_CHANNEL_CONF_2);
          if (! mx->voice) { throw -2; }
          mx->mixer = al_create_mixer(44100, ALLEGRO_AUDIO_DEPTH_FLOAT32, ALLEGRO_CHANNEL_CONF_2);
          if (! al_attach_mixer_to_voice(mx->mixer, mx->voice)) { throw -3; }
          return mx;
        }
        catch (...) {
          delete mx;
          return NULL;
        }
      }

      boost::shared_ptr<sound> CreateSlot(int slot_num)
      {
        boost::shared_ptr<sound> slot;
        try {
          slot = sound::create();
          if (! slot) { throw -1; }
          slots[slot_num] = slot;
          cast_snd(slot->get_rawobj())->mx = mixer;
          cast_snd(slot->get_rawobj())->id = slot_num;
        }
        catch (...) {
          slot.reset();
        }
        return slot;
      }

      boost::shared_ptr<sound> GetSlot(int slot_num)
      {
        if (slot_num == 0)
        {
          // auto finding a non-used slot
          int i;
          for (i = -1; ; i--)
          {
            std::map<int, boost::shared_ptr<sound> >::iterator found;
            found = slots.find(i);
            if (found == slots.end())
            {
              // slots[i] is not found
              return CreateSlot(i);
            }
            else if (slots[i] == NULL)
            {
              // cannnels[i] is NULL
              return CreateSlot(i);
            }
            else if (! slots[i]->is_playing())
            {
              // channels[i] is not playing, therefore ready for another use
              return slots[i];
            }
            else
            {
              // slots[i] is found
              continue;
            }
          }
        }
        else // if (slot_num != 0)
        {
          if (slots[slot_num] == NULL)
          {
            // slots[i] is NULL
            return CreateSlot(slot_num);
          }
          else { return slots[slot_num]; }
        }
      }

      bool IsActive()
      {
        return al_get_mixer_playing(mixer);
      }

    public:
      bool active;
      ALLEGRO_VOICE *voice;
      ALLEGRO_MIXER *mixer;
//      std::map<int, sound *> slots;
      std::map<int, boost::shared_ptr<sound> > slots;
  };
  static myMixer *cast_mixer(void *obj) { return (myMixer *)obj; }

  mixer::mixer() : base(), _obj(NULL) { }

  mixer::~mixer()
  {
    if (_obj)
    {
      delete cast_mixer(_obj);
      _obj = NULL;
    }
  }

  bool mixer::activate(bool active)
  {
    return cast_mixer(_obj)->Activate(active);
  }

  bool mixer::clear_slot(int slot_num)
  {
    return cast_mixer(_obj)->ClearSlot(slot_num);
  }

  boost::shared_ptr<mixer> mixer::create()
  {
    boost::shared_ptr<mixer> mx;
    try {
      mx.reset(new mixer);
      if (! mx) { throw -1; }
      mx->_obj = myMixer::Create();
      if (! mx->_obj) { throw -2; }
    }
    catch (...) {
      mx.reset();
      lev::debug_print("error on mixer instance creation");
    }
    return mx;
  }

  int mixer::get_channels()
  {
    return al_get_mixer_channels(cast_mixer(_obj)->mixer);
  }

  int mixer::get_freq()
  {
    return al_get_mixer_frequency(cast_mixer(_obj)->mixer);
  }

  boost::shared_ptr<sound> mixer::get_slot(int slot_num)
  {
    return cast_mixer(_obj)->GetSlot(slot_num);
  }

  bool mixer::is_active()
  {
    return cast_mixer(_obj)->IsActive();
  }

}

