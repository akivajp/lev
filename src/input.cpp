/////////////////////////////////////////////////////////////////////////////
// Name:        lev/input.cpp
// Purpose:     source for input management class
// Author:      Akiva Miura <akiva.miura@gmail.com>
// Modified by:
// Created:     06/20/2012
// Copyright:   (C) 2010-2012 Akiva Miura
// Licence:     MIT License
/////////////////////////////////////////////////////////////////////////////

// pre-compiled header
#include "prec.h"

// declarations
#include "lev/input.hpp"

// dependencies
#include "lev/debug.hpp"

// libraries
#include <allegro5/allegro.h>

int luaopen_lev_input(lua_State *L)
{
  using namespace luabind;
  using namespace lev;

  open(L);
  // beginning of loading
  globals(L)["package"]["loaded"]["lev.input"] = true;
  // pre-requirement
  globals(L)["require"]("lev.system");

  module(L, "lev")
  [
    namespace_("classes")
    [
      class_<mouse, base, boost::shared_ptr<base> >("mouse")
        .def("button_is_down", &mouse::button_is_down)
        .def("get_axis", &mouse::get_axis)
        .property("buttons",   &mouse::get_buttons)
        .property("left_is_down",   &mouse::left_is_down)
        .property("middle_is_down", &mouse::middle_is_down)
        .property("num_axes", &mouse::get_num_axes)
        .property("num_buttons", &mouse::get_num_buttons)
        .property("right_is_down",  &mouse::right_is_down)
        .property("x", &mouse::get_x)
        .property("y", &mouse::get_y)
        .property("z", &mouse::get_z),
      class_<input, base, boost::shared_ptr<base> >("input")
        .property("mouse", &input::get_mouse_state)
        .property("mouse_state", &input::get_mouse_state)
    ]
  ];

  globals(L)["package"]["loaded"]["lev.input"] = true;
  return 0;
}
namespace lev
{

  static ALLEGRO_MOUSE_STATE* cast_mouse(void *obj)
  {
    return (ALLEGRO_MOUSE_STATE *)obj;
  }

  mouse::mouse() : base(), _obj(NULL) { }

  mouse::~mouse()
  {
    if (_obj)
    {
      delete cast_mouse(_obj);
      _obj = NULL;
    }
  }

  bool mouse::button_is_down(int button)
  {
    return al_mouse_button_down(cast_mouse(_obj), button);
  }

  boost::shared_ptr<mouse> mouse::get()
  {
    boost::shared_ptr<mouse> m;
    ALLEGRO_MOUSE_STATE *state = NULL;
    try {
      m.reset(new mouse);
      if (! m) { throw -1; }
      m->_obj = state = new ALLEGRO_MOUSE_STATE;
      if (! m->_obj) { throw -2; }
      al_get_mouse_state(state);
    }
    catch (...) {
      m.reset();
      lev::debug_print("error on mouse state instance creation");
    }
    return m;
  }

  int mouse::get_axis(int axis)
  {
    return al_get_mouse_state_axis(cast_mouse(_obj), axis);
  }

  int mouse::get_buttons()
  {
    return cast_mouse(_obj)->buttons;
  }

  int mouse::get_num_axes()
  {
    return al_get_mouse_num_axes();
  }

  int mouse::get_num_buttons()
  {
    return al_get_mouse_num_buttons();
  }

  bool mouse::left_is_down()   { return mouse::get_buttons() & 1; }
  bool mouse::middle_is_down() { return mouse::get_buttons() & 4; }
  bool mouse::right_is_down()  { return mouse::get_buttons() & 2; }

  int mouse::get_w() { return cast_mouse(_obj)->w; }
  int mouse::get_x() { return cast_mouse(_obj)->x; }
  int mouse::get_y() { return cast_mouse(_obj)->y; }
  int mouse::get_z() { return cast_mouse(_obj)->z; }


  boost::shared_ptr<input> input::get()
  {
    boost::shared_ptr<input> in;
    try {
      in.reset(new input);
      if (! in) { throw -1; }
    }
    catch (...) {
      in.reset();
      lev::debug_print("error on input instance creation");
    }
    return in;
  }

