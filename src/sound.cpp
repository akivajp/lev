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
#include "lev/fs.hpp"
#include "lev/system.hpp"
#include "register.hpp"

// libraries
#include <boost/shared_array.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <luabind/luabind.hpp>
#include <map>
#include <vorbis/vorbisfile.h>

// static member variable initialization
boost::shared_ptr<lev::mixer> lev::mixer::singleton;

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
        .def("play", &sound::load_and_play_path1)
        .def("play", &sound::play)
        .def("play", &sound::play0)
        .property("pos", &sound::get_position, &sound::set_position)
        .property("position", &sound::get_position, &sound::set_position),
//        .scope
//        [
//          def("create", &sound::create)
//        ],
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
        .property("type_id", &mixer::get_type_id)
        .property("type_name", &mixer::get_type_name)
        .scope
        [
          def("get", &mixer::get),
          def("init", &mixer::init)
        ]
    ]
  ];
//  object classes = globals(L)["lev"]["classes"];
//  object sound = globals(L)["lev"]["sound"];
//  sound["create"] = classes["sound"]["create"];
//  sound["mixer"] = classes["mixer"]["get"];

  globals(L)["package"]["loaded"]["lev.sound"] = true;
  return 0;
}

namespace lev
{

//static void test(const char *str, SDL_AudioSpec &t)
//{
//  printf("%s\n", str);
//  printf("freq: %d\n", t.freq);
//  printf("format: %d\n", t.format);
//  printf("channels: %d\n", t.channels);
//  printf("samples: %d\n", t.samples);
//}

  static SDL_AudioSpec& get_spec(class myMixer *mx);

  class audio_locker
  {
    public:
      audio_locker(class myMixer *mx) : mx(mx)
      {
        if (mx)
        {
//printf("LOCKING!\n");
          SDL_LockAudio();
        }
      }

      ~audio_locker()
      {
        if (mx)
        {
//printf("UNLOCKING!\n");
          SDL_UnlockAudio();
        }
      }

      class myMixer *mx;
  };

  class mySoundLoader
  {
    public:
      mySoundLoader() { }
      virtual ~mySoundLoader() { }
      virtual int Decode(const SDL_AudioSpec *spec, Uint8 *buf, Uint32 len) { }
      virtual unsigned long GetLength() { }
      virtual bool LoadAll(const SDL_AudioSpec *spec, Uint8 **buf, Uint32 *len) { }
      virtual bool Seek(double s = 0) { }
  };

  class myVorbisLoader : public mySoundLoader
  {
    protected:
      myVorbisLoader() { }

    public:
      virtual ~myVorbisLoader()
      {
        if (vf)
        {
          ov_clear(vf);
          free(vf);
          vf = NULL;
        }
      }

      virtual int Decode(const SDL_AudioSpec *spec, Uint8 *buf, Uint32 len)
      {
        if (! spec) { return -1; }

        int count;
        int endian = 0;
        int sign = 0;
        int word = 1;
        int pos = 0;

        switch (spec->format)
        {
          case AUDIO_S8:
            sign = 1;
            // word = 1;
            break;
          case AUDIO_U8:
            // sign = 0;
            // word = 1;
            break;
          case AUDIO_S16:
            sign = 1;
            word = 2;
            // endian = 0;
            break;
          case AUDIO_S16MSB:
            sign = 1;
            word = 2;
            endian = 1;
            break;
          case AUDIO_U16:
            sign = 0;
            word = 2;
            // endian = 0;
            break;
          case AUDIO_U16MSB:
            sign = 0;
            word = 2;
            endian = 1;
            break;
          default:
            return -1;
        }

        do {
          int current;
          count = ov_read(vf, (char *)buf + pos, len - pos,
                          endian, word, sign, &current);
          pos += count;
        } while (count > 0);

        return pos;
      }

      virtual unsigned long GetLength()
      {
        return vf->vi->channels * 2 * ov_pcm_total(vf, -1);
      }

      virtual bool LoadAll(const SDL_AudioSpec *spec, Uint8 **buf, Uint32 *len)
      {
//        vorbis_info *vi = NULL;
        int count, pos = 0;

        if (!buf && !len) { return NULL; }
        try {
          *len = GetLength();
          *buf = (Uint8 *)malloc(*len);
          if (! *buf) { throw -1; }

//          spec->channels = vi->channels;
//          spec->format = AUDIO_S16;
//          spec->freq = vi->rate;
//          spec->samples = 4096;
//          spec->size = *len;

          if (! Decode(spec, *buf, *len)) { throw -2; }
          return true;
        }
        catch (...) {
          free(*buf);
          return false;
        }
      }

