package = "Alien"

version = "0.5.0-1"

description = {
  summary = "Lua->C FFI",
  detailed = [[
    Alien lets a Lua application call load dynamic libraries and call C functions
    in a portable way, using libffi.
  ]],
  license = "MIT/X11",
  homepage = "http://alien.luaforge.net"
}

dependencies = { }

source = {
   url = "http://alien.luaforge.net/alien-0.5.0.tar.gz"
}

build = {
   type = "make",
   install_variables = {
      LUA_LIBDIR = "$(LIBDIR)",
      LUA_DIR = "$(LUADIR)",
      BIN_DIR = "$(BINDIR)",
      PREFIX = "$(PREFIX)"
   },
   platforms = {
     linux = {
       	build_variables = {
         LIB_OPTION = "-shared",
         CFLAGS = '$(CFLAGS) -I$(LUA_INCDIR) -DLINUX -fno-stack-protector',
	 LIB_EXT = '.so'
       	},
     },
     freebsd = {
       	build_variables = {
         LIB_OPTION = "-shared",
         CFLAGS = "$(CFLAGS) -I$(LUA_INCDIR) -DBSD",
	 LIB_EXT = '.so'
       	},
     },
     macosx = {
       	build_variables = {
         LIB_OPTION = "-bundle -undefined dynamic_lookup",
         CFLAGS = "$(CFLAGS) -I$(LUA_INCDIR) -DDARWIN",
	 LIB_EXT = '.dylib'
       	},
     },
     win32 = {
       	build_variables = {
         LIB_OPTION = "$(LUA_LIBDIR)\\lua5.1.lib",
         CFLAGS = "$(CFLAGS) /I$(LUA_INCDIR) /Ilibffi\\win32 /DWINDOWS",
       	}
     }
  }
}
