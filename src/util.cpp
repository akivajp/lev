/////////////////////////////////////////////////////////////////////////////
// Name:        src/util.hpp
// Purpose:     source for utility functions
// Author:      Akiva Miura <akiva.miura@gmail.com>
// Created:     01/18/2011
// Copyright:   (C) 2010-2011 Akiva Miura
// Licence:     MIT License
/////////////////////////////////////////////////////////////////////////////

// pre-compiled header
#include "prec.h"

// declartions
#include "lev/util.hpp"

// dependencies
//#include "lev/fs.hpp"
#include "lev/string.hpp"
#include "register.hpp"

// libraries
#include <boost/format.hpp>
#include <luabind/luabind.hpp>
#ifdef _WIN32
  #include <windows.h>
#endif // _WIN32

static const char *code_using =
"-- looking up of varnames\n\
local lookup = function(env, varname)\n\
  meta = getmetatable(env)\n\
  if meta.__owner[varname] ~= nil then\n\
    return meta.__owner[varname]\n\
  end\n\
  for i,t in ipairs(meta.__lookup) do\n\
    if t[varname] ~= nil then\n\
      return t[varname]\n\
    end\n\
  end\n\
  if (meta.__parent == meta.__owner) then\n\
    -- non sense to look up twice for the same table\n\
    return nil\n\
  end\n\
  return meta.__parent[varname]\n\
end\n\
\n\
-- subtituting the new value\n\
local substitute = function(env, key, value)\n\
  meta = getmetatable(env)\n\
  meta.__owner[key] = value\n\
end\n\
\n\
-- like using directive\n\
return function(...)\n\
  -- getting environment of the caller and its metatable\n\
  local env  = getfenv(2)\n\
  local meta = getmetatable(env) or {}\n\
  -- getting the caller itself\n\
  local f = _G.setfenv(2, env)\n\
  if (meta.__caller == f) then\n\
    -- setup was already done, changing looking up preference\n\
    if (...) == nil then\n\
      -- 1st arg is nil, resetting looking up setting\n\
      if #{...} >= 2 then\n\
        meta.__lookup = lev.util.reverse({_G.select(2, ...)})\n\
      else\n\
        meta.__lookup = {}\n\
      end\n\
      return env\n\
    end\n\
    for i,val in _G.ipairs({...}) do\n\
      lev.util.remove_first(meta.__lookup, val)\n\
      table.insert(meta.__lookup, 1, val)\n\
    end\n\
    return env\n\
  end\n\
\n\
  -- setting new looking up mechanism\n\
  local newenv  = {}\n\
  local newmeta = {}\n\
  newmeta.__caller = f\n\
  newmeta.__index = lookup\n\
  newmeta.__lookup = lev.util.reverse({...})\n\
  newmeta.__newindex = substitute\n\
  newmeta.__owner = meta.__owner or env\n\
  newmeta.__parent = env\n\
  setmetatable(newenv, newmeta)\n\
  setfenv(2, newenv)\n\
  return newenv\n\
end\n\
";


int luaopen_lev_util(lua_State *L)
{
  using namespace lev;
  using namespace luabind;

  open(L);
  globals(L)["require"]("lev.base");
//  globals(L)["require"]("lev.fs");

  module(L, "lev")
  [
    namespace_("util")
    [
      def("execute", &util::execute),
      def("print_table", &util::print_table),
      def("open", &util::open),
      def("open", &util::open1),
      def("serialize", &util::serialize1)
    ]
  ];
  object lev = globals(L)["lev"];
  object util = lev["util"];

  register_to(util, "execute_code", &util::execute_code_l);
  register_to(util, "merge", &util::merge);
  register_to(util, "remove_first", &util::remove_first);
  register_to(util, "reverse", &util::reverse);
  load_to(util, "using", code_using);

  lev["execute"] = util["execute"];
  lev["execute_code"] = util["execute_code"];
  globals(L)["package"]["loaded"]["lev.util"] = util;
  return 0;
}

namespace lev
{

  luabind::object util::copy_function(luabind::object func)
  {
    using namespace luabind;
    lua_State *L = func.interpreter();
    return globals(L)["loadstring"](globals(L)["string"]["dump"](func));
  }

  bool util::execute(const std::string &target)
  {
//    wxString exe = wxStandardPaths::Get().GetExecutablePath();
//    wxExecute(exe + wxT(" ") + wxString(target.c_str(), wxConvUTF8));
//    return true;
  }

