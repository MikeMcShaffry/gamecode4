package = "WSAPI-FCGI"

version = "1.2-1"

description = {
  summary = "Lua Web Server API FastCGI Adapter",
  detailed = [[
    WSAPI is an API that abstracts the web server from Lua web applications. This
    is the rock that contains the FCGI module lfcgi.
  ]],
  license = "MIT/X11",
  homepage = "http://www.keplerproject.org/wsapi"
}

dependencies = { "wsapi >= 1.2", "rings >= 1.2.3", "coxpcall >= 1.13" }

external_dependencies = {
  platforms = {
    unix = {
      FASTCGI = {
        header = "fcgi_stdio.h"
      }
    }
  }
}

source = {
  url = "http://github.com/downloads/keplerproject/wsapi/wsapi-1.2.tar.gz"
}

build = {
   platforms = {
     unix = {
        type = "module",
	modules = {
	  ["wsapi.fastcgi"] = "src/wsapi/fastcgi.lua",
	  lfcgi = {
            sources = "src/fastcgi/lfcgi.c",
            libraries = "fcgi",
	    incdirs = "$(FASTCGI_INCDIR)",
	    libdirs = "$(FASTCGI_LIBDIR)"
          }
        },
       install = { bin = { "src/launcher/wsapi.fcgi" } }
     },
     win32 = {
        type = "make",
   	install_target = "install-fcgi",
       	build_pass = true,
       	build_target = "fcgi",
       	build_variables = {
       	 LUA_INCLUDE = "$(LUA_INCDIR)",
	 	 LUA_LIB = "$(LUA_LIBDIR)\\lua5.1.lib",
         LIB_OPTION = "$(LUA_LIBDIR)\\lua5.1.lib $(FASTCGI_DIR)\\libfcgi\\Release\\fcgi_stdio.obj $(FASTCGI_DIR)\\libfcgi\\Release\\os_win32.obj $(FASTCGI_DIR)\\libfcgi\\Release\\fcgiapp.obj",
         CFLAGS = "$(CFLAGS) /I$(FASTCGI_DIR)\\include",
       	},
       	install_variables = {
         LUA_LIBDIR = "$(LIBDIR)",
	 BIN_DIR = "$(BINDIR)"
       	}
     }
  }
}
