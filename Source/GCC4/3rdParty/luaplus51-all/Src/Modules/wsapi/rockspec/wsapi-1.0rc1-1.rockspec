package = "WSAPI"

version = "1.0rc1-1"

description = {
  summary = "Lua Web Server API",
  detailed = [[
    WSAPI is an API that abstracts the web server from Lua web applications. This is the rock
    contains the base WSAPI libraries plus the CGI and Xavante adapters.
  ]],
  license = "MIT/X11",
  homepage = "http://www.keplerproject.org/wsapi"
}

dependencies = { 
  "luafilesystem 1.4.1rc1",
  "rings 1.2.1rc1",
  "coxpcall 1.12.0rc1"
}

source = {
  url = "http://wsapi.luaforge.net/wsapi-1.0rc1.tar.gz",
}

build = {
   type = "make",
   build_pass = false,
   install_target = "install-rocks",
   install_variables = {
     PREFIX  = "$(PREFIX)",
     LUA_BIN = "/usr/bin/env lua",
     LUA_DIR = "$(LUADIR)",
     BIN_DIR = "$(BINDIR)"
   },
   platforms = {
     win32 = {
       build_pass = true,
       build_target = "cgi",
       build_variables = {
         LUA_INCLUDE = "$(LUA_INCDIR)",
	 LUA_LIB = "$(LUA_LIBDIR)\\lua5.1.lib"
       }
     }
   }
}