  const char *input::to_keyname(long code)
  {
    static std::map<long, std::string> *keymap = NULL;

    if (keymap == NULL)
    {
      try {
        keymap = new std::map<long, std::string>;

        (*keymap)[ALLEGRO_KEY_A]            = "a";
        (*keymap)[ALLEGRO_KEY_B]            = "b";
        (*keymap)[ALLEGRO_KEY_C]            = "c";
        (*keymap)[ALLEGRO_KEY_D]            = "d";
        (*keymap)[ALLEGRO_KEY_E]            = "e";
        (*keymap)[ALLEGRO_KEY_F]            = "f";
        (*keymap)[ALLEGRO_KEY_G]            = "g";
        (*keymap)[ALLEGRO_KEY_H]            = "h";
        (*keymap)[ALLEGRO_KEY_I]            = "i";
        (*keymap)[ALLEGRO_KEY_J]            = "j";
        (*keymap)[ALLEGRO_KEY_K]            = "k";
        (*keymap)[ALLEGRO_KEY_L]            = "l";
        (*keymap)[ALLEGRO_KEY_M]            = "m";
        (*keymap)[ALLEGRO_KEY_N]            = "n";
        (*keymap)[ALLEGRO_KEY_O]            = "o";
        (*keymap)[ALLEGRO_KEY_P]            = "p";
        (*keymap)[ALLEGRO_KEY_Q]            = "q";
        (*keymap)[ALLEGRO_KEY_R]            = "r";
        (*keymap)[ALLEGRO_KEY_S]            = "s";
        (*keymap)[ALLEGRO_KEY_T]            = "t";
        (*keymap)[ALLEGRO_KEY_U]            = "u";
        (*keymap)[ALLEGRO_KEY_V]            = "v";
        (*keymap)[ALLEGRO_KEY_W]            = "w";
        (*keymap)[ALLEGRO_KEY_X]            = "x";
        (*keymap)[ALLEGRO_KEY_Y]            = "y";
        (*keymap)[ALLEGRO_KEY_Z]            = "z";

        (*keymap)[ALLEGRO_KEY_0]            = "0";
        (*keymap)[ALLEGRO_KEY_1]            = "1";
        (*keymap)[ALLEGRO_KEY_2]            = "2";
        (*keymap)[ALLEGRO_KEY_3]            = "3";
        (*keymap)[ALLEGRO_KEY_4]            = "4";
        (*keymap)[ALLEGRO_KEY_5]            = "5";
        (*keymap)[ALLEGRO_KEY_6]            = "6";
        (*keymap)[ALLEGRO_KEY_7]            = "7";
        (*keymap)[ALLEGRO_KEY_8]            = "8";
        (*keymap)[ALLEGRO_KEY_9]            = "9";

        (*keymap)[ALLEGRO_KEY_F1]           = "f1";
        (*keymap)[ALLEGRO_KEY_F2]           = "f2";
        (*keymap)[ALLEGRO_KEY_F3]           = "f3";
        (*keymap)[ALLEGRO_KEY_F4]           = "f4";
        (*keymap)[ALLEGRO_KEY_F5]           = "f5";
        (*keymap)[ALLEGRO_KEY_F6]           = "f6";
        (*keymap)[ALLEGRO_KEY_F7]           = "f7";
        (*keymap)[ALLEGRO_KEY_F8]           = "f8";
        (*keymap)[ALLEGRO_KEY_F9]           = "f9";
        (*keymap)[ALLEGRO_KEY_F10]          = "f10";
        (*keymap)[ALLEGRO_KEY_F11]          = "f11";
        (*keymap)[ALLEGRO_KEY_F12]          = "f12";
        (*keymap)[ALLEGRO_KEY_PAD_0]        = "pad0";
        (*keymap)[ALLEGRO_KEY_PAD_1]        = "pad1";
        (*keymap)[ALLEGRO_KEY_PAD_2]        = "pad2";
        (*keymap)[ALLEGRO_KEY_PAD_3]        = "pad3";
        (*keymap)[ALLEGRO_KEY_PAD_4]        = "pad4";
        (*keymap)[ALLEGRO_KEY_PAD_5]        = "pad5";
        (*keymap)[ALLEGRO_KEY_PAD_6]        = "pad6";
        (*keymap)[ALLEGRO_KEY_PAD_7]        = "pad7";
        (*keymap)[ALLEGRO_KEY_PAD_8]        = "pad8";
        (*keymap)[ALLEGRO_KEY_PAD_9]        = "pad9";

        (*keymap)[ALLEGRO_KEY_ESCAPE]       = "escape";
        (*keymap)[ALLEGRO_KEY_TILDE]        = "tilde";
        (*keymap)[ALLEGRO_KEY_MINUS]        = "minus";
        (*keymap)[ALLEGRO_KEY_EQUALS]       = "equal";
        (*keymap)[ALLEGRO_KEY_BACKSPACE]    = "backspace";
        (*keymap)[ALLEGRO_KEY_TAB]          = "tab";
        (*keymap)[ALLEGRO_KEY_OPENBRACE]    = "leftbracket";
        (*keymap)[ALLEGRO_KEY_CLOSEBRACE]   = "rightbracket";
        (*keymap)[ALLEGRO_KEY_ENTER]        = "enter";
        (*keymap)[ALLEGRO_KEY_SEMICOLON]    = "semicolon";
        (*keymap)[ALLEGRO_KEY_QUOTE]        = "quote";
        (*keymap)[ALLEGRO_KEY_BACKSLASH]    = "backslash";
        (*keymap)[ALLEGRO_KEY_BACKSLASH2]   = "backslash2";
        (*keymap)[ALLEGRO_KEY_COMMA]        = "comma";
        (*keymap)[ALLEGRO_KEY_FULLSTOP]     = "fullstop";
        (*keymap)[ALLEGRO_KEY_SLASH]        = "slash";
        (*keymap)[ALLEGRO_KEY_SPACE]        = "space";

        (*keymap)[ALLEGRO_KEY_INSERT]       = "insert";
        (*keymap)[ALLEGRO_KEY_DELETE]       = "delete";
        (*keymap)[ALLEGRO_KEY_HOME]         = "home";
        (*keymap)[ALLEGRO_KEY_END]          = "end";
        (*keymap)[ALLEGRO_KEY_PGUP]         = "pgup";
        (*keymap)[ALLEGRO_KEY_PGDN]         = "pgdn";
        (*keymap)[ALLEGRO_KEY_LEFT]         = "left";
        (*keymap)[ALLEGRO_KEY_RIGHT]        = "right";
        (*keymap)[ALLEGRO_KEY_UP]           = "up";
        (*keymap)[ALLEGRO_KEY_DOWN]         = "down";

        (*keymap)[ALLEGRO_KEY_PAD_SLASH]    = "pad_slash";
        (*keymap)[ALLEGRO_KEY_PAD_ASTERISK] = "pad_asterisk";
        (*keymap)[ALLEGRO_KEY_PAD_MINUS]    = "pad_minus";
        (*keymap)[ALLEGRO_KEY_PAD_PLUS]     = "pad_plus";
        (*keymap)[ALLEGRO_KEY_PAD_DELETE]   = "pad_delete";
        (*keymap)[ALLEGRO_KEY_PAD_ENTER]    = "pad_enter";

        (*keymap)[ALLEGRO_KEY_PRINTSCREEN]  = "printscreen";
        (*keymap)[ALLEGRO_KEY_PAUSE]        = "pause";

        (*keymap)[ALLEGRO_KEY_ABNT_C1]      = "abnt_c1";
        (*keymap)[ALLEGRO_KEY_YEN]          = "yen";
        (*keymap)[ALLEGRO_KEY_KANA]         = "kana";
        (*keymap)[ALLEGRO_KEY_CONVERT]      = "convert";
        (*keymap)[ALLEGRO_KEY_NOCONVERT]    = "noconvert";
        (*keymap)[ALLEGRO_KEY_AT]           = "at";
        (*keymap)[ALLEGRO_KEY_CIRCUMFLEX]   = "circumflex";
        (*keymap)[ALLEGRO_KEY_COLON2]       = "colon2";
        (*keymap)[ALLEGRO_KEY_KANJI]        = "kanji";

        (*keymap)[ALLEGRO_KEY_PAD_EQUALS]   = "pad_equal";
        (*keymap)[ALLEGRO_KEY_BACKQUOTE]    = "backquote";
        (*keymap)[ALLEGRO_KEY_SEMICOLON2]   = "semicolon2";
        (*keymap)[ALLEGRO_KEY_COMMAND]      = "command";
        (*keymap)[ALLEGRO_KEY_UNKNOWN]      = "unknown";

        (*keymap)[ALLEGRO_KEY_LSHIFT]       = "lshift";
        (*keymap)[ALLEGRO_KEY_RSHIFT]       = "rshift";
        (*keymap)[ALLEGRO_KEY_LCTRL]        = "lctrl";
        (*keymap)[ALLEGRO_KEY_RCTRL]        = "rctrl";
        (*keymap)[ALLEGRO_KEY_ALT]          = "lalt";
        (*keymap)[ALLEGRO_KEY_ALTGR]        = "ralt";
        (*keymap)[ALLEGRO_KEY_LWIN]         = "lwin";
        (*keymap)[ALLEGRO_KEY_RWIN]         = "rwin";
        (*keymap)[ALLEGRO_KEY_MENU]         = "menu";
        (*keymap)[ALLEGRO_KEY_SCROLLLOCK]   = "scrolllock";
        (*keymap)[ALLEGRO_KEY_NUMLOCK]      = "numlock";
        (*keymap)[ALLEGRO_KEY_CAPSLOCK]     = "capslock";

        (*keymap)[123]                      = "fn";
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

}

