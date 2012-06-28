#ifndef _INPUT_HPP
#define _INPUT_HPP

/////////////////////////////////////////////////////////////////////////////
// Name:        lev/input.hpp
// Purpose:     header for input management class
// Author:      Akiva Miura <akiva.miura@gmail.com>
// Modified by:
// Created:     06/20/2012
// Copyright:   (C) 2010-2012 Akiva Miura
// Licence:     MIT License
/////////////////////////////////////////////////////////////////////////////

#include "base.hpp"
#include <boost/shared_ptr.hpp>

extern "C" {
  int luaopen_lev_input(lua_State *L);
}


namespace lev
{

  class mouse : public base
  {
    private:
      mouse();
    public:
      ~mouse();

      bool button_is_down(int button);
      static boost::shared_ptr<mouse> get();
      int get_axis(int axis);
      int get_buttons();
      int get_num_axes();
      int get_num_buttons();
      virtual type_id get_type_id() const { return LEV_TMOUSE; }
      bool left_is_down();
      bool middle_is_down();
      bool right_is_down();
      int get_w();
      int get_x();
      int get_y();
      int get_z();
    protected:
      void *_obj;
  };

  class input : public base
  {
    private:
      input() : base() { }
    public:
      ~input() { }
      static boost::shared_ptr<input> get();
      boost::shared_ptr<mouse> get_mouse_state() { return mouse::get(); }
      virtual type_id get_type_id() const { return LEV_TINPUT; }
      static const char *to_keyname(long code);
  };

}


#endif // _INPUT_HPP

