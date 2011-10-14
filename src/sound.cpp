/////////////////////////////////////////////////////////////////////////////
// Name:        src/sound.cpp
// Purpose:     source for sound control
// Author:      Akiva Miura <akiva.miura@gmail.com>
// Modified by:
// Created:     05/13/2011
// Copyright:   (C) 2010-2011 Akiva Miura
// Licence:     MIT License
/////////////////////////////////////////////////////////////////////////////

#include "prec.h"

#include "lev/sound.hpp"
#include "lev/system.hpp"
#include "register.hpp"

#include <luabind/adopt_policy.hpp>
#include <luabind/luabind.hpp>
#include <map>


int luaopen_lev_sound(lua_State *L)
{
  using namespace luabind;
  using namespace lev;

  open(L);
  globals(L)["require"]("lev");

  module(L, "lev")
  [
    namespace_("sound"),
    namespace_("classes")
    [
      class_<sound, base>("sound")
        .def("clean", &sound::clean)
        .property("is_playing", &sound::is_playing, &sound::set_playing)
        .property("len", &sound::get_length)
        .property("length", &sound::get_length)
        .def("load", &sound::load)
        .def("open", &sound::open)
        .property("pan", &sound::get_pan, &sound::set_pan)
        .def("pause", &sound::pause)
        .def("play", &sound::load_and_play)
        .def("play", &sound::play)
        .property("pos", &sound::get_position, &sound::set_position)
        .property("position", &sound::get_position, &sound::set_position)
        .scope
        [
          def("create", &sound::create, adopt(result))
        ],
      class_<mixer, base>("mixer")
        .def("activate", &mixer::activate)
        .def("activate", &mixer::activate0)
        .def("clean_slot", &mixer::clean_slot)
        .property("channels", &mixer::get_channels)
        .property("freq", &mixer::get_freq)
        .property("frequency", &mixer::get_freq)
        .property("is_active", &mixer::is_active)
        .def("slot", &mixer::get_slot)
        .def("slot", &mixer::get_slot0)
        .def("start", &mixer::start)
        .def("stop", &mixer::stop)
        .property("type_id", &mixer::get_type_id)
        .property("type_name", &mixer::get_type_name)
        .scope
        [
          def("get", &mixer::get),
          def("init", &mixer::init)
        ]
    ]
  ];
  object classes = globals(L)["lev"]["classes"];
  object sound = globals(L)["lev"]["sound"];
  sound["create"] = classes["sound"]["create"];
  sound["mixer"] = classes["mixer"]["get"];

  globals(L)["package"]["loaded"]["lev.sound"] = sound;
  return 0;
}

extern "C" {
  #include "stb_vorbis.c"
}

namespace lev
{

  static SDL_AudioSpec& get_spec(class myMixer *mx);

  class audio_locker
  {
    public:
      audio_locker()
      {
//printf("LOCKING!\n");
        SDL_LockAudio();
      }

      ~audio_locker()
      {
//printf("UNLOCKING!\n");
        SDL_UnlockAudio();
      }
  };

  class mySound
  {
    private:

      mySound() : buf(NULL), pos(0), len(0), spec(), mx(NULL), playing(false)
      {
      }

    public:

      ~mySound()
      {
        Clean();
      }

      bool Clean()
      {
        if (mx && buf)
        {
          audio_locker lock;
          free(buf);
          buf = NULL;
          pos = 0;
          len = 0;
          playing = false;
          return true;
        }
        else if (buf)
        {
          free(buf);
          buf = NULL;
          pos = 0;
          len = 0;
          playing = false;
          return true;
        }
        return false;
      }

      static mySound *Create()
      {
        mySound *snd = NULL;
        try {
          snd = new mySound;
          return snd;
        }
        catch (...) {
          delete snd;
          return NULL;
        }
      }

      double GetLength()
      {
        if (len == 0) { return 0; }
        switch (spec.format)
        {
          case AUDIO_S8:
          case AUDIO_U8:
            return 1000 * (len / (double)(spec.freq * spec.channels));
          default:
            return 1000 * (len / (double)(spec.freq * spec.channels * 2));
        }
      }

      float GetPan()
      {
        return 0;
//        if (this->buf == NULL) { return 0; }
//        return playback->get_pan();
      }

      bool GetPlaying()
      {
        return playing;
      }

      double GetPosition()
      {
        if (pos == 0) { return 0; }
        switch (spec.format)
        {
          case AUDIO_S8:
          case AUDIO_U8:
            return 1000 * (pos / (double)(spec.freq * spec.channels));
          default:
            return 1000 * (pos / (double)(spec.freq * spec.channels * 2));
        }
      }

