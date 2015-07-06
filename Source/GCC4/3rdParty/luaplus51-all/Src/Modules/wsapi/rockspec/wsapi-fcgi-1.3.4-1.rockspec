package = "WSAPI-FCGI"

version = "1.3.4-1"

description = {
  summary = "Lua Web Server API FastCGI Adapter",
  detailed = [[
    WSAPI is an API that abstracts the web server from Lua web applications. This
    is the rock that contains the FCGI module lfcgi.
  ]],
  license = "MIT/X11",
  homepage = "http://www.keplerproject.org/wsapi"
}

dependencies = { "wsapi >= 1.3.4", "rings >= 1.2.3", "coxpcall >= 1.13" }

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
  url = "http://github.com/downloads/keplerproject/wsapi/wsapi-1.3.4.tar.gz"
}

build = {
 platforms = {
   unix = {
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
   },
   windows = {
    type = "builtin",
	modules = {
	  ["wsapi.fastcgi"] = "src/wsapi/fastcgi.lua",
	  lfcgi = {
            sources = "src/fastcgi/lfcgi.c",
            libraries = { "libfcgi", "ws2_32" },
	    incdirs = "$(FASTCGI_INCDIR)",
	    libdirs = "$(FASTCGI_LIBDIR)"
          }
        },
       install = { bin = { "src/launcher/wsapi.fcgi" } }
   }
 }
}
