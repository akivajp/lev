#ifndef _BASE_HPP
#define _BASE_HPP

/////////////////////////////////////////////////////////////////////////////
// Name:        lev/base.hpp
// Purpose:     header for base class of the all
// Author:      Akiva Miura <akiva.miura@gmail.com>
// Modified by:
// Created:     12/12/2010
// Copyright:   (C) 2010-2012 Akiva Miura
// Licence:     MIT License
/////////////////////////////////////////////////////////////////////////////

#include <boost/shared_ptr.hpp>
#include <luabind/luabind.hpp>
#include <string>

extern "C" {
  int luaopen_lev_base(lua_State *L);
}

namespace lev
{

  class base
  {
    public:
      typedef boost::shared_ptr<base> ptr;

      enum type_id
      {
        LEV_TNONE = -1,
        LEV_TBASE = 1,

          LEV_TARCHIVE,
          LEV_TCOLOR,
          LEV_TDEBUGGER,

          LEV_TDRAWABLE,
            LEV_TANIMATION,
            LEV_TCANVAS,
              LEV_TBITMAP,
              LEV_TSCREEN,
            LEV_TCANVAS_END,
            LEV_TCLICKABLE,
              LEV_TLAYOUT,
              LEV_TMAP,
            LEV_TCLICKABLE_END,
            LEV_TSPACER,
            LEV_TTEXTURE,
            LEV_TTRANSITION,
          LEV_TDRAWABLE_END,

          LEV_TEVENT,

          LEV_TFILE,
            LEV_TMEMFILE,
          LEV_TFILE_END,

          LEV_TFILEPATH,
          LEV_TFONT,
          LEV_TMIXER,
          LEV_TPOINT,
          LEV_TRECT,
          LEV_TSIZE,
          LEV_TSOUND,
          LEV_TSREGEX,
          LEV_TSREGEX_COMPILER,
          LEV_TSTOP_WATCH,
          LEV_TSYSTEM,
          LEV_TTEMP_NAME,

          LEV_TTIMER,
            LEV_TCLOCK,
          LEV_TTIMER_END,

          LEV_TUSTRING,
          LEV_TVECTOR,
        LEV_TBASE_END,
      };
    protected:
      base() { }
      virtual ~base() { }
    public:
      virtual type_id get_type_id() const { return LEV_TBASE; }
      virtual const char *get_type_name() const
      {
        return get_type_name_by_id(get_type_id());
      }

      static type_id get_base_id(type_id id);
      static const char *get_type_name_by_id(type_id id);
      static bool is_type_of(const luabind::object &obj, type_id id,
                             type_id id_stop = LEV_TNONE);
      static std::string tostring(const base *b);
  };

}

#endif // _BASE_HPP