      static myVorbisLoader *Open(const std::string &file)
      {
        myVorbisLoader *l = NULL;
        try {
          l = new myVorbisLoader;
          l->vf = (OggVorbis_File *)malloc(sizeof(OggVorbis_File));
          if (! l->vf) { throw -1; }
          // opening ogg file, checking validity
          if (ov_fopen((char *)file.c_str(), l->vf) != 0) { throw -2; }
          return l;
        }
        catch (...) {
          delete l;
          return NULL;
        }
      }

      virtual bool Seek(double s = 0)
      {
        if (ov_time_seek(vf, s) == 0) { return true; }
        return false;
      }

      OggVorbis_File *vf;
  };

  class mySound
  {
    private:

      mySound()
        : buf(NULL), pos(0), len(0), loader(NULL), loop(false),
          spec(), mx(NULL), playing(false)
      { }

    public:

      ~mySound()
      {
        Clear();
      }

      bool Clear()
      {
        audio_locker lock(mx);
        if (buf)
        {
          free(buf);
          buf = NULL;
        }
        if (loader)
        {
          delete loader;
          loader = NULL;
        }
        pos = 0;
        len = 0;
        loop = false;
        playing = false;
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
        if (len == 0) { return 0; }
        switch (spec.format)
        {
          case AUDIO_S8:
          case AUDIO_U8:
            return len / (double)(spec.freq * spec.channels);
          default:
            return len / (double)(spec.freq * spec.channels * 2);
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
            return pos / (double)(spec.freq * spec.channels);
          default:
            return pos / (double)(spec.freq * spec.channels * 2);
        }
      }

      bool LoadSample(const std::string &filename)
      {
        Clear();
        Uint8 *wav_buf;
        Uint32 wav_len;

        // trying to load WAV
        if (SDL_LoadWAV(filename.c_str(), &spec, &wav_buf, &wav_len) != NULL)
        {
          // success to load WAV
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

            audio_locker lock(mx);
            buf = cvt.buf;
            len = cvt.len_cvt;
            spec = audio;
          }
        }
        else if (mx)
        {
          // trying to load Vorbis
          boost::shared_ptr<mySoundLoader> l(myVorbisLoader::Open(filename));
          if (! l) { return false; }
          else if (! l->LoadAll(&get_spec(mx), &wav_buf, &wav_len)) { return false; }
          // success to load Vorbis
          audio_locker lock(mx);
          buf = wav_buf;
          len = wav_len;
          spec = get_spec(mx);
        }

        return true;
      }

      bool OpenStream(const std::string &filename)
      {
        Clear();
        Uint8 *wav_buf;
        Uint32 wav_len;

        // trying to load WAV
        if (SDL_LoadWAV(filename.c_str(), &spec, &wav_buf, &wav_len) != NULL)
        {
          // success to load WAV
          if (mx)
          {
            SDL_AudioCVT cvt;
            SDL_AudioSpec &audio = get_spec(mx);
//test("WAV SPEC", spec);
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

            audio_locker lock(mx);
            buf = cvt.buf;
            len = cvt.len_cvt;
            spec = audio;
          }
        }
        else if (mx)
        {
          audio_locker lock(mx);
          // trying to load Vorbis
          loader = myVorbisLoader::Open(filename);
          if (! loader) { return false; }
          // success to load Vorbis
          len = loader->GetLength();
          spec = get_spec(mx);
        }

        return true;
      }

      bool Play(const std::string &filename, bool repeat)
      {
        OpenStream(filename);
        return SetPlaying(true, repeat);
      }

      bool SetPan(float pan)
      {
        return 0;
//        if (this->buf == NULL) { return false; }
//        playback->set_pan(pan);
//        return true;
      }

      bool SetPlaying(bool play, bool repeat)
      {
        if (mx)
        {
          playing = play;
          loop = repeat;
          return true;
        }
        return false;
      }

      bool SetPosition(double s)
      {
        audio_locker locker(mx);
        if (len > 0)
        {
          long new_pos = len * (s / GetLength());
          if (new_pos < 0)
          {
            pos = len + new_pos;
            if (pos < 0) { pos = 0; }
          }
          else if (new_pos >= len) { pos = len; }
          else { pos = new_pos; }

          if (loader)
          {
            if (s >= 0)
            {
              loader->Seek(s);
            }
            else
            {
              loader->Seek(GetLength() + s);
            }
          }
          return true;
        }
        return false;
      }

