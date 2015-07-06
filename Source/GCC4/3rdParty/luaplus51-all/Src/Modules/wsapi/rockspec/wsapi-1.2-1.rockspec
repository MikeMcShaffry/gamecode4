package = "WSAPI"

version = "1.2-1"

description = {
  summary = "Lua Web Server API",
  detailed = [[
    WSAPI is an API that abstracts the web server from Lua web applications. This is the rock
    that contains the base WSAPI libraries plus the CGI adapters.
  ]],
  license = "MIT/X11",
  homepage = "http://www.keplerproject.org/wsapi"
}

dependencies = { "luafilesystem >= 1.4.2" }

source = {
  url = "http://cloud.github.com/downloads/keplerproject/wsapi/wsapi-1.2.tar.gz"
}

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
	  ["wsapi.sapi"] = "src/wsapi/sapi.lua",
	  ["wsapi.ringer"] = "src/wsapi/ringer.lua",
	},
	copy_directories = { "samples", "doc", "tests" },
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
