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
          (*base_id_map)[LEV_TDEBUGGER]  = LEV_TBASE;

          (*base_id_map)[LEV_TDRAWABLE] = LEV_TBASE;
          {
            (*base_id_map)[LEV_TANIMATION]  = LEV_TDRAWABLE;
            (*base_id_map)[LEV_TCANVAS]     = LEV_TDRAWABLE;
            {
              (*base_id_map)[LEV_TBITMAP]      = LEV_TCANVAS;
              (*base_id_map)[LEV_TSCREEN]      = LEV_TCANVAS;
            }
            (*base_id_map)[LEV_TLAYOUT]     = LEV_TDRAWABLE;
            (*base_id_map)[LEV_TMAP]        = LEV_TDRAWABLE;
            (*base_id_map)[LEV_TSPACER]     = LEV_TDRAWABLE;
            (*base_id_map)[LEV_TTEXTURE]    = LEV_TDRAWABLE;
            (*base_id_map)[LEV_TTRANSITION] = LEV_TDRAWABLE;
          }

          (*base_id_map)[LEV_TEVENT]           = LEV_TBASE;
          (*base_id_map)[LEV_TFILE]            = LEV_TBASE;
          {
            (*base_id_map)[LEV_TMEMFILE]       = LEV_TFILE;
          }
          (*base_id_map)[LEV_TFILEPATH]        = LEV_TBASE;
          (*base_id_map)[LEV_TFONT]            = LEV_TBASE;
          (*base_id_map)[LEV_TMIXER]           = LEV_TBASE;
          (*base_id_map)[LEV_TPOINT]           = LEV_TBASE;
          (*base_id_map)[LEV_TRECT]            = LEV_TBASE;
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

          (*base_id_map)[LEV_TUSTRING]         = LEV_TBASE;
          (*base_id_map)[LEV_TVECTOR]          = LEV_TBASE;

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
        (*type_name_map)[LEV_TANIMATION]  = "lev.animation";
        (*type_name_map)[LEV_TARCHIVE]    = "lev.archive";
        (*type_name_map)[LEV_TBASE]       = "lev.base";
        (*type_name_map)[LEV_TBITMAP]     = "lev.bitmap";
        (*type_name_map)[LEV_TCANVAS]     = "lev.canvas";
        (*type_name_map)[LEV_TCOLOR]      = "lev.color";
        (*type_name_map)[LEV_TDEBUGGER]   = "lev.debugger";
        (*type_name_map)[LEV_TDRAWABLE]   = "lev.drawable";
        (*type_name_map)[LEV_TEVENT]      = "lev.event";
        (*type_name_map)[LEV_TFILE]       = "lev.file";
        (*type_name_map)[LEV_TFILEPATH]   = "lev.filepath";
        (*type_name_map)[LEV_TFONT]       = "lev.font";
        (*type_name_map)[LEV_TLAYOUT]     = "lev.layout";
        (*type_name_map)[LEV_TMAP]        = "lev.map";
        (*type_name_map)[LEV_TMEMFILE]    = "lev.memfile";
        (*type_name_map)[LEV_TMIXER]      = "lev.mixer";
        (*type_name_map)[LEV_TPOINT]      = "lev.point";
        (*type_name_map)[LEV_TRECT]       = "lev.rect";
        (*type_name_map)[LEV_TSCREEN]     = "lev.screen";
        (*type_name_map)[LEV_TSPACER]     = "lev.spacer";
        (*type_name_map)[LEV_TTEXTURE]    = "lev.texture";
        (*type_name_map)[LEV_TTRANSITION] = "lev.transition";
        (*type_name_map)[LEV_TSIZE]       = "lev.size";
        (*type_name_map)[LEV_TSOUND]      = "lev.sound";
        (*type_name_map)[LEV_TSREGEX]     = "lev.sregex";
        (*type_name_map)[LEV_TSREGEX_COMPILER] = "lev.sregex_compiler";
        (*type_name_map)[LEV_TSTOP_WATCH] = "lev.stop_watch";
        (*type_name_map)[LEV_TSYSTEM]     = "lev.system";
        (*type_name_map)[LEV_TTEMP_NAME]  = "lev.temp_name";
        (*type_name_map)[LEV_TTIMER]      = "lev.timer";
        (*type_name_map)[LEV_TCLOCK]      = "lev.clock";
        (*type_name_map)[LEV_TUSTRING]    = "lev.ustring";
        (*type_name_map)[LEV_TVECTOR]     = "lev.vector";
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