      bool LoadSample(const std::string &filename)
      {
        Clean();
        Uint8 *wav_buf;
        Uint32 wav_len;
        if (SDL_LoadWAV(filename.c_str(), &spec, &wav_buf, &wav_len) == NULL) { return false; }
        if (mx)
        {
          SDL_AudioCVT cvt;
          SDL_AudioSpec &audio = get_spec(mx);
          SDL_BuildAudioCVT(&cvt, spec.format,  spec.channels,  spec.freq,
                                  audio.format, audio.channels, audio.freq);
          cvt.buf = (Uint8 *)malloc(wav_len * cvt.len_mult);
          memcpy(cvt.buf, wav_buf, wav_len);
          cvt.len = wav_len;
          if (SDL_ConvertAudio(&cvt) < 0)
          {
            free(wav_buf);
            free(cvt.buf);
            return false;
          }
          free(wav_buf);

          audio_locker lock;
          buf = cvt.buf;
          len = cvt.len_cvt;
          spec = audio;
//printf("CONVERTED!\n");
        }
        return true;
      }

      bool OpenStream(const std::string &filename)
      {
        Clean();
        Uint8 *wav_buf;
        Uint32 wav_len;
        if (SDL_LoadWAV(filename.c_str(), &spec, &wav_buf, &wav_len) == NULL) { return false; }
        if (mx)
        {
          SDL_AudioCVT cvt;
          SDL_AudioSpec &audio = get_spec(mx);
          SDL_BuildAudioCVT(&cvt, spec.format,  spec.channels,  spec.freq,
                                  audio.format, audio.channels, audio.freq);
          cvt.buf = (Uint8 *)malloc(wav_len * cvt.len_mult);
          memcpy(cvt.buf, wav_buf, wav_len);
          cvt.len = wav_len;
          if (SDL_ConvertAudio(&cvt) < 0)
          {
            free(wav_buf);
            free(cvt.buf);
            return false;
          }
          free(wav_buf);

          audio_locker lock;
          buf = cvt.buf;
          len = cvt.len_cvt;
          spec = audio;
//printf("CONVERTED!\n");
        }
        return true;
      }

      bool Play(const std::string &filename)
      {
        OpenStream(filename);
        return SetPlaying(true);
      }

      bool SetPan(float pan)
      {
        return 0;
//        if (this->buf == NULL) { return false; }
//        playback->set_pan(pan);
//        return true;
      }

      bool SetPlaying(bool play)
      {
        if (buf == NULL) { return false; }
        if (mx)
        {
          playing = play;
          return true;
        }
        return false;
      }

      bool SetPosition(double msec)
      {
        if (mx && buf)
        {
          audio_locker lock;
          long new_pos = len * (msec / GetLength());
          if (new_pos < 0) { pos = len + new_pos; }
          else if (new_pos >= len) { pos = len; }
          else { pos = new_pos; }
          return true;
        }
        else if (buf)
        {
          long new_pos = len * (msec / GetLength());
          if (new_pos < 0) { pos = len + new_pos; }
          else if (new_pos >= len) { pos = len; }
          else { pos = new_pos; }
          return true;
        }
        return false;
      }

      SDL_AudioSpec spec;
      Uint8* buf;
      Uint32 len, pos;
      bool playing;
      class myMixer *mx;
  };

  static mySound *cast_snd(void *obj) { return (mySound *)obj; }

  sound::sound() : _obj(NULL) {}

  sound::~sound()
  {
    if (_obj) { delete (mySound *)_obj; }
  }

  sound* sound::create()
  {
    sound *snd = NULL;
    try {
      snd = new sound;
      snd->_obj = mySound::Create();
      return snd;
    }
    catch (...) {
      delete snd;
      return NULL;
    }
  }

  bool sound::clean() { return ((mySound *)_obj)->Clean(); }
  double sound::get_length() { return ((mySound *)_obj)->GetLength(); }
  float sound::get_pan() { return ((mySound *)_obj)->GetPan(); }
  double sound::get_position() { return cast_snd(_obj)->GetPosition(); }
  bool sound::is_playing() { return ((mySound *)_obj)->GetPlaying(); }

  bool sound::load(const std::string &filename) { return cast_snd(_obj)->LoadSample(filename); }
  bool sound::load_and_play(const std::string &filename) { return ((mySound *)_obj)->Play(filename); }
  bool sound::open(const std::string &filename) { return ((mySound *)_obj)->OpenStream(filename); }
  bool sound::set_pan(float pan) { return ((mySound *)_obj)->SetPan(pan); }
  bool sound::set_playing(bool play) { return ((mySound *)_obj)->SetPlaying(play); }
  bool sound::set_position(double pos) { return ((mySound *)_obj)->SetPosition(pos); }



