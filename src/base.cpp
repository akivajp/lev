/////////////////////////////////////////////////////////////////////////////
// Name:        src/base.cpp
// Purpose:     source for base class of the all
// Author:      Akiva Miura <akiva.miura@gmail.com>
// Modified by:
// Created:     02/24/2012
// Copyright:   (C) 2010-2012 Akiva Miura
// Licence:     MIT License
/////////////////////////////////////////////////////////////////////////////

// pre-compiled header
#include "prec.h"
// declarations
#include "lev/base.hpp"
// dependencies
#include "lev/debug.hpp"
// libraries
#include <boost/format.hpp>
#include <map>

int luaopen_lev_base(lua_State *L)
{
  using namespace luabind;
  using namespace lev;

  open(L);
  globals(L)["require"]("io");
  globals(L)["require"]("math");
  globals(L)["require"]("os");
  globals(L)["require"]("string");
  globals(L)["require"]("table");

  // base classes
  module(L, "lev")
  [
    namespace_("classes")
    [
      // base of all
      class_<base, boost::shared_ptr<base> >("base")
        .def("__tostring", &base::tostring)
        .property("type_id", &base::get_type_id)
        .property("type_name", &base::get_type_name)
    ]
  ];

  globals(L)["package"]["loaded"]["lev.base"] = true;
  return 0;
}

namespace lev
{

  base::type_id base::get_base_id(type_id id)
  {
    static std::map<type_id, type_id>* base_id_map = NULL;

    if (! base_id_map)
    {
      try {
        base_id_map = new std::map<type_id, type_id>;
        (*base_id_map)[LEV_TNONE]       = LEV_TNONE;

        (*base_id_map)[LEV_TBASE]       = LEV_TNONE;
        {
          (*base_id_map)[LEV_TARCHIVE]  = LEV_TBASE;
          (*base_id_map)[LEV_TCOLOR]    = LEV_TBASE;

          (*base_id_map)[LEV_TDRAWABLE] = LEV_TBASE;
          {
            (*base_id_map)[LEV_TANIMATION]  = LEV_TDRAWABLE;
            (*base_id_map)[LEV_TIMAGE]      = LEV_TDRAWABLE;
            (*base_id_map)[LEV_TLAYOUT]     = LEV_TDRAWABLE;
            (*base_id_map)[LEV_TMAP]        = LEV_TDRAWABLE;
            (*base_id_map)[LEV_TTEXTURE]    = LEV_TDRAWABLE;
            (*base_id_map)[LEV_TTRANSITION] = LEV_TDRAWABLE;
          }

          (*base_id_map)[LEV_TEVENT]           = LEV_TBASE;
          (*base_id_map)[LEV_TFILE_PATH]       = LEV_TBASE;
          (*base_id_map)[LEV_TFONT]            = LEV_TBASE;
          (*base_id_map)[LEV_TMIXER]           = LEV_TBASE;
          (*base_id_map)[LEV_TPOINT]           = LEV_TBASE;
          (*base_id_map)[LEV_TRASTER]          = LEV_TBASE;
          (*base_id_map)[LEV_TRECT]            = LEV_TBASE;
          (*base_id_map)[LEV_TSCREEN]          = LEV_TBASE;
          (*base_id_map)[LEV_TSIZE]            = LEV_TBASE;
          (*base_id_map)[LEV_TSOUND]           = LEV_TBASE;
          (*base_id_map)[LEV_TSREGEX]          = LEV_TBASE;
          (*base_id_map)[LEV_TSREGEX_COMPILER] = LEV_TBASE;
          (*base_id_map)[LEV_TSTOP_WATCH]      = LEV_TBASE;
          (*base_id_map)[LEV_TSYSTEM]          = LEV_TBASE;
          (*base_id_map)[LEV_TTEMP_NAME]       = LEV_TBASE;

          (*base_id_map)[LEV_TTIMER]           = LEV_TBASE;
          {
            (*base_id_map)[LEV_TCLOCK] = LEV_TTIMER;
          }

          (*base_id_map)[LEV_TUNICODE]         = LEV_TBASE;
          (*base_id_map)[LEV_TVECTOR]          = LEV_TBASE;

          (*base_id_map)[LEV_TWINDOW]          = LEV_TBASE;
          {
            (*base_id_map)[LEV_TDEBUG_WINDOW]  = LEV_TWINDOW;
          }
        }
      }
      catch (...) {
        delete base_id_map;
        base_id_map = NULL;
        lev::debug_print("error on class id map setup\n");
        return LEV_TNONE;
      }
    }

    std::map<type_id, type_id>::iterator found = base_id_map->find(id);
    if (found == base_id_map->end()) { return LEV_TNONE; }
    else { return found->second; }
  }

