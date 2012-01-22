/////////////////////////////////////////////////////////////////////////////
// Name:        src/system.cpp
// Purpose:     source for system managing class
// Author:      Akiva Miura <akiva.miura@gmail.com>
// Modified by:
// Created:     13/10/2011
// Copyright:   (C) 2010-2011 Akiva Miura
// Licence:     MIT License
/////////////////////////////////////////////////////////////////////////////

// pre-compiled header
#include "prec.h"

// declarations
#include "lev/system.hpp"

// dependencies
#include "lev/draw.hpp"
#include "lev/timer.hpp"
#include "lev/window.hpp"
#include "register.hpp"

// libraries
#include <map>
#include <luabind/luabind.hpp>
#include <SDL/SDL.h>
#include <vector>

//namespace lev
//{
//  class finalizer : public base
//  {
//    protected:
//      finalizer() : base() { }
//    public:
//      virtual ~finalizer()
//      {
//printf("FINALIZE!\n");
//        system::get()->done();
//      }
//
//      static finalizer* create()
//      {
//        finalizer *f = NULL;
//        try {
//          f = new finalizer;
//          return f;
//        }
//        catch (...) {
//          delete f;
//          return NULL;
//        }
//      }
//
//      virtual type_id get_type_id() const { return LEV_TFINALIZER; }
//      virtual const char *get_type_name() const { return "lev.system.finalizer"; }
//  };
//}

int luaopen_lev_system(lua_State *L)
{
  using namespace luabind;
  using namespace lev;

  open(L);
  // beginning to load
  globals(L)["package"]["loaded"]["lev.system"] = true;
  // pre-requirement
  globals(L)["require"]("lev.base");
  globals(L)["require"]("lev.image");
  globals(L)["require"]("lev.timer");

  module(L, "lev")
  [
    namespace_("system"),
    namespace_("classes")
    [
      class_<event, base>("event")
        .property("button", &event::get_button)
        .property("dx", &event::get_dx)
        .property("dy", &event::get_dy)
        .property("is_pressed", &event::is_pressed)
        .property("is_released", &event::is_released)
        .property("key", &event::get_key)
        .property("keycode", &event::get_key_code)
        .property("key_code", &event::get_key_code)
        .property("left_is_down", &event::left_is_down)
        .property("middle_is_down", &event::middle_is_down)
        .property("pressed", &event::is_pressed)
        .property("released", &event::is_released)
        .property("right_is_down", &event::right_is_down)
        .property("scan_code", &event::get_scan_code)
        .property("scancode", &event::get_scan_code)
        .property("x", &event::get_x)
        .property("y", &event::get_y),
      class_<lev::system, base>("system")
        .def("clock", &system::create_clock, adopt(result))
        .def("clock", &system::create_clock0, adopt(result))
        .def("close", &system::done)
        .def("create_clock", &system::create_clock, adopt(result))
        .def("create_clock", &system::create_clock0, adopt(result))
        .def("create_timer", &system::create_timer, adopt(result))
        .def("create_timer", &system::create_timer0, adopt(result))
        .def("delay", &system::delay)
        .def("do_event", &system::do_event)
        .def("do_events", &system::do_events)
        .def("done", &system::done)
        .property("is_running", &system::is_running, &system::set_running)
        .property("on_button_down", &system::get_on_button_down, &system::set_on_button_down)
        .property("on_button_up", &system::get_on_button_up, &system::set_on_button_up)
        .property("on_key_down", &system::get_on_key_down, &system::set_on_key_down)
        .property("on_key_up", &system::get_on_key_up, &system::set_on_key_up)
        .property("on_left_down", &system::get_on_left_down, &system::set_on_left_down)
        .property("on_left_up", &system::get_on_left_up, &system::set_on_left_up)
        .property("on_middle_down", &system::get_on_middle_down, &system::set_on_middle_down)
        .property("on_middle_up", &system::get_on_middle_up, &system::set_on_middle_up)
        .property("on_motion", &system::get_on_motion, &system::set_on_motion)
        .property("on_quit", &system::get_on_quit, &system::set_on_quit)
        .property("on_right_down", &system::get_on_right_down, &system::set_on_right_down)
        .property("on_right_up", &system::get_on_right_up, &system::set_on_right_up)
        .property("on_tick", &system::get_on_tick, &system::set_on_tick)
        .def("quit", &system::quit)
        .def("quit", &system::quit0)
        .def("run", &system::run)
        .property("screen", &system::get_screen)
        .def("set_running", &system::set_running)
        .def("set_video_mode", &system::set_video_mode)
        .def("set_video_mode", &system::set_video_mode2)
        .property("ticks", &system::get_ticks)
        .def("timer", &system::create_timer, adopt(result))
        .def("timer", &system::create_timer0, adopt(result))
        .def("toggle_full_screen", &system::toggle_full_screen)
        .scope
        [
          def("create_window_c", &window::create, adopt(result)),
          def("get_c", &system::get),
          def("init", &system::init)
        ]
//      class_<lev::finalizer, base>("finalizer")
//        .scope
//        [
//          def("create", &finalizer::create, adopt(result))
//        ]
    ]
  ];
  object lev = globals(L)["lev"];
  object classes = lev["classes"];
  register_to(classes["system"], "get", &system::get_l);
  register_to(classes["system"], "create_window", &window::create_l);
  register_to(classes["system"], "window", &window::create_l);
  lev["system"] = classes["system"]["get"];
//  lev["finalizer"] = classes["finalizer"]["create"];

  // post-requirement
  globals(L)["require"]("lev.window");
  // end of loading
  globals(L)["package"]["loaded"]["lev.system"] = true;
  return 0;
}

