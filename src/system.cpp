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
#include "lev/debug.hpp"
#include "lev/draw.hpp"
#include "lev/screen.hpp"
#include "lev/sound.hpp"
#include "lev/timer.hpp"
#include "lev/util.hpp"
#include "register.hpp"

// libraries
#include <map>
#include <luabind/luabind.hpp>
#include <luabind/raw_policy.hpp>
#include <SDL2/SDL.h>
#include <vector>

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
  globals(L)["require"]("lev.debug");

  module(L, "lev")
  [
    namespace_("system"),
    namespace_("classes")
    [
      class_<event, base, boost::shared_ptr<base> >("event")
        .property("button", &event::get_button)
        .property("dx", &event::get_dx)
        .property("dy", &event::get_dy)
        .property("id", &event::get_id)
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
      class_<lev::system, base, boost::shared_ptr<base> >("system")
        .def("close", &system::done)
        .def("create_mixer", &mixer::init)
        .property("dbg", &system::get_debugger)
        .property("debugger", &system::get_debugger)
        .def("delay", &system::delay)
        .def("do_event", &system::do_event)
        .def("do_events", &system::do_events)
        .def("done", &system::done)
        .property("is_debugging", &system::is_debugging)
        .property("is_running", &system::is_running, &system::set_running)
        .def("mixer", &mixer::init)
        .property("name", &system::get_name, &system::set_name)
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
        .def("set_running", &system::set_running)
        .def("start_debug", &system::start_debug)
        .def("stop_debug", &system::stop_debug)
        .property("ticks", &system::get_ticks)
        .scope
        [
          def("get", &system::get),
          def("init", &system::init, raw(_1))
        ]
    ]
  ];
  object lev = globals(L)["lev"];
  object classes = lev["classes"];
  lev["system"] = classes["system"]["init"];

  // post-requirement
  globals(L)["require"]("lev.screen");
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
      lev::debug_print(lua_tostring(L, -1));
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
      lev::debug_print(lua_tostring(L, -1));
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
        case SDL_BUTTON_X1:
          return "up";
        case SDL_BUTTON_X2:
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

  int event::get_id() const
  {
    SDL_WindowEvent &evt = (SDL_WindowEvent &)cast_evt(_obj)->evt;
    return evt.windowID;
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
    else if (evt.type == SDL_MOUSEWHEEL)
    {
      SDL_MouseWheelEvent &wheel = (SDL_MouseWheelEvent &)evt;
      return wheel.x;
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
    else if (evt.type == SDL_MOUSEWHEEL)
    {
      SDL_MouseWheelEvent &wheel = (SDL_MouseWheelEvent &)evt;
      return wheel.y;
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

  class system_core
  {
    public:
      typedef boost::shared_ptr<system_core> ptr;
      static system_core::ptr singleton;
    protected:
      system_core(lua_State *L) :
        dbg(),
        funcs(), name("lev"), running(true),
        on_tick(),
        on_left_down(),   on_left_up(),
        on_middle_down(), on_middle_up(),
        on_right_down(),  on_right_up(),
        screens(),
        timers(),
        L(L)
      { }
    public:

      static system_core::ptr init(lua_State *L)
      {
        if (singleton) { return singleton; }
        try {
          singleton.reset(new system_core(L));
          if (! singleton) { throw -1; }
printf("INITTING!\n");
          if (SDL_Init(SDL_INIT_EVERYTHING) < 0) { throw -2; }
printf("INITTED!\n");

          // Fullscreen feature of SDL is not yet completed.
          //SDL_DisplayMode mode = { 0 };
          //SDL_SetFullscreenDisplayMode(&mode);

          SDL_GL_SetAttribute(SDL_GL_RED_SIZE,    8);
          SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,  8);
          SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,   8);
          SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,  8);
          SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
          SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        }
        catch (...) {
          singleton.reset();
        }
        return singleton;
      }

      virtual ~system_core()
      {
printf("QUITING1\n");
        dbg.reset();
printf("QUITING2\n");
        SDL_Quit();
printf("QUITING3\n");
      }

      lua_State *L;
      debugger::ptr dbg;
      std::map<Uint32, luabind::object> funcs;
      std::string name;
      luabind::object on_tick;
      luabind::object on_left_down,   on_left_up;
      luabind::object on_middle_down, on_middle_up;
      luabind::object on_right_down,  on_right_up;
      std::map<Uint32, boost::weak_ptr<screen> > screens;
      std::vector<boost::weak_ptr<timer> > timers;
      bool running;
  };
  system_core::ptr system_core::singleton;

  class impl_system : public system
  {
    public:
      typedef boost::shared_ptr<impl_system> ptr;
    protected:
      impl_system() : core() { }

    public:
      virtual ~impl_system()
      {
        done();
      }

      virtual bool attach(debugger::ptr d)
      {
        if (! core || ! d) { return false; }
        core->dbg = d;
        return true;
      }

      virtual bool attach(boost::shared_ptr<screen> s)
      {
        if (! core) { return false; }
        if (s->get_id() >= 0)
        {
          core->screens[s->get_id()] = s;
          return true;
        }
        return false;
      }

      virtual bool attach(boost::shared_ptr<timer> t)
      {
        if (! core) { return false; }
        if (! t) { return false; }
printf("ATTACH TIMER!\n");
        core->timers.push_back(t);
        return true;
      }

      virtual bool delay(unsigned long msec = 1000)
      {
        if (! core) { return false; }
        SDL_Delay(msec);
        return true;
      }

      virtual bool do_event()
      {
        if (! core) { return false; }
        SDL_Event sdl_evt;
        event e;
        std::vector<boost::weak_ptr<timer> >::iterator i = core->timers.begin();
        for ( ; i != core->timers.end(); i++)
        {
          if (boost::shared_ptr<timer> t = i->lock())
          {
printf("PROVING!\n");
            t->probe();
          }
          else
          {
printf("DETACHING TIMER!\n");
            core->timers.erase(i);
            break;
          }
        }

        if (SDL_PollEvent(&sdl_evt))
        {
          luabind::object f;
          cast_evt(e.get_rawobj())->evt = sdl_evt;

          if (sdl_evt.type == SDL_KEYDOWN || sdl_evt.type == SDL_KEYUP)
          {
            SDL_KeyboardEvent &keyboard = (SDL_KeyboardEvent &)sdl_evt;
            if (boost::shared_ptr<screen> s = core->screens[keyboard.windowID].lock())
            {
              if (sdl_evt.type == SDL_KEYDOWN) { f = s->get_on_key_down(); }
              else { f = s->get_on_key_up(); }
            }
          }
          else if (sdl_evt.type == SDL_MOUSEBUTTONDOWN || sdl_evt.type == SDL_MOUSEBUTTONUP)
          {
            SDL_MouseButtonEvent &mouse = (SDL_MouseButtonEvent &)sdl_evt;
            if (boost::shared_ptr<screen> s = core->screens[mouse.windowID].lock())
            {
              if (mouse.button == SDL_BUTTON_LEFT)
              {
                if (mouse.state == SDL_PRESSED)       { f = s->get_on_left_down(); }
                else if (mouse.state == SDL_RELEASED) { f = s->get_on_left_up(); }
              }
              else if (mouse.button == SDL_BUTTON_MIDDLE)
              {
                if (mouse.state == SDL_PRESSED)       { f = s->get_on_middle_down(); }
                else if (mouse.state == SDL_RELEASED) { f = s->get_on_middle_up(); }
              }
              else if (mouse.button == SDL_BUTTON_RIGHT)
              {
                if (mouse.state == SDL_PRESSED)       { f = s->get_on_right_down(); }
                else if (mouse.state == SDL_RELEASED) { f = s->get_on_right_up(); }
              }
            }
            if (! f.is_valid())
            {
              if (mouse.button == SDL_BUTTON_LEFT)
              {
                if (mouse.state == SDL_PRESSED)       { f = core->on_left_down; }
                else if (mouse.state == SDL_RELEASED) { f = core->on_left_up; }
              }
              if (mouse.button == SDL_BUTTON_MIDDLE)
              {
                if (mouse.state == SDL_PRESSED)       { f = core->on_middle_down; }
                else if (mouse.state == SDL_RELEASED) { f = core->on_middle_up; }
              }
              if (mouse.button == SDL_BUTTON_RIGHT)
              {
                if (mouse.state == SDL_PRESSED)       { f = core->on_right_down; }
                else if (mouse.state == SDL_RELEASED) { f = core->on_right_up; }
              }
            }
          }
          else if (sdl_evt.type == SDL_MOUSEMOTION)
          {
            SDL_MouseMotionEvent &motion = (SDL_MouseMotionEvent &)sdl_evt;
            if (boost::shared_ptr<screen> s = core->screens[motion.windowID].lock())
            {
              f = s->get_on_motion();
            }
          }
          else if (sdl_evt.type == SDL_MOUSEWHEEL)
          {
            SDL_MouseWheelEvent &wheel = (SDL_MouseWheelEvent &)sdl_evt;
            if (boost::shared_ptr<screen> s = core->screens[wheel.windowID].lock())
            {
              f = s->get_on_wheel();
            }
          }
          else if (sdl_evt.type == SDL_WINDOWEVENT)
          {
            if (screen::ptr s = core->screens[sdl_evt.window.windowID].lock())
            {
              if (sdl_evt.window.event == SDL_WINDOWEVENT_CLOSE)
              {
                f = s->get_on_close();
                if (! f.is_valid())
                {
                  s->close();
                }
              }
            }
          }
          else if (sdl_evt.type == SDL_QUIT)
          {
            f = core->funcs[SDL_QUIT];
            if (! f.is_valid()) { set_running(false); }
          }
          else
          {
            // OTHERS
          }

          if (! f.is_valid())
          {
            f = core->funcs[sdl_evt.type];
          }
          if (f.is_valid() && luabind::type(f) == LUA_TFUNCTION)
          {
            try {
              f(&e);
            }
            catch (...) {
              lev::debug_print(lua_tostring(core->L, -1));
              lev::debug_print("error on event processing\n");
            }
          }
          return true;
        }
        return false;
      }

      virtual bool do_events()
      {
        if (! core) { return false; }
        while (do_event()) { }
        return true;
      }

      bool done()
      {
        if (! core) { return false; }
//printf("DONE CORE COUNT: %ld\n", system_core::singleton.use_count());
        core.reset();
//printf("RESETTED CORE COUNT: %ld\n", system_core::singleton.use_count());
        if (system_core::singleton.use_count() == 1)
        {
printf("SINGLETON CLEAR!\n");
          system_core::singleton.reset();
        }
        return true;
      }

      static impl_system::ptr init(lua_State *L)
      {
        impl_system::ptr sys;
        if (! L) { return sys; }
        try {
          sys.reset(new impl_system);
          if (! sys) { throw -1; }
          sys->core = system_core::init(L);
          if (! sys->core) { throw -2; }
//printf("INIT CORE COUNT: %ld\n", sys->core.use_count());
        }
        catch (...) {
          lev::debug_print("error on system initialization");
          sys.reset();
        }
        return sys;
      }

      static impl_system::ptr get()
      {
        impl_system::ptr sys;
        if (! system_core::singleton) { return sys; }
        try {
          sys.reset(new impl_system);
          if (! sys) { throw -1; }
          sys->core = system_core::singleton;
//printf("GET CORE COUNT: %ld\n", sys->core.use_count());
        }
        catch (...) {
          lev::debug_print("error on system instance cloning");
          sys.reset();
        }
        return sys;
      }

      virtual debugger::ptr get_debugger()
      {
        if (! core) { return debugger::ptr(); }
        return core->dbg;
      }

      virtual std::string get_name() const
      {
        if (! core) { return ""; }
        return core->name;
      }

      virtual luabind::object get_on_button_down()
      {
        if (! core) { return luabind::object(); }
        return core->funcs[SDL_MOUSEBUTTONDOWN];
      }

      virtual luabind::object get_on_button_up()
      {
        if (! core) { return luabind::object(); }
        return core->funcs[SDL_MOUSEBUTTONUP];
      }

      virtual luabind::object get_on_key_down()
      {
        if (! core) { return luabind::object(); }
        return core->funcs[SDL_KEYDOWN];
      }

      virtual luabind::object get_on_key_up()
      {
        if (! core) { return luabind::object(); }
        return core->funcs[SDL_KEYUP];
      }

      virtual luabind::object get_on_left_down()
      {
        if (! core) { return luabind::object(); }
        return core->on_left_down;
      }

      virtual luabind::object get_on_left_up()
      {
        if (! core) { return luabind::object(); }
        return core->on_left_up;
      }

      virtual luabind::object get_on_middle_down()
      {
        if (! core) { return luabind::object(); }
        return core->on_middle_down;
      }

      virtual luabind::object get_on_middle_up()
      {
        if (! core) { return luabind::object(); }
        return core->on_middle_up;
      }

      virtual luabind::object get_on_motion()
      {
        if (! core) { return luabind::object(); }
        return core->funcs[SDL_MOUSEMOTION];
      }

      virtual luabind::object get_on_quit()
      {
        if (! core) { return luabind::object(); }
        return core->funcs[SDL_QUIT];
      }

      virtual luabind::object get_on_right_down()
      {
        if (! core) { return luabind::object(); }
        return core->on_right_down;
      }

      virtual luabind::object get_on_right_up()
      {
        if (! core) { return luabind::object(); }
        return core->on_right_up;
      }

      virtual luabind::object get_on_tick()
      {
        if (! core) { return luabind::object(); }
        return core->on_tick;
      }

      virtual unsigned long get_ticks() const
      {
        return SDL_GetTicks();
      }

      virtual bool is_debugging() const
      {
        if (! core) { return false; }
        return core->dbg;
      }

      virtual bool is_running() const
      {
        if (! core) { return false; }
        return core->running;
      }

      virtual bool quit(bool force)
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

      virtual bool run()
      {
        if (! core) { return false; }
        core->running = true;
        while (is_running())
        {
          try {
            if (core->on_tick && luabind::type(core->on_tick) == LUA_TFUNCTION)
            {
              core->on_tick();
    //          safe_call(core->on_tick);
            }
            do_events();
          }
          catch (...) {
            lev::debug_print(lua_tostring(core->L, -1));
            lev::debug_print("error on system::run");
            return false;
          }
        }
        return true;
      }

      virtual bool set_name(const std::string &name)
      {
        if (! core) { return false; }
        core->name = name;
        return true;
      }

      virtual bool set_on_button_down(luabind::object func)
      {
        if (! core) { return false; }
        core->funcs[SDL_MOUSEBUTTONDOWN] = func;
        return true;
      }

      virtual bool set_on_button_up(luabind::object func)
      {
        if (! core) { return false; }
        core->funcs[SDL_MOUSEBUTTONUP] = func;
        return true;
      }

      virtual bool set_on_key_down(luabind::object func)
      {
        if (! core) { return false; }
        core->funcs[SDL_KEYDOWN] = func;
        return true;
      }

      virtual bool set_on_key_up(luabind::object func)
      {
        if (! core) { return false; }
        core->funcs[SDL_KEYUP] = func;
        return true;
      }

      virtual bool set_on_left_down(luabind::object func)
      {
        if (! core) { return false; }
        core->on_left_down = func;
        return true;
      }

      virtual bool set_on_left_up(luabind::object func)
      {
        if (! core) { return false; }
        core->on_left_up = func;
        return true;
      }

      virtual bool set_on_middle_down(luabind::object func)
      {
        if (! core) { return false; }
        core->on_middle_down = func;
        return true;
      }

      virtual bool set_on_middle_up(luabind::object func)
      {
        if (! core) { return false; }
        core->on_middle_up = func;
        return true;
      }

      virtual bool set_on_motion(luabind::object func)
      {
        if (! core) { return false; }
        core->funcs[SDL_MOUSEMOTION] = func;
        return true;
      }

      virtual bool set_on_quit(luabind::object func)
      {
        if (! core) { return false; }
        core->funcs[SDL_QUIT] = func;
        return true;
      }

      virtual bool set_on_right_down(luabind::object func)
      {
        if (! core) { return false; }
        core->on_right_down = func;
        return true;
      }

      virtual bool set_on_right_up(luabind::object func)
      {
        if (! core) { return false; }
        core->on_right_up = func;
        return true;
      }

      virtual bool set_on_tick(luabind::object func)
      {
        if (! core) { return false; }
        core->on_tick = func;
        return true;
      }

      virtual bool set_running(bool run)
      {
        if (! core) { return false; }
        core->running = run;
        return true;
      }

      virtual debugger::ptr start_debug()
      {
        return debugger::start();
      }

      virtual bool stop_debug()
      {
        if (! core) { return false; }
        if (! core->dbg) { return false; }
        core->dbg.reset();
        return true;
      }

      system_core::ptr core;
  };

  system::ptr system::get()
  {
    return impl_system::get();
  }

  system::ptr system::init(lua_State *L)
  {
    return impl_system::init(L);
  }

  lua_State *system::get_interpreter()
  {
    if (! system_core::singleton) { return NULL; }
    return system_core::singleton->L;
  }

}

