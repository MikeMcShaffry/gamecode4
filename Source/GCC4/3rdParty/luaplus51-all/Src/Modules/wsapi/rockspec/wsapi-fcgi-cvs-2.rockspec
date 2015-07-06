package = "WSAPI-FCGI"

version = "cvs-2"

description = {
  summary = "Lua Web Server API FastCGI Adapter",
  detailed = [[
    WSAPI is an API that abstracts the web server from Lua web applications. This
    is the rock that contains the FCGI module lfcgi.
  ]],
  license = "MIT/X11",
  homepage = "http://www.keplerproject.org/wsapi"
}

dependencies = { "wsapi cvs", "rings >= 1.2.3", "coxpcall >= 1.13" }

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
   url = "git://github.com/keplerproject/wsapi.git",
}

build = {
        type = "builtin",
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
}