namespace lev
{

  const char *input::to_keyname(long code)
  {
    static std::map<long, std::string> *keymap = NULL;

    if (keymap == NULL)
    {
      try {
        keymap = new std::map<long, std::string>;

        for (int i = 0; i <= 127; i++)
        {
          char ascii[2] = {tolower(i), 0};
          (*keymap)[tolower(i)] = ascii;
        }

        (*keymap)[SDLK_UNKNOWN]            = "(unknown)";
        (*keymap)[SDLK_AC_BACK]            = "ac_back";
        (*keymap)[SDLK_AC_BOOKMARKS]       = "ac_bookmarks";
        (*keymap)[SDLK_AC_FORWARD]         = "ac_forward";
        (*keymap)[SDLK_AC_HOME]            = "ac_home";
        (*keymap)[SDLK_AC_REFRESH]         = "ac_refresh";
        (*keymap)[SDLK_AC_SEARCH]          = "ac_search";
        (*keymap)[SDLK_AC_STOP]            = "ac_stop";
        (*keymap)[SDLK_AGAIN]              = "again";
        (*keymap)[SDLK_ALTERASE]           = "alterase";
        (*keymap)[SDLK_AMPERSAND]          = "ampersand";
        (*keymap)[SDLK_APPLICATION]        = "application";
        (*keymap)[SDLK_ASTERISK]           = "asterisk";
        (*keymap)[SDLK_AT]                 = "at";
        (*keymap)[SDLK_AUDIOMUTE]          = "audiomute";
        (*keymap)[SDLK_AUDIONEXT]          = "audionext";
        (*keymap)[SDLK_AUDIOPLAY]          = "audioplay";
        (*keymap)[SDLK_AUDIOPREV]          = "audioprev";
        (*keymap)[SDLK_AUDIOSTOP]          = "audiostop";
        (*keymap)[SDLK_BACKSLASH]          = "backslash";
        (*keymap)[SDLK_BACKSPACE]          = "backspace";
        (*keymap)[SDLK_BACKQUOTE]          = "backquote";
        (*keymap)[SDLK_BRIGHTNESSDOWN]     = "brightnessdown";
        (*keymap)[SDLK_BRIGHTNESSUP]       = "brightnessup";
        (*keymap)[SDLK_CALCULATOR]         = "calculator";
        (*keymap)[SDLK_CANCEL]             = "cancel";
        (*keymap)[SDLK_CAPSLOCK]           = "capslock";
        (*keymap)[SDLK_CARET]              = "caret";
        (*keymap)[SDLK_CLEAR]              = "clear";
        (*keymap)[SDLK_CLEARAGAIN]         = "clearagain";
        (*keymap)[SDLK_COLON]              = "colon";
        (*keymap)[SDLK_COMMA]              = "comma";
        (*keymap)[SDLK_COMPUTER]           = "computer";
        (*keymap)[SDLK_COPY]               = "copy";
        (*keymap)[SDLK_CRSEL]              = "crsel";
        (*keymap)[SDLK_CURRENCYUNIT]       = "currencyunit";
        (*keymap)[SDLK_CURRENCYSUBUNIT]    = "currencysubunit";
        (*keymap)[SDLK_CUT]                = "cut";
        (*keymap)[SDLK_DECIMALSEPARATOR]   = "decimalseparator";
        (*keymap)[SDLK_DELETE]             = "delete";
        (*keymap)[SDLK_DISPLAYSWITCH]      = "displayswitch";
        (*keymap)[SDLK_DOLLAR]             = "dollar";
        (*keymap)[SDLK_DOWN]               = "down";
        (*keymap)[SDLK_QUOTEDBL]           = "doublequote";
        (*keymap)[SDLK_EJECT]              = "eject";
        (*keymap)[SDLK_END]                = "end";
        (*keymap)[SDLK_EQUALS]             = "equals";
        (*keymap)[SDLK_ESCAPE]             = "escape";
        (*keymap)[SDLK_EXCLAIM]            = "exclaim";
        (*keymap)[SDLK_EXECUTE]            = "execute";
        (*keymap)[SDLK_EXSEL]              = "exsel";
        (*keymap)[SDLK_F1]                 = "f1";
        (*keymap)[SDLK_F2]                 = "f2";
        (*keymap)[SDLK_F3]                 = "f3";
        (*keymap)[SDLK_F4]                 = "f4";
        (*keymap)[SDLK_F5]                 = "f5";
        (*keymap)[SDLK_F6]                 = "f6";
        (*keymap)[SDLK_F7]                 = "f7";
        (*keymap)[SDLK_F8]                 = "f8";
        (*keymap)[SDLK_F9]                 = "f9";
        (*keymap)[SDLK_F10]                = "f10";
        (*keymap)[SDLK_F11]                = "f11";
        (*keymap)[SDLK_F12]                = "f12";
        (*keymap)[SDLK_F13]                = "f13";
        (*keymap)[SDLK_F14]                = "f14";
        (*keymap)[SDLK_F15]                = "f15";
        (*keymap)[SDLK_F16]                = "f16";
        (*keymap)[SDLK_F17]                = "f17";
        (*keymap)[SDLK_F18]                = "f18";
        (*keymap)[SDLK_F19]                = "f19";
        (*keymap)[SDLK_F20]                = "f20";
        (*keymap)[SDLK_F21]                = "f21";
        (*keymap)[SDLK_F22]                = "f22";
        (*keymap)[SDLK_F23]                = "f23";
        (*keymap)[SDLK_F24]                = "f24";
        (*keymap)[SDLK_FIND]               = "find";
        (*keymap)[SDLK_GREATER]            = "greater";
        (*keymap)[SDLK_HASH]               = "hash";
        (*keymap)[SDLK_HELP]               = "help";
        (*keymap)[SDLK_HOME]               = "home";
        (*keymap)[SDLK_INSERT]             = "insert";
        (*keymap)[SDLK_KBDILLUMDOWN]       = "kbdillumdown";
        (*keymap)[SDLK_KBDILLUMTOGGLE]     = "kbdillumtoggle";
        (*keymap)[SDLK_KBDILLUMUP]         = "kbdillumup";
        (*keymap)[SDLK_KP_0]               = "kp_0";
        (*keymap)[SDLK_KP_1]               = "kp_1";
        (*keymap)[SDLK_KP_2]               = "kp_2";
        (*keymap)[SDLK_KP_3]               = "kp_3";
        (*keymap)[SDLK_KP_4]               = "kp_4";
        (*keymap)[SDLK_KP_5]               = "kp_5";
        (*keymap)[SDLK_KP_6]               = "kp_6";
        (*keymap)[SDLK_KP_7]               = "kp_7";
        (*keymap)[SDLK_KP_8]               = "kp_8";
        (*keymap)[SDLK_KP_9]               = "kp_9";
        (*keymap)[SDLK_KP_00]              = "kp_00";
        (*keymap)[SDLK_KP_000]             = "kp_000";
        (*keymap)[SDLK_KP_A]               = "kp_a";
        (*keymap)[SDLK_KP_AMPERSAND]       = "kp_ampersand";
        (*keymap)[SDLK_KP_AT]              = "kp_at";
        (*keymap)[SDLK_KP_B]               = "kp_b";
        (*keymap)[SDLK_KP_BACKSPACE]       = "kp_backspace";
        (*keymap)[SDLK_KP_BINARY]          = "kp_binary";
        (*keymap)[SDLK_KP_C]               = "kp_c";
        (*keymap)[SDLK_KP_CLEAR]           = "kp_clear";
        (*keymap)[SDLK_KP_CLEARENTRY]      = "kp_clearentry";
        (*keymap)[SDLK_KP_COLON]           = "kp_colon";
        (*keymap)[SDLK_KP_COMMA]           = "kp_comma";
        (*keymap)[SDLK_KP_D]               = "kp_d";
        (*keymap)[SDLK_KP_DBLAMPERSAND]    = "kp_dblampersand";
        (*keymap)[SDLK_KP_DBLVERTICALBAR]  = "kp_dblverticalbar";
        (*keymap)[SDLK_KP_DECIMAL]         = "kp_decimal";
        (*keymap)[SDLK_KP_DIVIDE]          = "kp_divide";
        (*keymap)[SDLK_KP_E]               = "kp_e";
        (*keymap)[SDLK_KP_ENTER]           = "kp_enter";
        (*keymap)[SDLK_KP_EQUALS]          = "kp_equals";
        (*keymap)[SDLK_KP_EQUALSAS400]     = "kp_equalsas400";
        (*keymap)[SDLK_KP_EXCLAM]          = "kp_exclam";
        (*keymap)[SDLK_KP_F]               = "kp_f";
        (*keymap)[SDLK_KP_GREATER]         = "kp_greater";
        (*keymap)[SDLK_KP_HASH]            = "kp_hash";
        (*keymap)[SDLK_KP_HEXADECIMAL]     = "kp_hexadecimal";
        (*keymap)[SDLK_KP_LEFTBRACE]       = "kp_leftbrace";
        (*keymap)[SDLK_KP_LEFTPAREN]       = "kp_leftparen";
        (*keymap)[SDLK_KP_LESS]            = "kp_less";
        (*keymap)[SDLK_KP_MEMADD]          = "kp_memadd";
        (*keymap)[SDLK_KP_MEMCLEAR]        = "kp_memclear";
        (*keymap)[SDLK_KP_MEMDIVIDE]       = "kp_memdivide";
        (*keymap)[SDLK_KP_MEMMULTIPLY]     = "kp_memmultiply";
        (*keymap)[SDLK_KP_MEMRECALL]       = "kp_memrecall";
        (*keymap)[SDLK_KP_MEMSTORE]        = "kp_memstore";
        (*keymap)[SDLK_KP_MEMSUBTRACT]     = "kp_memsubtract";
        (*keymap)[SDLK_KP_MINUS]           = "kp_minus";
        (*keymap)[SDLK_KP_MULTIPLY]        = "kp_multiply";
        (*keymap)[SDLK_KP_OCTAL]           = "kp_octal";
        (*keymap)[SDLK_KP_PERCENT]         = "kp_percent";
        (*keymap)[SDLK_KP_PERIOD]          = "kp_period";
        (*keymap)[SDLK_KP_PLUS]            = "kp_plus";
        (*keymap)[SDLK_KP_PLUSMINUS]       = "kp_plusminus";
        (*keymap)[SDLK_KP_POWER]           = "kp_power";
        (*keymap)[SDLK_KP_RIGHTBRACE]      = "kp_rightbrace";
        (*keymap)[SDLK_KP_RIGHTPAREN]      = "kp_rightparen";
        (*keymap)[SDLK_KP_SPACE]           = "kp_space";
        (*keymap)[SDLK_KP_TAB]             = "kp_tab";
        (*keymap)[SDLK_KP_VERTICALBAR]     = "kp_verticalbar";
        (*keymap)[SDLK_KP_XOR]             = "kp_xor";
        (*keymap)[SDLK_LALT]               = "lalt";
        (*keymap)[SDLK_LCTRL]              = "lctrl";
        (*keymap)[SDLK_LEFT]               = "left";
        (*keymap)[SDLK_LEFTBRACKET]        = "leftbracket";
        (*keymap)[SDLK_LEFTPAREN]          = "leftparen";
        (*keymap)[SDLK_LESS]               = "less";
        (*keymap)[SDLK_LGUI]               = "lgui";
        (*keymap)[SDLK_LSHIFT]             = "lshift";
        (*keymap)[SDLK_MAIL]               = "mail";
        (*keymap)[SDLK_MEDIASELECT]        = "mediaselect";
        (*keymap)[SDLK_MENU]               = "menu";
        (*keymap)[SDLK_MINUS]              = "minus";
        (*keymap)[SDLK_MODE]               = "mode";
        (*keymap)[SDLK_MUTE]               = "mute";
        (*keymap)[SDLK_NUMLOCKCLEAR]       = "numlockclear";
        (*keymap)[SDLK_OPER]               = "oper";
        (*keymap)[SDLK_OUT]                = "out";
        (*keymap)[SDLK_PAGEDOWN]           = "pagedown";
        (*keymap)[SDLK_PAGEUP]             = "pageup";
        (*keymap)[SDLK_PASTE]              = "paste";
        (*keymap)[SDLK_PAUSE]              = "pause";
        (*keymap)[SDLK_PERCENT]            = "percent";
        (*keymap)[SDLK_PERIOD]             = "period";
        (*keymap)[SDLK_PLUS]               = "plus";
        (*keymap)[SDLK_POWER]              = "power";
        (*keymap)[SDLK_PRINTSCREEN]        = "printscreen";
        (*keymap)[SDLK_PRIOR]              = "prior";
        (*keymap)[SDLK_QUOTE]              = "quote";
        (*keymap)[SDLK_QUESTION]           = "question";
        (*keymap)[SDLK_RALT]               = "ralt";
        (*keymap)[SDLK_RCTRL]              = "rctrl";
        (*keymap)[SDLK_RETURN]             = "return";
        (*keymap)[SDLK_RETURN2]            = "return2";
        (*keymap)[SDLK_RGUI]               = "rgui";
        (*keymap)[SDLK_RIGHT]              = "right";
        (*keymap)[SDLK_RIGHTPAREN]         = "rightparen";
        (*keymap)[SDLK_RIGHTBRACKET]       = "rightbracket";
        (*keymap)[SDLK_RSHIFT]             = "rshift";
        (*keymap)[SDLK_SCROLLLOCK]         = "scrolllock";
        (*keymap)[SDLK_SELECT]             = "select";
        (*keymap)[SDLK_SEMICOLON]          = "semicolon";
        (*keymap)[SDLK_SEPARATOR]          = "separator";
        (*keymap)[SDLK_SLASH]              = "slash";
        (*keymap)[SDLK_SLEEP]              = "sleep";
        (*keymap)[SDLK_SPACE]              = "space";
        (*keymap)[SDLK_STOP]               = "stop";
        (*keymap)[SDLK_SYSREQ]             = "sysreq";
        (*keymap)[SDLK_TAB]                = "tab";
        (*keymap)[SDLK_THOUSANDSSEPARATOR] = "thousandsseparator";
        (*keymap)[SDLK_UNDERSCORE]         = "underscore";
        (*keymap)[SDLK_UNDO]               = "undo";
        (*keymap)[SDLK_UP]                 = "up";
        (*keymap)[SDLK_VOLUMEDOWN]         = "volumedown";
        (*keymap)[SDLK_VOLUMEUP]           = "volumeup";
        (*keymap)[SDLK_WWW]                = "www";
      }
      catch (...) {
        delete keymap;
        keymap = NULL;
        return "(error)";
      }
    }

    std::map<long, std::string>::iterator found;
    found = keymap->find(code);
    if (found == keymap->end()) { return "(undefined)"; }
    else { return found->second.c_str(); }
  }

