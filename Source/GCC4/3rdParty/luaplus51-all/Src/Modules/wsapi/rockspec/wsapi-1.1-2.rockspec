package = "WSAPI"

version = "1.1-2"

source = {
  url = "http://luaforge.net/frs/download.php/3935/wsapi-1.1.0.tar.gz",
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

dependencies = { "luafilesystem >= 1.4.2" }

build = {
   platforms = {
     unix = {
        type = "module",
	modules = {
	  ["wsapi.common"] = "src/wsapi/common.lua",
	  ["wsapi.request"] = "src/wsapi/request.lua",
	  ["wsapi.response"] = "src/wsapi/response.lua",
	  ["wsapi.util"] = "src/wsapi/util.lua",
	  ["wsapi.cgi"] = "src/wsapi/cgi.lua",
	  ["wsapi.fastcgi"] = "src/wsapi/fastcgi.lua",
	  ["wsapi.ringer"] = "src/wsapi/ringer.lua",
	  ["wsapi.sapi"] = "src/wsapi/sapi.lua",
	  ["wsapi.xavante"] = "src/wsapi/xavante.lua"
	},
	copy_directories = { "samples", "doc" },
	install = { bin = { "src/launcher/wsapi.cgi" } }
     },
     win32 = {
        type = "make",
       	build_pass = true,
        build_target = "cgi",
        build_variables = {
         LUA_INCLUDE = "$(LUA_INCDIR)",
	 LUA_LIB = "$(LUA_LIBDIR)\\lua5.1.lib"
        },
   	install_target = "install-rocks",
   	install_variables = {
     	  PREFIX  = "$(PREFIX)",
     	  LUA_BIN = "/usr/bin/env lua",
     	  LUA_DIR = "$(LUADIR)",
     	  BIN_DIR = "$(BINDIR)"
   	},
     }
   }
}
