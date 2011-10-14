#ifndef _PREC_H
#define _PREC_H

/////////////////////////////////////////////////////////////////////////////
// Name:        src/prec.h
// Purpose:     using for generate pre compiled header
// Author:      Akiva Miura <akiva.miura@gmail.com>
// Modified by:
// Created:     13/10/2011
// Copyright:   (C) 2010-2011 Akiva Miura
// Licence:     MIT License
/////////////////////////////////////////////////////////////////////////////

extern "C" {
#include <SDL/SDL.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <boost/format.hpp>
#include <luabind/adopt_policy.hpp>
#include <luabind/luabind.hpp>

#include <GL/glu.h>


#endif // _PREC_H