  class myEvent
  {
    public:
      myEvent() : evt() { }

      ~myEvent() { }

      SDL_Event evt;
//      int x, y, dx, dy, button, device;
//      bool left, middle, right;
//      bool pressed, released;
//      long key_code;
  };
  static myEvent* cast_evt(void *obj) { return (myEvent *)obj; }

  static luabind::object safe_call(luabind::object func)
  {
    lua_State *L = func.interpreter();
    func.push(L);
    if (lua_pcall(L, 0 /* nargs */, 1 /* nreturns */, 0 /* std errror */))
    {
      fprintf(stderr, "%s\n", lua_tostring(L, -1));
    }
    luabind::object o(from_stack(L, -1));
    lua_pop(L, 1);
    return o;
  }
  static luabind::object safe_call(luabind::object func, luabind::object arg1)
  {
    lua_State *L = func.interpreter();
    func.push(L);
    arg1.push(L);
    if (lua_pcall(L, 1 /* nargs */, 1 /* nreturns */, 0 /* std errror */))
    {
      fprintf(stderr, "%s\n", lua_tostring(L, -1));
    }
    luabind::object o(from_stack(L, -1));
    lua_pop(L, 1);
    return o;
  }

  event::event() : base(), _obj(NULL)
  {
    _obj = new myEvent();
  }

