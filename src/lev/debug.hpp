#ifndef _DEBUG_HPP
#define _DEBUG_HPP

/////////////////////////////////////////////////////////////////////////////
// Name:        src/lev/debug.hpp
// Purpose:     header for debugging features
// Author:      Akiva Miura <akiva.miura@gmail.com>
// Modified by:
// Created:     02/08/2012
// Copyright:   (C) 2010-2012 Akiva Miura
// Licence:     MIT License
/////////////////////////////////////////////////////////////////////////////

#include "base.hpp"
#include "draw.hpp"
#include "window.hpp"

#include <boost/shared_ptr.hpp>
#include <luabind/luabind.hpp>

extern "C" {
  int luaopen_lev_debug(lua_State *L);
}

namespace lev
{

  class debug_window : public window
  {
    protected:
      debug_window();
    public:
      virtual ~debug_window();
      static boost::shared_ptr<debug_window> get() { return singleton; }
      virtual type_id get_type_id() const { return LEV_TDEBUG_WINDOW; }
      virtual const char *get_type_name() const { return "lev.debug_window"; }
      static boost::shared_ptr<debug_window> init();
    protected:
      boost::shared_ptr<screen> ptr_screen;
      static boost::shared_ptr<debug_window> singleton;
  };
}

#endif // _DEBUG_HPP

