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
#include <boost/shared_array.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <luabind/luabind.hpp>
#include <map>
#include <SDL2/SDL.h>
#include <vorbis/vorbisfile.h>

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
    namespace_("classes")
    [
      class_<sound, base, boost::shared_ptr<base> >("sound")
        .def("clear", &sound::clear)
        .property("is_playing", &sound::is_playing, &sound::set_playing)
        .property("len", &sound::get_length)
        .property("length", &sound::get_length)
        .def("open", &sound::open)
        .def("open", &sound::open_file)
        .def("open", &sound::open_path)
        .property("pan", &sound::get_pan, &sound::set_pan)
        .def("pause", &sound::pause)
        .def("play", &sound::open_and_play)
        .def("play", &sound::open_and_play1)
        .def("play", &sound::open_and_play_file)
        .def("play", &sound::open_and_play_file1)
        .def("play", &sound::play)
        .def("play", &sound::play0)
        .property("pos", &sound::get_position, &sound::set_position)
        .property("position", &sound::get_position, &sound::set_position)
        .property("vol", &sound::get_volume, &sound::set_volume)
        .property("volume", &sound::get_volume, &sound::set_volume),
      class_<mixer, base, base::ptr>("mixer")
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
        .scope
        [
          def("get", &mixer::get),
          def("init", &mixer::init)
        ]
    ]
  ];
  object lev = globals(L)["lev"];
  object classes = lev["classes"];
//  object sound = globals(L)["lev"]["sound"];
//  sound["create"] = classes["sound"]["create"];
//  sound["mixer"] = classes["mixer"]["get"];

  lev["mixer"] = classes["mixer"]["init"];

  globals(L)["package"]["loaded"]["lev.sound"] = true;
  return 0;
}

namespace lev
{

  class audio_locker
  {
    public:
      audio_locker()
      {
        SDL_LockAudio();
      }

      ~audio_locker()
      {
        SDL_UnlockAudio();
      }
  };

  class sound_loader
  {
    public:
      typedef boost::shared_ptr<sound_loader> ptr;
    protected:
      sound_loader(SDL_AudioSpec *spec) :
        spec(spec)
      { }
    public:
      virtual ~sound_loader() { }
      virtual int load_samples(Uint8 *buf, Uint32 len) = 0;
      virtual double get_length() = 0;
      virtual double get_position() = 0;

      static int get_word_size(const SDL_AudioSpec *spec)
      {
        switch (spec->format)
        {
          case AUDIO_S8:
          case AUDIO_U8:
            return 1;
          case AUDIO_S16LSB:
          case AUDIO_S16MSB:
          case AUDIO_U16LSB:
          case AUDIO_U16MSB:
            return 2;
          case AUDIO_S32LSB:
          case AUDIO_S32MSB:
          case AUDIO_F32LSB:
          case AUDIO_F32MSB:
            return 4;
          default:
            return 0;
        }
      }

      static bool is_big_endian(const SDL_AudioSpec *spec)
      {
        switch (spec->format)
        {
          case AUDIO_S16MSB:
          case AUDIO_S32MSB:
          case AUDIO_F32MSB:
            return true;
          default:
            return false;
        }
      }

      static bool is_signed(const SDL_AudioSpec *spec)
      {
        switch (spec->format)
        {
          case AUDIO_S8:
          case AUDIO_S16LSB:
          case AUDIO_S16MSB:
          case AUDIO_S32LSB:
          case AUDIO_S32MSB:
          case AUDIO_F32LSB:
          case AUDIO_F32MSB:
            return true;
          default:
            return false;
        }
      }

      virtual bool set_position(double seconds = 0) = 0;

      SDL_AudioSpec *spec;
  };

  class wav_loader : public sound_loader
  {
    public:
      typedef boost::shared_ptr<wav_loader> ptr;
    protected:
      wav_loader(SDL_AudioSpec *spec) :
        sound_loader(spec),
        data(NULL), data_pos(0)
      { }
    public:
      virtual ~wav_loader()
      {
        if (data)
        {
          free(data);
          data = NULL;
        }
      }

      virtual double get_length()
      {
        return data_len / double(spec->freq * spec->channels * get_word_size(spec));
      }

      virtual double get_position()
      {
        return get_length() * data_pos / data_len;
      }

      virtual int load_samples(Uint8 *buf, Uint32 len)
      {
        if (data_pos + len > data_len)
        {
          len = data_len - data_pos;
        }
        memcpy(buf, data + data_pos, len);
        data_pos += len;
        return len;
      }

