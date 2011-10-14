/////////////////////////////////////////////////////////////////////////////
// Name:        src/sound.cpp
// Purpose:     source for sound control
// Author:      Akiva Miura <akiva.miura@gmail.com>
// Modified by:
// Created:     05/13/2011
// Copyright:   (C) 2010-2011 Akiva Miura
// Licence:     MIT License
/////////////////////////////////////////////////////////////////////////////

//#include "prec.h"

#include "lev/sound.hpp"
#include "register.hpp"

#include <luabind/adopt_policy.hpp>
#include <luabind/luabind.hpp>
#include <map>

/*
int luaopen_lev_sound(lua_State *L)
{
  using namespace luabind;
  using namespace lev;

  luabind::open(L);
  globals(L)["require"]("lev");

  module(L, "lev")
  [
    namespace_("sound")
    [
      def("init", &sound::init),
      def("play", &sound::play),
      def("stop", &sound::stop_all)
    ],
    namespace_("classes")
    [
      class_<channel, base>("channel")
        .def("clean", &channel::clean)
        .def("load", &channel::load)
        .def("open", &channel::open)
        .def("pause", &channel::pause)
        .def("play", &channel::play)
        .def("play", &channel::play_with)
        .property("id", &channel::get_id)
        .property("is_playing", &channel::get_playing, &channel::set_playing)
        .property("isplaying", &channel::get_playing, &channel::set_playing)
        .property("len", &channel::get_length)
        .property("length", &channel::get_length)
        .property("pan", &channel::get_pan, &channel::set_pan)
        .property("pos", &channel::get_position, &channel::set_position)
        .property("position", &channel::get_position, &channel::set_position),
      class_<mixer, base>("mixer")
        .def("ch", &mixer::get_channel)
        .def("ch", &mixer::get_channel0)
        .def("channel", &mixer::get_channel)
        .def("channel", &mixer::get_channel0)
        .def("clean_channel", &mixer::clean_channel)
        .def("get_channel", &mixer::get_channel)
        .def("get_channel", &mixer::get_channel0)
        .def("pause", &mixer::pause)
        .def("play", &mixer::play)
        .property("is_playing", &mixer::get_playing, &mixer::set_playing)
        .property("isplaying", &mixer::get_playing, &mixer::set_playing)
        .property("type_id", &mixer::get_type_id)
        .property("type_name", &mixer::get_type_name)
        .scope
        [
          def("create_c", &mixer::create, adopt(result))
        ]
    ]
  ];
  object classes = globals(L)["lev"]["classes"];
  object sound = globals(L)["lev"]["sound"];
  register_to(classes["mixer"], "create", &mixer::create_l);
  sound["mixer"] = classes["mixer"]["create"];

  globals(L)["package"]["loaded"]["lev.sound"] = sound;
  return 0;
}

namespace lev
{

  class myChannel
  {
    private:

      myChannel() : buf(NULL), playback(NULL) {}

    public:

      ~myChannel() { Clean(); }

      bool Clean()
      {
        if (this->buf)
        {
          playback->stop();
          delete playback;
          playback = NULL;
          delete buf;
          buf = NULL;
          return true;
        }
        else { return false; }
      }

      static myChannel *Create()
      {
        myChannel *ch = new myChannel;
        if (ch == NULL) { return NULL; }
        return ch;
      }

      double GetLength()
      {
        if (this->buf == NULL) { return -1; }
        return playback->get_length();
      }

      float GetPan()
      {
        if (this->buf == NULL) { return 0; }
        return playback->get_pan();
      }

      bool GetPlaying()
      {
        if (this->buf == NULL) { return false; }
        return playback->is_playing();
      }

      double GetPosition()
      {
        if (this->buf == NULL) { return -1; }
        return playback->get_position();
      }

      bool LoadSample(const char *filename)
      {
//        ALLEGRO_SAMPLE *sample = NULL;
//        ALLEGRO_SAMPLE_INSTANCE *instance = NULL;
//        if (mixer == NULL) { return false; }
//
//        sample = al_load_sample(filename);
//        if (sample == NULL) { goto Error; }
//        instance = al_create_sample_instance(sample);
//        if (instance == NULL) { goto Error; }
//        if (not al_attach_sample_instance_to_mixer(instance, (ALLEGRO_MIXER *)mixer))
//        {
//          goto Error;
//        }
//        if (this->snd) { this->Clean(); }
//        inst = instance;
//        snd  = sample;
//        type = SOUND_SAMPLE;
//        return true;
//
//        Error:
//
//        if (instance) { al_destroy_sample_instance(instance); }
//        if (sample) { al_destroy_sample(sample); }
        return false;
      }

      bool OpenStream(const char *filename)
      {
        CL_SoundBuffer *buffer = NULL;
        CL_SoundBuffer_Session *session = NULL;
        try {
          if (this->buf) { this->Clean(); }
          buffer = new CL_SoundBuffer(filename);
          session = new CL_SoundBuffer_Session(buffer->prepare());
          this->buf      = buffer;
          this->playback = session;
          return true;
        }
        catch (...) {
          delete session;
          delete buffer;
          return false;
        }
      }

      bool Play(const char *filename)
      {
        OpenStream(filename);
        return SetPlaying(true);
      }

      bool SetPan(float pan)
      {
        if (this->buf == NULL) { return false; }
        playback->set_pan(pan);
        return true;
      }

      bool SetPlaying(bool play)
      {
        if (this->buf == NULL) { return false; }
        if (play == true) { playback->play(); }
        else { playback->stop(); }
        return true;
      }

      bool SetPosition(double pos)
      {
        if (this->buf == NULL) { return false; }
        playback->set_position(pos);
        return true;
      }

      CL_SoundBuffer *buf;
      CL_SoundBuffer_Session *playback;
  };

  channel::channel() : _obj(NULL) {}
  channel::~channel()
  {
    if (_obj) { delete (myChannel *)_obj; }
  }

  channel* channel::create(int id)
  {
    channel *ch = new channel;
    if (ch == NULL) { return NULL; }
    myChannel *obj = myChannel::Create();
    if (obj == NULL) { goto Error; }
    ch->_obj = obj;
    ch->_id = id;
    return ch;

    Error:
    delete ch;
    return NULL;
  }

  bool channel::clean() { return ((myChannel *)_obj)->Clean(); }
  double channel::get_length() { return ((myChannel *)_obj)->GetLength(); }
  float channel::get_pan() { return ((myChannel *)_obj)->GetPan(); }
  bool channel::get_playing() { return ((myChannel *)_obj)->GetPlaying(); }
  double channel::get_position() { return ((myChannel *)_obj)->GetPosition(); }

  bool channel::load(const char *filename) { return ((myChannel *)_obj)->LoadSample(filename); }
  bool channel::open(const char *filename) { return ((myChannel *)_obj)->OpenStream(filename); }
  bool channel::play_with(const char *filename) { return ((myChannel *)_obj)->Play(filename); }
  bool channel::set_pan(float pan) { return ((myChannel *)_obj)->SetPan(pan); }
  bool channel::set_playing(bool play) { return ((myChannel *)_obj)->SetPlaying(play); }
  bool channel::set_position(double pos) { return ((myChannel *)_obj)->SetPosition(pos); }



  // Mixer Implementation

  class myMixer
  {
    public:


      myMixer() : channels() {}

      ~myMixer()
      {
        std::map<unsigned, channel *>::iterator i;
        for (i = channels.begin(); i != channels.end(); i++)
        {
          if (i->second != NULL) { i->second->clean(); }
        }
      }

      static myMixer *Create()
      {
        myMixer *mx = new myMixer;
        if (mx == NULL) { return NULL; }
        return mx;

        Error:
        delete mx;
        return NULL;
      }

      bool CleanChannel(int channel_num)
      {
        if (channel_num < 0) { return false; }
        std::map<unsigned int, channel *>::iterator found;
        found = channels.find(channel_num);
        if (found != channels.end())
        {
          delete found->second;
          //found->second = NULL;
          channels.erase(found);
          return true;
        }
        else { return false; }
      }

      channel *CreateChannel(int channel_num)
      {
        channel *ch = channel::create(channel_num);
        if (ch == NULL) { return NULL; }
        channels[channel_num] = ch;
        return ch;
      }

      channel *GetChannel(int channel_num)
      {
        if (channel_num < 0) { return NULL; }
        else if (channel_num == 0)
        {
          int i;
          for (i = 1; ; i++)
          {
            std::map<unsigned int, channel *>::iterator found;
            found = channels.find(i);
            if (found == channels.end())
            {
              // channels[i] is not found
              return CreateChannel(i);
            }
            else if (channels[i] == NULL)
            {
              // cannnels[i] is NULL
              return CreateChannel(i);
            }
            else
            {
              // channels[i] is found
              continue;
            }
          }
        }
        else // if (channel_num > 0)
        {
          if (channels[channel_num] == NULL)
          {
            // channels[i] is NULL
            return CreateChannel(channel_num);
          }
          else { return channels[channel_num]; }
        }
      }

      bool GetPlaying()
      {
//        return al_get_mixer_playing((ALLEGRO_MIXER *)mixer);
      }

      bool SetPlaying(bool play)
      {
//        return al_set_mixer_playing((ALLEGRO_MIXER *)mixer, play);
      }

    private:
//      ALLEGRO_VOICE *voice;
//      ALLEGRO_MIXER *mixer;
      std::map<unsigned int, channel *> channels;
  };


  mixer::mixer() : _obj(NULL) { }

  mixer::~mixer()
  {
    if (_obj) { delete (myMixer *)_obj; }
  }

  bool mixer::clean_channel(int channel_num)
  {
    return ((myMixer *)_obj)->CleanChannel(channel_num);
  }

  mixer* mixer::create()
  {
    mixer *mx = NULL;
    try {
      if (! sound::init()) { throw -1; }
      mx = new mixer;
      mx->_obj = myMixer::Create();
      if (! mx->_obj) { throw -2; }
      return mx;
    }
    catch (...) {
      delete mx;
      return NULL;
    }
  }

  int mixer::create_l(lua_State *L)
  {
    using namespace luabind;

    object func = globals(L)["lev"]["classes"]["mixer"]["create_c"];
    object mx = func();
    if (mx)
    {
      lua_pushcfunction(L, &mixer::get_field);
      object get_field(from_stack(L, 1));
      lua_pop(L, 1);
      object meta = getmetatable(mx);
      if (not meta["__index2"])
      {
        meta["__index2"] = meta["__index"];
        meta["__index"] = get_field;
      }
    }
    mx.push(L);
    return 1;
  }

  channel* mixer::get_channel(int channel_num)
  {
    return ((myMixer *)_obj)->GetChannel(channel_num);
  }

  int mixer::get_field(lua_State *L)
  {
    using namespace luabind;

    luaL_checktype(L, 1, LUA_TUSERDATA);
    object mixer = object(from_stack(L, 1));
    object index = object(from_stack(L, 2));

    if (luabind::type(index) == LUA_TNUMBER)
    {
      object func = mixer["get_channel"];
      object channel = func(mixer, index);
      channel.push(L);
      return 1;
    }
    else
    {
      object func = getmetatable(mixer)["__index2"];
      object field = func(mixer, index);
      field.push(L);
      return 1;
    }
  }

  bool mixer::get_playing()
  {
    return ((myMixer *)_obj)->GetPlaying();
  }

  bool mixer::set_playing(bool play)
  {
    return ((myMixer *)_obj)->SetPlaying(play);
  }




  class mySoundManager
  {
    public:
      mySoundManager()
        : setup_sound(NULL), setup_vorbis(NULL), output(NULL),
          play_back(), play_list() {}

      ~mySoundManager()
      {
        if (output) { delete output; }
        if (setup_vorbis) { delete setup_vorbis; }
        if (setup_sound) { delete setup_sound; }
      }

      static mySoundManager *Create()
      {
        mySoundManager *mng = NULL;
        try {
          mng = new mySoundManager;
          mng->setup_sound = new CL_SetupSound;
          mng->setup_vorbis = new CL_SetupVorbis;
          mng->output = new CL_SoundOutput(44100);
          return mng;
        }
        catch (...) {
          delete mng;
          return NULL;
        }
      }

      bool PlayAdd(const char *filename)
      {
        CL_SoundBuffer *buf = new CL_SoundBuffer(filename);
        if (buf == NULL) { return false; }
        CL_SoundBuffer_Session *back = new CL_SoundBuffer_Session(buf->play());
        play_list.push_back(buf);
        play_back.push_back(back);
        return true;
      }

      bool StopAll()
      {
        int i;
        for (i = 0; i < play_list.size(); i++)
        {
          play_back[i]->stop();
          delete (CL_SoundBuffer_Session *)(play_back[i]);
          delete (CL_SoundBuffer *)(play_list[i]);
        }
        play_back.clear();
        play_list.clear();
        return true;
      }

      CL_SetupSound  *setup_sound;
      CL_SetupVorbis *setup_vorbis;
      CL_SoundOutput *output;
      std::vector<CL_SoundBuffer *> play_list;
      std::vector<CL_SoundBuffer_Session *> play_back;
  };

  sound::sound() : _obj(NULL) { }

  sound::~sound()
  {
    if (_obj) { delete (mySoundManager *)_obj; }
  }

  sound* sound::get()
  {
    static sound snd;
    if (snd._obj) { return &snd; }
    try {
      snd._obj = mySoundManager::Create();
      if (snd._obj == NULL) { return NULL; }
      else { return &snd; }
    }
    catch (...) {
      return NULL;
    }
  }

  bool sound::init()
  {
    if (sound::get()) { return true; }
    return false;
  }

  bool sound::play(const char *filename)
  {
    sound *snd = sound::get();
    if (snd == NULL) { return false; }
    return ((mySoundManager *)snd->_obj)->PlayAdd(filename);
  }

  bool sound::stop_all()
  {
    sound *snd = sound::get();
    if (snd == NULL) { return false; }
    return ((mySoundManager *)snd->_obj)->StopAll();
  }
}

*/

