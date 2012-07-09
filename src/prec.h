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
#include <SDL2/SDL.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <ft2build.h>
#include FT_FREETYPE_H
}

#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/scoped_array.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/xpressive/xpressive.hpp>

#include <luabind/raw_policy.hpp>
#include <luabind/luabind.hpp>

#include <GL/glu.h>


#endif // _PREC_H