      SDL_AudioSpec spec;
      Uint8* buf;
      Uint32 len, pos;
      bool loop;
      bool playing;
      class myMixer *mx;
      mySoundLoader *loader;
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
      fprintf(stderr, "error on sound instance creation\n");
    }
    return snd;
  }

  bool sound::clear() { return ((mySound *)_obj)->Clear(); }
  double sound::get_length() { return ((mySound *)_obj)->GetLength(); }
  float sound::get_pan() { return ((mySound *)_obj)->GetPan(); }
  double sound::get_position() { return cast_snd(_obj)->GetPosition(); }
  bool sound::is_playing() { return ((mySound *)_obj)->GetPlaying(); }

  bool sound::load(const std::string &filename)
  {
    return cast_snd(_obj)->LoadSample(filename);
  }
  bool sound::load_path(boost::shared_ptr<file_path> path)
  {
    return load(path->get_full_path());
  }
  bool sound::load_and_play(const std::string &filename, bool repeat)
  {
    return ((mySound *)_obj)->Play(filename, repeat);
  }
  bool sound::load_and_play_path(boost::shared_ptr<file_path> path, bool repeat)
  {
    return load_and_play(path->get_full_path(), repeat);
  }

  bool sound::open(const std::string &filename)
  {
    return cast_snd(_obj)->OpenStream(filename);
  }
  bool sound::open_path(boost::shared_ptr<file_path> path)
  {
    return open(path->get_full_path());
  }
  bool sound::set_pan(float pan) { return ((mySound *)_obj)->SetPan(pan); }
  bool sound::set_playing(bool play, bool repeat)
  {
    return ((mySound *)_obj)->SetPlaying(play, repeat);
  }

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
        std::map<int, boost::shared_ptr<sound> >::iterator i;
        for (i = slots.begin(); i != slots.end(); i++)
        {
          if (i->second != NULL) { i->second->clear(); }
        }
      }

      bool Activate(bool active)
      {
        if (active) { SDL_PauseAudio(0); }
        else { SDL_PauseAudio(1); }
        this->active = active;
        return true;
      }

      bool ClearSlot(int slot_num)
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
//          if (system::init() == NULL) { throw -1; }
          if (SDL_OpenAudio(&request, &mx->spec) < 0) { throw -2; }
//test("REQUEST", request);
//test("ACCEPT",  mx->spec);
          SDL_PauseAudio(0);
          mx->active = true;
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
          audio_locker lock(this);
          slots[slot_num] = slot;
          cast_snd(slot->get_rawobj())->mx = this;
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
        return active;
      }

    public:
      bool active;
      SDL_AudioSpec spec;
//      std::map<int, sound *> slots;
      std::map<int, boost::shared_ptr<sound> > slots;
  };

  static myMixer *cast_mx(void *obj) { return (myMixer *)obj; }

  SDL_AudioSpec& get_spec(class myMixer *mx)
  {
    return mx->spec;
  }

  void my_mix_audio(void *udata, Uint8 *stream, int len)
  {
    myMixer *mx = cast_mx(udata);

    std::map<int, boost::shared_ptr<sound> >::iterator i;
    memset(stream, 0, len);
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
          if (snd->buf)
          {
            SDL_MixAudio(stream, &snd->buf[snd->pos], seek_len, SDL_MIX_MAXVOLUME);
          }
          else if (snd->loader)
          {
            boost::shared_array<Uint8> buf(new Uint8[seek_len]);
            snd->loader->Decode(&mx->spec, buf.get(), seek_len);
            SDL_MixAudio(stream, buf.get(), seek_len, SDL_MIX_MAXVOLUME);
          }
          snd->pos += seek_len;
        }
        else // if (seek_len <= 0)
        {
          if (snd->loop)
          {
            snd->SetPosition(0);
          }
          else
          {
            snd->playing = false;
          }
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
//printf("CLOSING AUDIO!\n");
    SDL_CloseAudio();
//printf("CLOSED AUDIO!\n");
  }

  bool mixer::activate(bool active)
  {
    return cast_mx(_obj)->Activate(active);
  }

  bool mixer::clear_slot(int slot_num)
  {
    return ((myMixer *)_obj)->ClearSlot(slot_num);
  }

  int mixer::get_channels()
  {
    return cast_mx(_obj)->spec.channels;
  }

  int mixer::get_freq()
  {
    return cast_mx(_obj)->spec.freq;
  }

  boost::shared_ptr<sound> mixer::get_slot(int slot_num)
  {
    return cast_mx(_obj)->GetSlot(slot_num);
  }

  boost::shared_ptr<mixer> mixer::init(boost::shared_ptr<system> sys)
  {
    if (! sys) { return boost::shared_ptr<mixer>(); }
    if (singleton) { return singleton; }
    try {
      singleton.reset(new mixer);
      if (! singleton) { throw -1; }
      singleton->_obj = myMixer::Create();
      if (! singleton->_obj) { throw -2; }
    }
    catch (...) {
      singleton.reset();
      fprintf(stderr, "error on mixer instance creation\n");
    }
    return singleton;
  }

  bool mixer::is_active()
  {
    return cast_mx(_obj)->IsActive();
  }

}

