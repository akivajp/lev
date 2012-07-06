/////////////////////////////////////////////////////////////////////////////
// Name:        src/debug.cpp
// Purpose:     source for debugging features
// Author:      Akiva Miura <akiva.miura@gmail.com>
// Modified by:
// Created:     02/08/2012
// Copyright:   (C) 2010-2012 Akiva Miura
// Licence:     MIT License
/////////////////////////////////////////////////////////////////////////////

// pre-compiled header
#include "prec.h"

// declarations
#include "lev/debug.hpp"

// dependencies
#include "lev/font.hpp"
#include "lev/system.hpp"
#include "lev/string.hpp"
#include "lev/util.hpp"


namespace lev
{

  class impl_debugger : public debugger
  {
    public:
      typedef boost::shared_ptr<impl_debugger> ptr;
    protected:
      impl_debugger() :
        debugger(),
        log(), buffer(),
        scr(), lay()
      { }
    public:
      virtual ~impl_debugger()
      {
      }

      virtual bool clear()
      {
        scr->clear();
        lay->clear();
        scr->swap();
        log = "";
        buffer = "";
        return true;
      }

      virtual layout::ptr get_layout()
      {
        return lay;
      }

      virtual std::string get_log() const
      {
        return log;
      }

      virtual screen::ptr get_screen()
      {
        return scr;
      }

      virtual bool print(const std::string &message_utf8, int font_size)
      {
        if (! scr) { return false; }
        if (! lay || ! lay->get_font()) { return false; }
        if (font_size <= 0) { return false; }
        try {
          scr->set_current();
          boost::shared_ptr<ustring> str = ustring::from_utf8(message_utf8);
          if (! str) { throw -1; }
          lay->get_font()->set_size(font_size);
          for (int i = 0; i < str->length(); i++)
          {
            std::string unit = str->index_str(i)->to_utf8();
            if (unit == "\r") { continue; }
            else if (unit == "\n") { lay->reserve_new_line(); }
            else { lay->reserve_word(unit); }
          }
          lay->complete();
          scr->clear();
          if (lay->get_h() > scr->get_h())
          {
            scr->draw(lay, 0, - (lay->get_h() - scr->get_h()), 255);
          }
          else
          {
            scr->draw(lay, 0, 0, 255);
          }
          scr->swap();
          log += message_utf8;
          buffer += message_utf8;

          if (lay->get_h() > scr->get_h() * 2)
          {
            // messages are to long, purging the oldest part
            str = ustring::from_utf8(buffer);
            str = str->sub_string1(str->length() / 4);
            buffer = str->to_utf8();
            lay->clear();

            for (int i = 0; i < str->length(); i++)
            {
              std::string unit = str->index_str(i)->to_utf8();
              if (unit == "\r") { continue; }
              else if (unit == "\n") { lay->reserve_new_line(); }
              else { lay->reserve_word(unit); }
            }
          }
        }
        catch (...) {
          fprintf(stderr, "error on debug printing\n");
          return false;
        }
        return true;
      }

      virtual bool show()
      {
        if (! scr) { return false; }
        return scr->show();
      }

      static debugger::ptr start()
      {
        system::ptr sys = system::get();
        impl_debugger::ptr dbg;
        if (! sys) { return dbg; }
        if (sys->get_debugger()) { return sys->get_debugger(); }
        try {
          const int w = 640, h = 480;
          dbg.reset(new impl_debugger);
          if (! dbg) { throw -1; }
          dbg->scr = screen::create("Lev Debug Window", 0, 0, w, h);
          if (! dbg->scr) { throw -2; }
          dbg->lay = layout::create(w);
          if (! dbg->lay) { throw -3; }
          if (! sys->attach(dbg)) { throw -4; }
        }
        catch (...) {
          dbg.reset();
          lev::debug_print("error on debugger instance creation");
        }
        return dbg;
      }

      static bool stop()
      {
        system::ptr sys = system::get();
        if (! sys) { return false; }
        return sys->stop_debug();
      }

      std::string log, buffer;
      boost::shared_ptr<screen> scr;
      boost::shared_ptr<layout> lay;
  };

  debugger::ptr debugger::start()
  {
    return impl_debugger::start();
  }


  bool debug_print(const std::string &message_utf8)
  {
    time_t t;
    struct tm *t_st;
    time(&t);
    t_st = localtime(&t);
    char buf[9];
    strftime(buf, 9, "%H:%M:%S", t_st);

    system::ptr sys = system::get();
    if (sys->get_debugger())
    {
      sys->get_debugger()->show();
      return sys->get_debugger()->print1(std::string("[") + buf + "]: " + message_utf8 + "\n");
    }
    printf("Debug Message (%s): %s\n", buf, message_utf8.c_str());
  }

  bool debug_print_lua(luabind::object obj)
  {
    return debug_print(util::tostring(obj));
  }

}

int luaopen_lev_debug(lua_State *L)
{
  using namespace luabind;
  using namespace lev;

  open(L);
  // beginning of loading
  globals(L)["package"]["loaded"]["lev.debug"] = true;
  // pre-requirement
  globals(L)["require"]("lev.system");
  globals(L)["require"]("lev.screen");

  module(L, "lev")
  [
    namespace_("debug")
    [
      def("print", &debug_print_lua)
    ],
    namespace_("classes")
    [
      class_<debugger, base, boost::shared_ptr<base> >("debugger")
        .def("clear", &debugger::clear)
        .def("get_log", &debugger::get_log)
        .property("layout", &debugger::get_layout)
        .property("log", &debugger::get_log)
        .def("print", &debugger::print)
        .def("print", &debugger::print1)
        .property("screen", &debugger::get_screen)
        .scope
        [
//          def("get", &debugger::get),
          def("start", &debugger::start)
        ]
    ]
  ];
  object lev = globals(L)["lev"];
  object classes = lev["classes"];
  object debug = lev["debug"];

  lev["debugger"] = classes["debugger"]["start"];
  globals(L)["package"]["loaded"]["lev.debug"] = true;
  return 0;
}