  const char *base::get_type_name_by_id(type_id id)
  {
    static std::map<type_id, const char *>* type_name_map = NULL;

    if (! type_name_map)
    {
      try {
        type_name_map = new std::map<type_id, const char *>;
        (*type_name_map)[LEV_TNONE]       = "(none)";
        (*type_name_map)[LEV_TBASE]       = "lev.base";
        (*type_name_map)[LEV_TARCHIVE]    = "lev.archive";
        (*type_name_map)[LEV_TCOLOR]      = "lev.prim.color";
        (*type_name_map)[LEV_TDRAWABLE]   = "lev.drawable";
        (*type_name_map)[LEV_TANIMATION]  = "lev.image.animation";
        (*type_name_map)[LEV_TIMAGE]      = "lev.image";
        (*type_name_map)[LEV_TLAYOUT]     = "lev.image.layout";
        (*type_name_map)[LEV_TMAP]        = "lev.image.map";
        (*type_name_map)[LEV_TTEXTURE]    = "lev.image.texture";
        (*type_name_map)[LEV_TTRANSITION] = "lev.image.transition";
        (*type_name_map)[LEV_TEVENT]      = "lev.event";
        (*type_name_map)[LEV_TFILE_PATH]  = "lev.fs.file_path";
        (*type_name_map)[LEV_TFONT]       = "lev.font";
        (*type_name_map)[LEV_TMIXER]      = "lev.sound.mixer";
        (*type_name_map)[LEV_TPOINT]      = "lev.prim.point";
        (*type_name_map)[LEV_TRASTER]     = "lev.raster";
        (*type_name_map)[LEV_TRECT]       = "lev.prim.rect";
        (*type_name_map)[LEV_TSCREEN]     = "lev.screen";
        (*type_name_map)[LEV_TSIZE]       = "lev.prim.size";
        (*type_name_map)[LEV_TSOUND]      = "lev.sound";
        (*type_name_map)[LEV_TSREGEX]     = "lev.string.sregex";
        (*type_name_map)[LEV_TSREGEX_COMPILER] = "lev.string.sregex_compiler";
        (*type_name_map)[LEV_TSTOP_WATCH] = "lev.stop_watch";
        (*type_name_map)[LEV_TSYSTEM]     = "lev.system";
        (*type_name_map)[LEV_TTEMP_NAME]  = "lev.fs.temp_name";
        (*type_name_map)[LEV_TTIMER]      = "lev.timer";
        (*type_name_map)[LEV_TCLOCK]      = "lev.clock";
        (*type_name_map)[LEV_TUNICODE]    = "lev.string.unicode";
        (*type_name_map)[LEV_TVECTOR]     = "lev.prim.vector";
        (*type_name_map)[LEV_TWINDOW]     = "lev.window";
        (*type_name_map)[LEV_TDEBUG_WINDOW] = "lev.debug_window";
      }
      catch (...) {
        delete type_name_map;
        type_name_map = NULL;
        lev::debug_print("error on class type name map setup\n");
        return "(error)";
      }
    }

    std::map<type_id, const char *>::iterator found = type_name_map->find(id);
    if (found == type_name_map->end()) { return "(not found)"; }
    else { return found->second; }
  }

  bool base::is_type_of(const luabind::object &obj, type_id id,
                        type_id id_stop)
  {
    if (id_stop == LEV_TNONE) { id_stop = id; }
    if (! obj.is_valid()) { return false; }
    if (luabind::type(obj) != LUA_TUSERDATA) { return false; }

    luabind::object obj_id = obj["type_id"];
    if (! obj_id.is_valid()) { return false; }
    if (luabind::type(obj_id) != LUA_TNUMBER) { return false; }

    int id_of_obj = luabind::object_cast<int>(obj_id);
    if (id <= id_of_obj && id_of_obj <= id_stop) { return true; }
    return false;
  }

  std::string base::tostring(const base *b)
  {
    if (! b) { return ""; }
    return (boost::format("%1%: %2%") % b->get_type_name() % b).str();
  }

}