      virtual bool set_position(double seconds = 0)
      {
        long new_pos = data_len * seconds / get_length();
        if (new_pos < 0)
        {
          new_pos = data_len + new_pos;
          if (new_pos < 0) { new_pos = 0; }
        }
        else if (new_pos > data_len) { new_pos = data_len; }
        data_pos = new_pos;
        return true;
      }

      static wav_loader::ptr open(file::ptr src, SDL_AudioSpec *spec)
      {
        wav_loader::ptr ld;
        if (! src) { return ld; }
        if (! spec) { return ld; }
        SDL_RWops *ops = (SDL_RWops *)src->get_ops();
        Uint32 wav_len;
        Uint8 *wav_buf = NULL;
        SDL_AudioSpec wav_spec;
        SDL_AudioCVT cvt;
        try {
          src->seek(0);
          ld.reset(new wav_loader(spec));
          if (! ld) { throw -1; }
          if (SDL_LoadWAV_RW(ops, 0, &wav_spec, &wav_buf, &wav_len) == NULL) { throw -2; }
          SDL_BuildAudioCVT(&cvt, wav_spec.format, wav_spec.channels, wav_spec.freq,
                                  spec->format,    spec->channels,    spec->freq);
          ld->data = (Uint8 *)malloc(wav_len * cvt.len_mult);
          cvt.buf = ld->data;
          cvt.len = wav_len;
          memcpy(cvt.buf, wav_buf, wav_len);
          if (SDL_ConvertAudio(&cvt) < 0) { throw -3; }
          ld->data_len = cvt.len_cvt;
        }
        catch (...) {
          ld.reset();
        }
        if (wav_buf)
        {
          SDL_FreeWAV(wav_buf);
          wav_buf = NULL;
        }
        return ld;
      }

      SDL_AudioSpec wav_spec;
      Uint8 *data;
      Uint32 data_len, data_pos;
  };

  class vorbis_loader : public sound_loader
  {
    public:
      typedef boost::shared_ptr<vorbis_loader> ptr;
    protected:
      vorbis_loader(SDL_AudioSpec *spec) :
        sound_loader(spec),
        src(),
        vf(NULL)
      { }
    public:
      virtual ~vorbis_loader()
      {
        if (vf)
        {
          ov_clear(vf);
          delete vf;
          vf = NULL;
        }
      }

      virtual double get_length()
      {
        return ov_time_total(vf, -1);
      }

      virtual double get_position()
      {
        return ov_time_tell(vf);
      }

      virtual int load_samples(Uint8 *buf, Uint32 len)
      {
        if (! spec) { return -1; }

        int count;
        int pos = 0;
        int big_endian = is_big_endian(spec);
        int sign = is_signed(spec);
        int word = get_word_size(spec);

//printf("START LOAD VORBIS\n");
        do {
          int current;
//printf("LOAD VORBIS: POS:%d, LEN:%d\n", (int)pos, (int)len);
          count = ov_read(vf, (char *)buf + pos, len - pos,
                          big_endian, word, sign, &current);
          pos += count;
        } while (count > 0);
//printf("END LOAD VORBIS\n");

        return pos;
      }

      static size_t ogg_read(void *ptr, size_t size, size_t nmemb, void *datasource)
      {
        SDL_RWops *ops = (SDL_RWops *)datasource;
        return ops->read(ops, ptr, size, nmemb);
      }

      static int ogg_seek(void *datasource, ogg_int64_t offset, int whence)
      {
        SDL_RWops *ops = (SDL_RWops *)datasource;
        return ops->seek(ops, offset, whence);
      }

      static long ogg_tell(void *datasource)
      {
        SDL_RWops *ops = (SDL_RWops *)datasource;
        return ops->seek(ops, 0, SEEK_CUR);
      }

      static vorbis_loader::ptr open(file::ptr src, SDL_AudioSpec *spec)
      {
        vorbis_loader::ptr ld;
        if (! src) { return ld; }
        if (! spec) { return ld; }
        SDL_RWops *ops = (SDL_RWops *)src->get_ops();
        ov_callbacks cb = { ogg_read, ogg_seek, NULL, ogg_tell };
        try {
          src->seek(0);
          ld.reset(new vorbis_loader(spec));
          if (! ld) { throw -1; }
          ld->vf = new OggVorbis_File;
          if (! ld->vf) { throw -2; }
          ld->src = src;
          // opening ogg file, checking validity
          if (ov_open_callbacks(ops, ld->vf, NULL, 0, cb) != 0)
          {
            // cleaning the vorbis file before the dtor
            delete ld->vf;
            ld->vf = NULL;
            throw -3;
          }
        }
        catch (...) {
          ld.reset();
        }
        return ld;
      }

