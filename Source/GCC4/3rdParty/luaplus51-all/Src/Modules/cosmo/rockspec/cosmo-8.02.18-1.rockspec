package = "Cosmo"

version = "8.02.18-1"

description = {
  summary = "Safe templates for Lua",
  detailed = [[
Cosmo is a "safe templates" engine. It allows you to fill nested templates, 
providing many of the advantages of Turing-complete template engines, 
without without the downside of allowing arbitrary code in the templates.
  ]],
  license = "MIT/X11",
  homepage = "http://cosmo.luaforge.net"
}

dependencies = { }

source = {
  url = "http://cosmo.luaforge.net/cosmo-8.02.18.tar.gz"
}

build = {
   platforms = {
     unix = {
        type = "make",
	build_pass = true,
	build_target = "all",
   	install_target = "install-rocks",
       	build_variables = {
          LIB_OPTION = "$(LIBFLAG)",
          CFLAGS = "$(CFLAGS) -I$(LUA_INCDIR)",
       	},
       	install_variables = {
	  PREFIX  = "$(PREFIX)",
          LUA_BIN = "/usr/bin/env lua",
          LUA_DIR= "$(LUADIR)",
          LUA_LIBDIR = "$(LIBDIR)"
       	}
     },
     win32 = {
        type = "make",
	build_pass = true,
	build_target = "all",
   	install_target = "install-rocks",
       	build_variables = {
          LIB_OPTION = "$(LUA_LIBDIR)\\lua5.1.lib",
          CFLAGS = "$(CFLAGS)",
	  LUA_INCLUDE = "$(LUA_INCDIR)"
       	},
       	install_variables = {
	  PREFIX  = "$(PREFIX)",
          LUA_BIN = "/usr/bin/env lua",
          LUA_DIR= "$(LUADIR)",
          LUA_LIBDIR = "$(LIBDIR)"
       	}
     }
  }
}