  event::~event()
  {
    if (_obj) { delete cast_evt(_obj); }
  }

  std::string event::get_button() const
  {
    SDL_Event &evt = cast_evt(_obj)->evt;
    if (evt.type == SDL_MOUSEBUTTONDOWN || evt.type == SDL_MOUSEBUTTONUP)
    {
      SDL_MouseButtonEvent &mouse = (SDL_MouseButtonEvent &)evt;
      switch (mouse.button)
      {
        case SDL_BUTTON_LEFT:
          return "left";
        case SDL_BUTTON_MIDDLE:
          return "middle";
        case SDL_BUTTON_RIGHT:
          return "right";
        case SDL_BUTTON_WHEELUP:
          return "up";
        case SDL_BUTTON_WHEELDOWN:
          return "down";
        default:
          return "";
      }
    }
    return "";
  }

  int event::get_dx() const
  {
    SDL_Event &evt = cast_evt(_obj)->evt;
    if (evt.type == SDL_MOUSEMOTION)
    {
      SDL_MouseMotionEvent &motion = (SDL_MouseMotionEvent &)evt;
      return motion.xrel;
    }
    return 0;
  }

  int event::get_dy() const
  {
    SDL_Event &evt = cast_evt(_obj)->evt;
    if (evt.type == SDL_MOUSEMOTION)
    {
      SDL_MouseMotionEvent &motion = (SDL_MouseMotionEvent &)evt;
      return motion.yrel;
    }
    return 0;
  }