  // Mixer Implementation

  // private mixing function (proto-type)
  void my_mix_audio(void *mixer, Uint8 *stream, int len);

  class myMixer
  {
    private:

      myMixer() : active(false), slots(), spec() {}
    public:

      ~myMixer()
      {
        std::map<int, sound *>::iterator i;
        for (i = slots.begin(); i != slots.end(); i++)
        {
          if (i->second != NULL) { i->second->clean(); }
        }
      }

      bool Activate(bool active)
      {
        if (active) { SDL_PauseAudio(0); }
        else { SDL_PauseAudio(1); }
        this->active = active;
        return true;
      }

      bool CleanSlot(int slot_num)
      {
        return false;
//        if (slot_num < 0) { return false; }
//        std::map<unsigned int, sound *>::iterator found;
//        found = slots.find(slot_num);
//        if (found != slots.end())
//        {
//          delete found->second;
//          //found->second = NULL;
//          channels.erase(found);
//          return true;
//        }
//        else { return false; }
      }

      static myMixer *Create()
      {
        myMixer *mx = NULL;
        SDL_AudioSpec request;
        try {
          mx = new myMixer;
          request.freq     = 44100;
          request.format   = AUDIO_S16;
          request.channels = 2;
          request.samples  = 512;
          request.callback = my_mix_audio;
          request.userdata = mx;
          if (system::init() == NULL) { throw -1; }
          if (SDL_OpenAudio(&request, &mx->spec) < 0) { throw -2; }
          SDL_PauseAudio(0);
          mx->active = true;
          return mx;
        }
        catch (...) {
          delete mx;
          return NULL;
        }
      }

      sound *CreateSlot(int slot_num)
      {
        sound *slot = sound::create();
        if (slot == NULL) { return NULL; }
        audio_locker lock;
        slots[slot_num] = slot;
        cast_snd(slot->get_rawobj())->mx = this;
        return slot;
      }

      sound *GetSlot(int slot_num)
      {
        if (slot_num == 0)
        {
          int i;
          for (i = -1; ; i--)
          {
            std::map<int, sound *>::iterator found;
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
        return active;
      }

    public:
      bool active;
      SDL_AudioSpec spec;
      std::map<int, sound *> slots;
  };

  static myMixer *cast_mx(void *obj) { return (myMixer *)obj; }

  SDL_AudioSpec& get_spec(class myMixer *mx)
  {
    return mx->spec;
  }

  void my_mix_audio(void *udata, Uint8 *stream, int len)
  {
    myMixer *mx = cast_mx(udata);

    std::map<int, sound *>::iterator i;
    for (i = mx->slots.begin(); i != mx->slots.end(); i++)
    {
      if (! i->second) { continue; }
      else
      {
        mySound *snd = cast_snd(i->second->get_rawobj());
        int seek_len = snd->len - snd->pos;
        if (! snd->playing) { continue; }
        if (seek_len > len) { seek_len = len; }
        if (seek_len > 0)
        {
          SDL_MixAudio(stream, &snd->buf[snd->pos], seek_len, SDL_MIX_MAXVOLUME);
          snd->pos += seek_len;
        }
      }
    }
  }

  static SDL_AudioSpec& get_spec(mixer *mx)
  {
    return cast_mx(mx->get_rawobj())->spec;
  }


  mixer::mixer() : base(), _obj(NULL) { }

  mixer::~mixer()
  {
    if (_obj) { delete (myMixer *)_obj; }
//printf("CLOSING1\n");
//    SDL_QuitSubSystem(SDL_INIT_AUDIO);
//    SDL_CloseAudio();
//printf("CLOSING2\n");
  }

  bool mixer::activate(bool active)
  {
    return cast_mx(_obj)->Activate(active);
  }

  bool mixer::clean_slot(int slot_num)
  {
    return ((myMixer *)_obj)->CleanSlot(slot_num);
  }

  int mixer::get_channels()
  {
    return cast_mx(_obj)->spec.channels;
  }

  int mixer::get_freq()
  {
    return cast_mx(_obj)->spec.freq;
  }

  sound* mixer::get_slot(int slot_num)
  {
    return ((myMixer *)_obj)->GetSlot(slot_num);
  }

  mixer* mixer::init()
  {
    static lev::mixer mx;
    if (mx._obj) { return &mx; }
    mx._obj = myMixer::Create();
    return &mx;
  }

  bool mixer::is_active()
  {
    return cast_mx(_obj)->IsActive();
  }

}