  int util::execute_code_l(lua_State *L)
  {
    using namespace luabind;
    const char *code = "";

//    object t = util::get_merged(L, 1, -1);
//    if (t["code"]) { code = object_cast<const char *>(t["code"]); }
//    else if (t["c"]) { code = object_cast<const char *>(t["c"]); }
//    else if (t["lua.string1"]) { code = object_cast<const char *>(t["lua.string1"]); }
//
//    try {
//      std::string prefix = application::get_app()->get_name() + "/" + "localcode";
//      object tmp = globals(L)["lev"]["classes"]["temp_name"]["create"](prefix, ".lua");
//      std::string name = object_cast<const char *>(tmp["name"]);
//      FILE *out = fopen(name.c_str(), "w");
//      if (out == NULL) { throw -1; }
//      int len = strlen(code);
//      if (fwrite(code, 1, len, out) != len) { throw -2; }
//      fclose(out);
//      util::execute("\"" + name + "\"");
//
//      lua_pushboolean(L, true);
//      return 1;
//    }
//    catch (...) {
//      lua_pushboolean(L, false);
//      return 1;
//    }

    return 0;
  }

  luabind::object util::get_merged(lua_State *L, int begin, int end)
  {
    using namespace luabind;

    if (end < 0) { end = lua_gettop(L) + 1 + end; }
    if (end < begin) { return newtable(L); }
    lua_pushcfunction(L, &util::merge);
    newtable(L).push(L);
    for (int i = begin; i <= end; i++)
    {
      object(from_stack(L, i)).push(L);
    }
    lua_call(L, end + 2 - begin, 1);
    object t(from_stack(L, -1));
    lua_pop(L, 1);
    return t;
  }

  int util::merge(lua_State *L)
  {
    using namespace luabind;
    int n = lua_gettop(L);

    // 1st arg must be table
    luaL_checktype(L, 1, LUA_TTABLE);
    object target(from_stack(L, 1));

    for (int i = 2; i <= n; i++)
    {
      object arg(from_stack(L, i));
      switch (type(arg))
      {
        case LUA_TBOOLEAN:
          target["lua.boolean"] = arg;
          for (int j = 1; ; ++j)
          {
            std::string key = boost::io::str(boost::format("lua.boolean%1%") % j);
            if (!target[key.c_str()])
            {
              target[key.c_str()] = arg;
              break;
            }
          }
          break;
        case LUA_TFUNCTION:
          target["lua.function"] = arg;
          for (int j = 1; ; ++j)
          {
            std::string key = boost::io::str(boost::format("lua.function%1%") % j);
            if (!target[key.c_str()])
            {
              target[key.c_str()] = arg;
              break;
            }
          }
          break;
        case LUA_TNUMBER:
          target["lua.number"] = arg;
          for (int j = 1; ; ++j)
          {
            std::string key = boost::io::str(boost::format("lua.number%1%") % j);
            if (!target[key.c_str()])
            {
              target[key.c_str()] = arg;
              break;
            }
          }
          break;
        case LUA_TSTRING:
          target["lua.string"] = arg;
          for (int j = 1; ; ++j)
          {
            std::string key = boost::io::str(boost::format("lua.string%1%") % j);
            if (!target[key.c_str()])
            {
              target[key.c_str()] = arg;
              break;
            }
          }
          break;
        case LUA_TTABLE:
          for (iterator i(arg), end; i != end; ++i)
          {
            if (type(i.key()) == LUA_TNUMBER)
            {
              object tmp = *i;
              lua_pushcfunction(L, &util::merge);
              target.push(L);
              tmp.push(L);
              lua_call(L, 2, 0);
            }
//            else if (type(i.key()) == LUA_TSTRING)
//            {
//              target[i.key()] = *i;
//              for (int j = 1; ; ++j)
//              {
//                std::string key = (boost::format("%1%%2%") % i.key() % j).str();
//                if (!target[key.c_str()])
//                {
//                  target[key.c_str()] = *i;
//                  break;
//                }
//              }
//            }
            else { target[i.key()] = *i; }
          }
          break;
        case LUA_TUSERDATA:
          base *base_obj;
          base_obj = object_cast<base *>(arg);
          if (base_obj)
          {
            const char *name = base_obj->get_type_name();
            target[name] = arg;
            for (int j = 1; ; ++j)
            {
              std::string key = (boost::format("%1%%2%") % name % j).str();
              if (!target[key.c_str()])
              {
                target[key.c_str()] = arg;
                break;
              }
            }
          }
          target["lua.userdata1"] = arg;
          for (int j = 1; ; ++j)
          {
            std::string key = boost::io::str(boost::format("lua.userdata1%1%") % j);
            if (!target[key.c_str()])
            {
              target[key.c_str()] = arg;
              break;
            }
          }
          break;
        case LUA_TNIL:
        default:
          break;
      }
    }

    target.push(L);
    return 1;
  }

