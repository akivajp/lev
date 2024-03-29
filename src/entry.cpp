/////////////////////////////////////////////////////////////////////////////
// Name:        src/entry.cpp
// Purpose:     source for binding the C++ library to lua script
// Author:      Akiva Miura <akiva.miura@gmail.com>
// Created:     12/01/2010
// Copyright:   (C) 2010-2011 Akiva Miura
// Licence:     MIT License
/////////////////////////////////////////////////////////////////////////////

// pre-compiled header
#include "prec.h"

// dependencies
#include "lev/lev.hpp"
#include "lev/entry.hpp"

// libraries
// ...

int luaopen_lev(lua_State *L)
{
  using namespace lev;
  using namespace luabind;

  open(L);

  globals(L)["require"]("lev.base");
  globals(L)["package"]["loaded"]["lev"] = globals(L)["lev"];
  set_preloaders(L);
  return 0;
}

int luaopen_lev_std(lua_State *L)
{
  using namespace luabind;
  using namespace lev;

  try {
    open(L);
    globals(L)["require"]("debug");
    globals(L)["require"]("io");
    globals(L)["require"]("math");
    globals(L)["require"]("os");
    globals(L)["require"]("string");
    globals(L)["require"]("table");

    globals(L)["require"]("lev");
    globals(L)["require"]("lev.archive");
  //  globals(L)["require"]("lev.db");
    globals(L)["require"]("lev.debug");
    globals(L)["require"]("lev.draw");
  //  globals(L)["require"]("lev.gui");
    globals(L)["require"]("lev.font");
    globals(L)["require"]("lev.fs");
  //  globals(L)["require"]("lev.gl");
    globals(L)["require"]("lev.image");
  //  globals(L)["require"]("lev.info");
    globals(L)["require"]("lev.map");
  //  globals(L)["require"]("lev.net");
    globals(L)["require"]("lev.package");
    globals(L)["require"]("lev.screen");
    globals(L)["require"]("lev.sound");
    globals(L)["require"]("lev.string");
    globals(L)["require"]("lev.system");
    globals(L)["require"]("lev.timer");
    globals(L)["require"]("lev.util");

    globals(L)["system"] = globals(L)["lev"]["system"]();
    globals(L)["mixer"]  = globals(L)["lev"]["mixer"]();
    globals(L)["package"]["loaded"]["lev.std"] = globals(L)["lev"];
    globals(L)["collectgarbage"]();
  }
  catch (...) {
    lev::debug_print(lua_tostring(L, -1));
    lev::debug_print("error on initializing \"lev.std\" library");
  }
  return 0;
}

namespace lev
{

  bool load_to(luabind::object to, const char *funcname, const char *code)
  {
    lua_State *L = to.interpreter();
    if (luaL_loadstring(L, code) != 0) { return false; }
    luabind::object loaded(luabind::from_stack(L, -1));
    lua_pop(L, 1);
    to[funcname] = loaded();
    return true;
  }

  bool register_to(luabind::object to, const char *funcname, lua_CFunction func)
  {
    lua_State *L = to.interpreter();
    lua_pushcfunction(L, func);
    luabind::object f(luabind::from_stack(L, -1));
    lua_pop(L, 1);
    to[funcname] = f;
    return true;
  }

  void set_preloaders(lua_State *L)
  {
    using namespace luabind;
    register_to(globals(L)["package"]["preload"], "lev", luaopen_lev);
    register_to(globals(L)["package"]["preload"], "lev.archive", luaopen_lev_archive);
    register_to(globals(L)["package"]["preload"], "lev.base", luaopen_lev_base);
//    register_to(globals(L)["package"]["preload"], "lev.db", luaopen_lev_db);
    register_to(globals(L)["package"]["preload"], "lev.debug", luaopen_lev_debug);
    register_to(globals(L)["package"]["preload"], "lev.draw", luaopen_lev_draw);
    register_to(globals(L)["package"]["preload"], "lev.font", luaopen_lev_font);
    register_to(globals(L)["package"]["preload"], "lev.fs", luaopen_lev_fs);
//    register_to(globals(L)["package"]["preload"], "lev.gl", luaopen_lev_gl);
//    register_to(globals(L)["package"]["preload"], "lev.gui", luaopen_lev_gui);
    register_to(globals(L)["package"]["preload"], "lev.image", luaopen_lev_image);
    register_to(globals(L)["package"]["preload"], "lev.map", luaopen_lev_map);
//    register_to(globals(L)["package"]["preload"], "lev.info", luaopen_lev_info);
//    register_to(globals(L)["package"]["preload"], "lev.input", luaopen_lev_input);
//    register_to(globals(L)["package"]["preload"], "lev.locale", luaopen_lev_locale);
//    register_to(globals(L)["package"]["preload"], "lev.net", luaopen_lev_net);
    register_to(globals(L)["package"]["preload"], "lev.package", luaopen_lev_package);
    register_to(globals(L)["package"]["preload"], "lev.prim", luaopen_lev_prim);
    register_to(globals(L)["package"]["preload"], "lev.screen", luaopen_lev_screen);
    register_to(globals(L)["package"]["preload"], "lev.sound", luaopen_lev_sound);
    register_to(globals(L)["package"]["preload"], "lev.std", luaopen_lev_std);
    register_to(globals(L)["package"]["preload"], "lev.string", luaopen_lev_string);
    register_to(globals(L)["package"]["preload"], "lev.system", luaopen_lev_system);
    register_to(globals(L)["package"]["preload"], "lev.timer", luaopen_lev_timer);
    register_to(globals(L)["package"]["preload"], "lev.util", luaopen_lev_util);
  }

}