  std::string event::get_key() const
  {
    SDL_Event &evt = cast_evt(_obj)->evt;
    if (evt.type == SDL_KEYDOWN || evt.type == SDL_KEYUP)
    {
      SDL_KeyboardEvent &key = (SDL_KeyboardEvent &)evt;
      return input::to_keyname(key.keysym.sym);
    }
    return "";
  }

  long event::get_key_code() const
  {
    SDL_Event &evt = cast_evt(_obj)->evt;
    if (evt.type == SDL_KEYDOWN || evt.type == SDL_KEYUP)
    {
      SDL_KeyboardEvent &key = (SDL_KeyboardEvent &)evt;
//      return key.keysym.unicode;
      return key.keysym.sym;
    }
    return -1;
  }

  long event::get_scan_code() const
  {
    SDL_Event &evt = cast_evt(_obj)->evt;
    if (evt.type == SDL_KEYDOWN || evt.type == SDL_KEYUP)
    {
      SDL_KeyboardEvent &key = (SDL_KeyboardEvent &)evt;
      return key.keysym.scancode;
    }
    return -1;
  }

  int event::get_x() const
  {
    SDL_Event &evt = cast_evt(_obj)->evt;
    if (evt.type == SDL_MOUSEMOTION)
    {
      SDL_MouseMotionEvent &motion = (SDL_MouseMotionEvent &)evt;
      return motion.x;
    }
    else if (evt.type == SDL_MOUSEBUTTONDOWN || evt.type == SDL_MOUSEBUTTONUP)
    {
      SDL_MouseButtonEvent &mouse = (SDL_MouseButtonEvent &)evt;
      return mouse.x;
    }
    return -1;
  }

