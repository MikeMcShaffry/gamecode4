package = "Rings"
version = "1.2.2-1"

source = {
   url = "http://luaforge.net/frs/download.php/3511/rings-1.2.2.tar.gz",
}

description = {
   summary = "Create new Lua states from within Lua",
   detailed = [[
      Rings is a library which provides a way to create new Lua states
      from within Lua. It also offers a simple way to communicate
      between the creator (master) and the created (slave) states.
   ]],
   license = "MIT/X11",
   homepage = "http://www.keplerproject.org/rings/"
}
dependencies = {
   "lua >= 5.1"
}
build = {
   platforms = {
     unix = {
        type = "make",
       	build_variables = {
         LIB_OPTION = "$(LIBFLAG)",
         CFLAGS = "$(CFLAGS) -I$(LUA_INCDIR)",
       	},
       	install_variables = {
         LUA_LIBDIR = "$(LIBDIR)",
	 LUA_DIR = "$(LUADIR)"
       	}
     },
     win32 = {
        type = "make",
       	build_variables = {
         LUA_LIB = "$(LUA_LIBDIR)\\lua5.1.lib",
         CFLAGS = "/MD $(CFLAGS) /I$(LUA_INCDIR)",
       	},
       	install_variables = {
         LUA_LIBDIR = "$(LIBDIR)",
         LUA_DIR = "$(LUADIR)",
	 BIN_DIR = "$(BINDIR)"
       	}
     }
  }
}
