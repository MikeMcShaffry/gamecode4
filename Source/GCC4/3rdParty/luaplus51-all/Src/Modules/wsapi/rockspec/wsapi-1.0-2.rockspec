package = "WSAPI"

version = "1.0-2"

source = {
  url = "http://luaforge.net/frs/download.php/3382/wsapi-1.0.0.tar.gz",
}

description = {
  summary = "Lua Web Server API",
  detailed = [[
    WSAPI is an API that abstracts the web server from Lua web applications. This is the rock
    contains the base WSAPI libraries plus the CGI and Xavante adapters.
  ]],
  license = "MIT/X11",
  homepage = "http://wsapi.luaforge.net"
}

dependencies = { }

build = {
   type = "make",
   build_pass = false,
   install_target = "install-rocks",
   install_variables = {
     PREFIX  = "$(PREFIX)",
     LUA_BIN = "$(LUA)",
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
