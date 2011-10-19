#ifndef _BASE_HPP
#define _BASE_HPP

/////////////////////////////////////////////////////////////////////////////
// Name:        lev/base.hpp
// Purpose:     header for base class of the all
// Author:      Akiva Miura <akiva.miura@gmail.com>
// Modified by:
// Created:     12/12/2010
// Copyright:   (C) 2011 Akiva Miura
// Licence:     MIT License
/////////////////////////////////////////////////////////////////////////////

#include <boost/format.hpp>
#include <luabind/luabind.hpp>
#include <string>

namespace lev
{

  class base
  {
    public:
      enum type_id
      {
        LEV_TNONE = -1,
        LEV_TBASE = 1,

          LEV_TCOLOR,

          LEV_TDRAWABLE,
            LEV_TIMAGE,
              LEV_TSCREEN,
            LEV_TIMAGE_END,
            LEV_TLAYOUT,
          LEV_TDRAWABLE_END,

          LEV_TFONT,
          LEV_TMIXER,
          LEV_TPOINT,
          LEV_TRASTER,
          LEV_TRECT,
          LEV_TSIZE,
          LEV_TSOUND,
          LEV_TSYSTEM,
          LEV_TUNISTR,
          LEV_TVECTOR,
        LEV_TBASE_END,
      };
    protected:
      base() { }
      virtual ~base() { }
    public:
      virtual type_id get_type_id() const { return LEV_TBASE; }
      virtual const char *get_type_name() const { return "lev.base"; }

      static std::string tostring(const base *b)
      {
        return (boost::format("%1%: %2%") % b->get_type_name() % b).str();
      }

      static bool is_type_of(const luabind::object &obj, type_id id,
                             type_id id_stop = LEV_TNONE)
      {
        if (id_stop == LEV_TNONE) { id_stop = id; }
        if (! obj) { return false; }
        if (luabind::type(obj) != LUA_TUSERDATA) { return false; }

        luabind::object obj_id = obj["type_id"];
        if (! obj_id) { return false; }
        if (luabind::type(obj_id) != LUA_TNUMBER) { return false; }

        int id_of_obj = luabind::object_cast<int>(obj_id);
        if (id <= id_of_obj && id_of_obj <= id_stop) { return true; }
        return false;
      }

  };

}

#endif // _BASE_HPP

