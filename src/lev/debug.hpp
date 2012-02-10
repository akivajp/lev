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
#include "image.hpp"
#include "window.hpp"

#include <boost/shared_ptr.hpp>
#include <luabind/luabind.hpp>

extern "C" {
  int luaopen_lev_debug(lua_State *L);
}

namespace lev
{

  bool debug_print(const std::string &message_utf8);

}

#endif // _DEBUG_HPP

