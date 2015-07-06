package = "WSAPI-FCGI"

version = "1.0rc1-1"

source = {
   url = "http://wsapi.luaforge.net/wsapi-1.0rc1.tar.gz",
}

description = {
  summary = "Lua Web Server API FastCGI Adapter",
  detailed = [[
    WSAPI is an API that abstracts the web server from Lua web applications. This
    is the rock that contains the FCGI module lfcgi.
  ]],
  license = "MIT/X11",
  homepage = "http://www.keplerproject.org/wsapi"
}

dependencies = { "wsapi 1.0rc1" }

external_dependencies = {
  FASTCGI = {
    header = "fcgi_stdio.h"
  }
}

build = {
   platforms = {
     unix = {
        type = "make",
   	install_target = "install-fcgi",
       	build_pass = true,
       	build_target = "fcgi",
       	build_variables = {
         LIB_OPTION = "$(LIBFLAG) -L$(FASTCGI_LIBDIR)",
         CFLAGS = "$(CFLAGS) -I$(LUA_INCDIR) -I$(FASTCGI_INCDIR)",
       	},
       	install_variables = {
         LUA_LIBDIR = "$(LIBDIR)"
       	}
     },
     win32 = {
        type = "make",
   	install_target = "install-fcgi",
       	build_pass = true,
       	build_target = "fcgi",
       	build_variables = {
         LIB_OPTION = "$(LUA_LIBDIR)\\lua5.1.lib $(FASTCGI_LIB)",
         CFLAGS = "$(CFLAGS) /I$(FASTCGI_INCDIR)",
       	},
       	install_variables = {
         LUA_LIBDIR = "$(LIBDIR)",
	 BIN_DIR = "$(BINDIR)"
       	}
     }
  }
}