  bool util::open(const std::string &path, const std::string &type)
  {
#ifdef _WIN32
    ShellExecute(0, "open", path.c_str(), 0, 0, SW_SHOWDEFAULT);
    return true;
#else
    if (path.empty()) { return false; }
    int unused = system((std::string("xdg-open ") + path).c_str());
    return true;
#endif // _WIN32
//    wxMimeTypesManager manager;
//    wxFileType *ft = NULL;
//    if (! type.empty())
//    {
//      ft = manager.GetFileTypeFromExtension(wxString(type.c_str(), wxConvUTF8));
//      if (! ft) { ft = manager.GetFileTypeFromMimeType(wxString(type.c_str(), wxConvUTF8)); }
//      if (! ft) { return false; }
//    }
//    else
//    {
//      wxString ext(file_system::get_ext(path).c_str(), wxConvUTF8);
//      if (path.find("http://") == 0 || path.find("https://") == 0)
//      {
//        ft = manager.GetFileTypeFromMimeType(wxT("text/html"));
//      }
//      else
//      {
//        ft = manager.GetFileTypeFromExtension(ext);
//      }
//      if (! ft) { return false; }
//    }
//    wxString command = ft->GetOpenCommand(wxString(path.c_str(), wxConvUTF8));
//    wxExecute(command);
//    return true;
  }

  bool util::print_table(luabind::object t)
  {
    using namespace luabind;

    lua_State *L = t.interpreter();
    if (!t) { return false; }
    if (type(t) == LUA_TTABLE)
    {
      for (iterator i(t), end; i != end; i++)
      {
        globals(L)["print"](i.key(), *i);
      }
    }
    else
    {
      globals(L)["print"](t);
    }
    return true;
  }

  // find "value" from "t" and remove first one
  int util::remove_first(lua_State *L)
  {
    using namespace luabind;

    luaL_checktype(L, 1, LUA_TTABLE);
    luaL_checkany(L, 2);
    object table(from_stack(L, 1));
    object value(from_stack(L, 2));
    object fn_remove = globals(L)["table"]["remove"];

    for (iterator i(table), end; i != end; i++)
    {
      if (*i == value)
      {
        object res = (*i);
        res.push(L);
        fn_remove(table, i.key());
        return 1;
      }
    }
    return 0;
  }

  // return new order-reversed table of "t"
  int util::reverse(lua_State *L)
  {
    using namespace luabind;

    luaL_checktype(L, 1, LUA_TTABLE);
    object table(from_stack(L, 1));
    object rev = newtable(L);
    object fn_insert = globals(L)["table"]["insert"];

    for (iterator i(table), end; i != end; i++)
    {
      fn_insert(rev, 1, *i);
    }
    rev.push(L);
    return 1;
  }

  // looking up of "varnames"
  static int lookup(lua_State *L)
  {
    using namespace luabind;

    luaL_checktype(L, 1, LUA_TTABLE);
    luaL_checkstring(L, 2);
    object env(from_stack(L, 1));
    object varname(from_stack(L, 2));

    object meta = getmetatable(env);
    if (meta["__owner"][varname] != nil)
    {
      meta["__owner"][varname].push(L);
      return 1;
    }
    for (iterator i(meta["__lookup"]), end; i != end; i++)
    {
      if ((*i)[varname] != nil)
      {
        (*i)[varname].push(L);
        return 1;
      }
    }
    // non sense to look up twice for the same table
    if (meta["__parent"] == meta["__owner"]) { return 0; }
    meta["__parent"][varname].push(L);
    return 1;
  }

  std::string util::serialize(luabind::object var, int indent)
  {
    using namespace luabind;

    lua_State *L = var.interpreter();

    if (indent > 20)
    {
      luaL_error(L, "a cyclic linkage was detected!");
      return "";
    }

    if (! var) { return "nil"; }
    if (type(var) == LUA_TNIL) { return "nil"; }
    if (type(var) == LUA_TNUMBER)
    {
      return object_cast<const char *>(globals(L)["tostring"](var));
    }
    else if (type(var) == LUA_TSTRING)
    {
//      unistr str(object_cast<const char *>(var));
//      std::string str(object_cast<const char *>(var));
//      str.Replace(wxT("\\"), wxT("\\\\"), true);
      var = var["gsub"](var, "\\", "\\\\");
//      str.Replace(wxT("\""), wxT("\\\""), true);
      var = var["gsub"](var, "\"", "\\\"");
//      str.Replace(wxT("\'"), wxT("\\\'"), true);
      var = var["gsub"](var, "\'", "\\\'");
//      str.Replace(wxT("\n"), wxT("\\\n"), true);
      var = var["gsub"](var, "\n", "\\n");
      std::string str(object_cast<const char *>(var));
      return "\"" + str + "\"";
    }
    else if (type(var) == LUA_TTABLE)
    {
      std::string str_indent = "";
      for (int i = 0; i < indent; i++)
      {
        str_indent += " ";
      }

      iterator i(var), end;
      std::string str_exp = "{\n";
      for (; i != end; i++)
      {
        if (type(i.key()) != LUA_TNUMBER && type(i.key()) != LUA_TSTRING) { continue; }
        str_exp += str_indent + "  [" + serialize(i.key(), 0) + "] = ";
        str_exp += serialize(*i, indent + 2) + ",\n";
      }
      str_exp += str_indent + "}";
      return str_exp;
    }
    return "nil";
  }

}