      virtual bool set_position(double s = 0)
      {
        if (s < 0)
        {
          s = s + get_length();
        }
        if (ov_time_seek(vf, s) == 0) { return true; }
        return false;
      }

      file::ptr src;
      OggVorbis_File *vf;
  };

  class impl_sound : public sound
  {
    public:
      typedef boost::shared_ptr<impl_sound> ptr;
    protected:
      impl_sound(SDL_AudioSpec *spec) :
        sound(),
        loader(), loop(false),
        spec(spec), playing(false), volume(1)
      { }
    public:
      virtual ~impl_sound()
      {
        clear();
      }

      virtual bool clear()
      {
        audio_locker lock;
        loader.reset();
        loop = false;
        playing = false;
        return true;
      }

      static impl_sound::ptr create(SDL_AudioSpec *spec)
      {
        impl_sound::ptr snd;
        try {
          snd.reset(new impl_sound(spec));
          if (! snd) { throw -1; }
        }
        catch (...) {
          snd.reset();
          lev::debug_print("error on sound instance creation");
        }
        return snd;
      }

      virtual double get_length()
      {
        if (! loader) { return 0; }
        return loader->get_length();
      }

      virtual float get_pan() const
      {
        return 0;
//        if (this->buf == NULL) { return 0; }
//        return playback->get_pan();
      }

      virtual double get_position()
      {
        if (! loader) { return 0; }
        return loader->get_position();
      }

      virtual double get_volume() const
      {
        return volume;
      }

      virtual bool is_playing() const
      {
        return playing;
      }

      virtual bool open(const std::string &filename)
      {
        return open_file(file::open(filename));
      }

      virtual bool open_file(file::ptr src)
      {
        clear();
        sound_loader::ptr ld;
        ld = wav_loader::open(src, spec);
        if (! ld) { ld = vorbis_loader::open(src, spec); }
        if (! ld) { return false; }
        audio_locker lock;
        loader = ld;
        return true;
      }

      virtual bool open_path(filepath::ptr path)
      {
        return open(path->get_string());
      }

      virtual bool open_and_play(const std::string &filename, bool repeat)
      {
        open(filename);
        return set_playing(true, repeat);
      }

      virtual bool open_and_play_file(file::ptr src, bool repeat = false)
      {
        open_file(src);
        return set_playing(true, repeat);
      }

      virtual bool set_pan(float pan)
      {
        return 0;
//        if (this->buf == NULL) { return false; }
//        playback->set_pan(pan);
//        return true;
      }

      virtual bool set_playing(bool play, bool repeat)
      {
        audio_locker lock;
        playing = play;
        loop = repeat;
        return true;
      }

      virtual bool set_position(double s)
      {
        if (! loader) { return false; }
        audio_locker locker;
        return loader->set_position(s);
      }

      virtual bool set_volume(double vol)
      {
        if (vol < 0 || vol > 1) { return false; }
        audio_locker lock;
        volume = vol;
        return true;
      }

      SDL_AudioSpec *spec;
      bool loop;
      bool playing;
      double volume;
      sound_loader::ptr loader;
  };

  // mixer class implementation
  class mixer_core
  {
    public:
      typedef boost::shared_ptr<mixer_core> ptr;
    protected:
      mixer_core() :
        active(false), slots()
      { }
    public:
      virtual ~mixer_core()
      {
        slots.clear();
//        if (system::get_interpreter())
//        {
//printf("CLOSING AUDIO!\n");
//          SDL_CloseAudio();
//printf("CLOSED AUDIO!\n");
//        }
      }

      static void audio_callback(void *udata, Uint8 *stream, int len)
      {
        mixer_core::ptr mx = mixer_core::singleton;
        if (! mx) { return; }

        std::map<int, impl_sound::ptr>::iterator i;
        memset(stream, 0, len);
        for (i = mx->slots.begin(); i != mx->slots.end(); i++)
        {
          if (! i->second) { continue; }
          impl_sound::ptr snd = i->second;
          if (! snd->playing) { continue; }
          if (! snd->loader) { continue; }
          Uint8 buf[len];
          memset(buf, len, 0);
          int loaded = snd->loader->load_samples(buf, len);
          if (loaded > 0)
          {
            SDL_MixAudio(stream, buf, len, SDL_MIX_MAXVOLUME * snd->volume);
          }
          else
          {
            if (snd->loop) { snd->set_position(0); }
            else { snd->playing = false; }
          }
        }
      }

