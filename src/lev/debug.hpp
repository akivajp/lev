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
#include "screen.hpp"

#include <boost/shared_ptr.hpp>
#include <luabind/luabind.hpp>

extern "C" {
  int luaopen_lev_debug(lua_State *L);
}

namespace lev
{

  class debugger : public base
  {
    public:
      typedef boost::shared_ptr<debugger> ptr;
    protected:
      debugger() : base() { }
    public:
      virtual ~debugger() { }
      virtual bool clear() = 0;
//      virtual bool close() = 0;
      static boost::shared_ptr<debugger> get();
      virtual boost::shared_ptr<layout> get_layout() = 0;
      virtual std::string get_log() const = 0;
      virtual screen::ptr get_screen() = 0;
      virtual type_id get_type_id() const { return LEV_TDEBUGGER; }
      virtual bool print(const std::string &message_utf8, int font_size = 18) = 0;
      bool print1(const std::string &message_utf8) { return print(message_utf8); }
      virtual bool show() = 0;
      static boost::shared_ptr<debugger> start();
      static bool stop();
  };

  bool debug_print(const std::string &message_utf8);
  bool debug_print_lua(luabind::object obj);

}

#endif // _DEBUG_HPP