  int event::get_y() const
  {
    SDL_Event &evt = cast_evt(_obj)->evt;
    if (evt.type == SDL_MOUSEMOTION)
    {
      SDL_MouseMotionEvent &motion = (SDL_MouseMotionEvent &)evt;
      return motion.y;
    }
    else if (evt.type == SDL_MOUSEBUTTONDOWN || evt.type == SDL_MOUSEBUTTONUP)
    {
      SDL_MouseButtonEvent &mouse = (SDL_MouseButtonEvent &)evt;
      return mouse.y;
    }
    return -1;
  }

  bool event::is_pressed() const
  {
    SDL_Event &evt = cast_evt(_obj)->evt;
    if (evt.type == SDL_MOUSEBUTTONDOWN || evt.type == SDL_MOUSEBUTTONUP)
    {
      SDL_MouseButtonEvent &mouse = (SDL_MouseButtonEvent &)evt;
      return mouse.state == SDL_PRESSED;
    }
    return false;
  }

  bool event::is_released() const
  {
    SDL_Event &evt = cast_evt(_obj)->evt;
    if (evt.type == SDL_MOUSEBUTTONDOWN || evt.type == SDL_MOUSEBUTTONUP)
    {
      SDL_MouseButtonEvent &mouse = (SDL_MouseButtonEvent &)evt;
      return mouse.state == SDL_RELEASED;
    }
    return false;
  }

  bool event::left_is_down() const
  {
    SDL_Event &evt = cast_evt(_obj)->evt;
    if (evt.type == SDL_MOUSEMOTION)
    {
      SDL_MouseMotionEvent &motion = (SDL_MouseMotionEvent &)evt;
      return motion.state & SDL_BUTTON_LMASK;
    }
    return false;
  }

  bool event::middle_is_down() const
  {
    SDL_Event &evt = cast_evt(_obj)->evt;
    if (evt.type == SDL_MOUSEMOTION)
    {
      SDL_MouseMotionEvent &motion = (SDL_MouseMotionEvent &)evt;
      return motion.state & SDL_BUTTON_MMASK;
    }
    return false;
  }

  bool event::right_is_down() const
  {
    SDL_Event &evt = cast_evt(_obj)->evt;
    if (evt.type == SDL_MOUSEMOTION)
    {
      SDL_MouseMotionEvent &motion = (SDL_MouseMotionEvent &)evt;
      return motion.state & SDL_BUTTON_RMASK;
    }
    return false;
  }

  class mySystem
  {
    private:
      mySystem()
        : funcs(), running(true),
          on_tick(),
          on_left_down(),   on_left_up(),
          on_middle_down(), on_middle_up(),
          on_right_down(),  on_right_up(),
          timers()
      { }
    public:
      ~mySystem() { }

      static mySystem* Create()
      {
        mySystem *sys = NULL;
        try {
          sys = new mySystem;
          return sys;
        }
        catch (...) {
          delete sys;
          return NULL;
        }
      }

      bool SetRunning(bool run)
      {
        running = run;
        return true;
      }

      std::map<Uint32, luabind::object> funcs;
      luabind::object on_tick;
      luabind::object on_left_down,   on_left_up;
      luabind::object on_middle_down, on_middle_up;
      luabind::object on_right_down,  on_right_up;
      bool running;
      std::vector<timer *> timers;
  };
  static mySystem *cast_sys(void *obj) { return (mySystem *)obj; }

  system::system() : base(), _obj(NULL)
  {
  }

  system::~system()
  {
    done();
  }