      static mixer_core::ptr create()
      {
        if (singleton) { return singleton; }
        try {
          singleton.reset(new mixer_core);
          if (! singleton) { throw -1; }
          SDL_AudioSpec request;
          request.freq     = 44100;
          request.format   = AUDIO_S16;
          request.channels = 2;
          request.samples  = 512;
          request.callback = mixer_core::audio_callback;
          request.userdata = singleton.get();
          if (SDL_OpenAudio(&request, &singleton->spec) < 0) { throw -2; }
          SDL_PauseAudio(0);
          singleton->active = true;
        }
        catch (...) {
          singleton.reset();
          lev::debug_print("error on audio system initialization");
        }
        return singleton;
      }

      bool active;
      SDL_AudioSpec spec;
      std::map<int, impl_sound::ptr> slots;
      // singleton
      static mixer_core::ptr singleton;
  };
  mixer_core::ptr mixer_core::singleton;

  class impl_mixer : public mixer
  {
    public:
      typedef boost::shared_ptr<impl_mixer> ptr;
    protected:
      impl_mixer() :
        mixer()
      { }
    public:
      virtual ~impl_mixer()
      {
        close();
      }

      virtual bool activate(bool active = true)
      {
        if (active) { SDL_PauseAudio(0); }
        else { SDL_PauseAudio(1); }
        core->active = active;
        return true;
      }

      virtual bool clear_slot(int slot_num)
      {
        std::map<int, impl_sound::ptr>::iterator found;
        found = core->slots.find(slot_num);
        if (found != core->slots.end())
        {
          audio_locker lock;
          core->slots.erase(found);
          return true;
        }
        return false;
      }

      bool close()
      {
        if (! core) { return false; }
printf("CLOSING MIXER CORE COUNT: %ld\n", mixer_core::singleton.use_count());
        core.reset();
printf("RESETTED MIXER CORE COUNT: %ld\n", mixer_core::singleton.use_count());
        if (mixer_core::singleton.use_count() == 1)
        {
printf("MIXER SINGLETON CLEAR\n");
          mixer_core::singleton.reset();
        }
      }

      sound::ptr create_slot(int slot_num)
      {
        impl_sound::ptr slot;
        try {
          slot = impl_sound::create(&core->spec);
          if (! slot) { throw -1; }
          audio_locker lock;
          core->slots[slot_num] = slot;
        }
        catch (...) {
          slot.reset();
        }
        return slot;
      }

      sound::ptr find_free_slot()
      {
        for (int i = -1; ; i--)
        {
          std::map<int, impl_sound::ptr>::iterator found;
          std::map<int, impl_sound::ptr> &slots = core->slots;
          found = slots.find(i);
          if (found == slots.end())
          {
            // slots[i] is not found
            return create_slot(i);
          }
          else if (! slots[i])
          {
            // cannnels[i] is NULL
            return create_slot(i);
          }
          else if (! slots[i]->is_playing())
          {
            // channels[i] is not playing, therefore ready for another use
            return slots[i];
          }
          else
          {
            // slots[i] is found and is used
            continue;
          }
        }
      }

      virtual int get_channels() const
      {
        return core->spec.channels;
      }

      virtual int get_freq() const
      {
        return core->spec.freq;
      }

      static impl_mixer::ptr get()
      {
        impl_mixer::ptr mx;
        if (! mixer_core::singleton) { return mx; }
        try {
          mx.reset(new impl_mixer);
          if (! mx) { throw -1; }
          mx->core = mixer_core::singleton;
printf("GET MIXER CORE COUNT: %ld\n", mx->core.use_count());
        }
        catch (...) {
          mx.reset();
          lev::debug_print("error on mixer instance cloning");
        }
        return mx;
      }

      virtual sound::ptr get_slot(int slot_num)
      {
        if (slot_num == 0)
        {
          return find_free_slot();
        }
        else // if (slot_num != 0)
        {
          if (! core->slots[slot_num])
          {
            // slots[i] is NULL
            return create_slot(slot_num);
          }
          else { return core->slots[slot_num]; }
        }
      }

      static impl_mixer::ptr init()
      {
        impl_mixer::ptr mx;
        if (! system::get_interpreter()) { return mx; }
        try {
          mx.reset(new impl_mixer);
          if (! mx) { throw -1; }
          mx->core = mixer_core::create();
          if (! mx->core) { throw -2; }
        }
        catch (...) {
          mx.reset();
          lev::debug_print("error on mixer instance creation");
        }
        return mx;
      }

      virtual bool is_active() const
      {
        return core->active;
      }

      mixer_core::ptr core;
  };

  mixer::ptr mixer::get()
  {
    return get();
  }

  mixer::ptr mixer::init()
  {
    return impl_mixer::init();
  }

//  SDL_AudioSpec& get_spec(mixer *mx)
//  {
//    return mixer_core::singleton->spec;
//  }

}