  clock* system::create_clock(double freq)
  {
    clock *c = clock::create(this, freq);
    if (! c) { return NULL; }
    try {
      cast_sys(_obj)->timers.push_back(c);
      return c;
    }
    catch (...) {
      delete c;
      return NULL;
    }
  }

  timer* system::create_timer(double interval)
  {
    timer *t = timer::create(this, interval);
    if (! t) { return NULL; }
    try {
      cast_sys(_obj)->timers.push_back(t);
      return t;
    }
    catch (...) {
      delete t;
      return NULL;
    }
  }

  bool system::delay(unsigned long msec)
  {
    SDL_Delay(msec);
    return true;
  }

  bool system::done()
  {
printf("QUITING1\n");
    if (_obj)
    {
printf("QUITING2\n");
      SDL_Quit();
      delete cast_sys(_obj);
      _obj = NULL;
      return true;
    }
printf("QUITING3\n");
    return false;
  }

  bool system::detach_timer(timer *t)
  {
    if (! _obj) { return NULL; }
    std::vector<timer *> &timers = cast_sys(_obj)->timers;
    std::vector<timer *>::iterator i = timers.begin();

    for ( ; i != timers.end(); i++)
    {
      if (*i == t)
      {
printf("DETACHING TIMER!\n");
        timers.erase(i);
        return true;
      }
    }
    return false;
  }

  bool system::do_event()
  {
    SDL_Event sdl_evt;
    event e;
    std::vector<timer *> &timers = cast_sys(_obj)->timers;
    std::vector<timer *>::iterator i = timers.begin();

    for ( ; i != timers.end(); i++)
    {
      (*i)->probe();
    }

    if (SDL_PollEvent(&sdl_evt))
    {
      mySystem *sys = cast_sys(_obj);
      luabind::object f = sys->funcs[sdl_evt.type];
      cast_evt(e.get_rawobj())->evt = sdl_evt;

      if (sdl_evt.type == SDL_KEYDOWN || sdl_evt.type == SDL_KEYUP)
      {
        SDL_KeyboardEvent &keyboard = (SDL_KeyboardEvent &)sdl_evt;
      }
      else if (sdl_evt.type == SDL_MOUSEBUTTONDOWN || sdl_evt.type == SDL_MOUSEBUTTONUP)
      {
        SDL_MouseButtonEvent &mouse = (SDL_MouseButtonEvent &)sdl_evt;
        if (mouse.button == SDL_BUTTON_LEFT)
        {
          if (mouse.state == SDL_PRESSED && sys->on_left_down)
          { f = sys->on_left_down; }
          if (mouse.state == SDL_RELEASED && sys->on_left_up)
          { f = sys->on_left_up; }
        }
        if (mouse.button == SDL_BUTTON_MIDDLE)
        {
          if (mouse.state == SDL_PRESSED && sys->on_middle_down)
          { f = sys->on_middle_down; }
          if (mouse.state == SDL_RELEASED && sys->on_middle_up)
          { f = sys->on_middle_up; }
        }
        if (mouse.button == SDL_BUTTON_RIGHT)
        {
          if (mouse.state == SDL_PRESSED && sys->on_right_down)
          { f = sys->on_right_down; }
          if (mouse.state == SDL_RELEASED && sys->on_right_up)
          { f = sys->on_right_up; }
        }
      }
      else if (sdl_evt.type == SDL_MOUSEMOTION)
      {
        SDL_MouseMotionEvent &motion = (SDL_MouseMotionEvent &)sdl_evt;
      }
      else if (sdl_evt.type == SDL_QUIT)
      {
        if (! f) { set_running(false); }
      }
      else
      {
        // OTHERS
      }

      if (f && luabind::type(f) == LUA_TFUNCTION)
      {
        try {
          f(&e);
        }
        catch (...) {
          fprintf(stderr, "%s\n", lua_tostring(f.interpreter(), -1));
        }
      }
      return true;
    }
    return false;
  }

  bool system::do_events()
  {
    while (do_event()) { }
    return true;
  }

  int system::get_l(lua_State *L)
  {
    using namespace luabind;
    object o = globals(L)["lev"]["classes"]["system"]["get_c"]();
//    o["finalizer"] = globals(L)["lev"]["classes"]["finalizer"]["create"]();
    o.push(L);
    return 1;
  }

  luabind::object system::get_on_button_down()
  {
    return cast_sys(_obj)->funcs[SDL_MOUSEBUTTONDOWN];
  }

  luabind::object system::get_on_button_up()
  {
    return cast_sys(_obj)->funcs[SDL_MOUSEBUTTONUP];
  }

  luabind::object system::get_on_key_down()
  {
    return cast_sys(_obj)->funcs[SDL_KEYDOWN];
  }

  luabind::object system::get_on_key_up()
  {
    return cast_sys(_obj)->funcs[SDL_KEYUP];
  }

  luabind::object system::get_on_left_down()
  {
    return cast_sys(_obj)->on_left_down;
  }

  luabind::object system::get_on_left_up()
  {
    return cast_sys(_obj)->on_left_up;
  }

  luabind::object system::get_on_middle_down()
  {
    return cast_sys(_obj)->on_middle_down;
  }

  luabind::object system::get_on_middle_up()
  {
    return cast_sys(_obj)->on_middle_up;
  }

  luabind::object system::get_on_motion()
  {
    return cast_sys(_obj)->funcs[SDL_MOUSEMOTION];
  }

  luabind::object system::get_on_quit()
  {
    return cast_sys(_obj)->funcs[SDL_QUIT];
  }

  luabind::object system::get_on_right_down()
  {
    return cast_sys(_obj)->on_right_down;
  }

  luabind::object system::get_on_right_up()
  {
    return cast_sys(_obj)->on_right_up;
  }

  luabind::object system::get_on_tick()
  {
    return cast_sys(_obj)->on_tick;
  }

  screen* system::get_screen()
  {
    return screen::get();
  }

  unsigned long system::get_ticks()
  {
    return SDL_GetTicks();
  }

  system* system::init()
  {
    static system sys;
    if (sys._obj) { return &sys; }
printf("INITTING!\n");
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) { return NULL; }
printf("INITTED!\n");

    SDL_EnableUNICODE(1);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE,    8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,  8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,   8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,  8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    sys._obj = mySystem::Create();
    if (! sys._obj) { return NULL; }
    return &sys;
  }

  bool system::is_running()
  {
    return cast_sys(_obj)->running;
  }

  bool system::quit(bool force)
  {
    if (force)
    {
      set_running(false);
    }
    else
    {
      SDL_Event q;
      q.type = SDL_QUIT;
      SDL_PushEvent(&q);
    }
    return true;
  }

  bool system::run()
  {
    mySystem *sys = cast_sys(_obj);
    while (is_running())
    {
//printf("RUNNING NOW!\n");
      if (sys->on_tick && luabind::type(sys->on_tick) == LUA_TFUNCTION)
      {
//printf("BEGIN CALL!\n");
        safe_call(sys->on_tick);
//printf("END CALL!\n");
      }
//printf("BEGIN EVENT!\n");
      do_events();
//printf("END EVENT!\n\n");
    }
    return true;
  }

  bool system::set_on_button_down(luabind::object func)
  {
    cast_sys(_obj)->funcs[SDL_MOUSEBUTTONDOWN] = func;
    return true;
  }

  bool system::set_on_button_up(luabind::object func)
  {
    cast_sys(_obj)->funcs[SDL_MOUSEBUTTONUP] = func;
    return true;
  }

  bool system::set_on_key_down(luabind::object func)
  {
    cast_sys(_obj)->funcs[SDL_KEYDOWN] = func;
    return true;
  }

  bool system::set_on_key_up(luabind::object func)
  {
    cast_sys(_obj)->funcs[SDL_KEYUP] = func;
    return true;
  }

  bool system::set_on_left_down(luabind::object func)
  {
    cast_sys(_obj)->on_left_down = func;
    return true;
  }

  bool system::set_on_left_up(luabind::object func)
  {
    cast_sys(_obj)->on_left_up = func;
    return true;
  }

  bool system::set_on_middle_down(luabind::object func)
  {
    cast_sys(_obj)->on_middle_down = func;
    return true;
  }

  bool system::set_on_middle_up(luabind::object func)
  {
    cast_sys(_obj)->on_middle_up = func;
    return true;
  }

  bool system::set_on_motion(luabind::object func)
  {
    cast_sys(_obj)->funcs[SDL_MOUSEMOTION] = func;
    return true;
  }

  bool system::set_on_quit(luabind::object func)
  {
    cast_sys(_obj)->funcs[SDL_QUIT] = func;
    return true;
  }

  bool system::set_on_right_down(luabind::object func)
  {
    cast_sys(_obj)->on_right_down = func;
    return true;
  }

  bool system::set_on_right_up(luabind::object func)
  {
    cast_sys(_obj)->on_right_up = func;
    return true;
  }

  bool system::set_on_tick(luabind::object func)
  {
    cast_sys(_obj)->on_tick = func;
    return true;
  }

  bool system::set_running(bool run)
  {
    return cast_sys(_obj)->SetRunning(run);
  }

  screen* system::set_video_mode(int width, int height, int depth)
  {
    return screen::set_mode(width, height, depth);
  }

  bool system::toggle_full_screen()
  {
    screen* s = system::get_screen();
    if (! s) { return false; }
    return SDL_WM_ToggleFullScreen((SDL_Surface *)s->get_rawobj());
  }

}

