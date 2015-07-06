package = "LuaCOM"
version = "1.4-1"
source = {
   url = "http://luacom.luaforge.net/luacom-1.4.zip",
}
description = {
   summary = "Use COM libraries from Lua",
   detailed = [[
LuaCOM is an add-on library to the Lua language that allows Lua programs to use and implement objects that follow Microsoft's Component Object Model (COM) specification and use the ActiveX technology for property access and method calls.   ]],
   license = "MIT/X11",
   homepage = "http://luaforge.net/projects/luacom/"
}
dependencies = {
   "lua >= 5.1"
}
build = {
   platforms = {
     win32 = {
        type = "make",
       	build_variables = {
         LUA_LIBDLL = "$(LUA_LIBDIR)",
	 LUA_INC = "$(LUA_INCDIR)",
         LUA = "$(LUA_LIBDIR)\\lua5.1.exe",
         LUAC = "$(LUA_LIBDIR)\\luac5.1.exe",
         BIN2C = "$(LUA_LIBDIR)\\bin2c5.1.exe"
       	},
        build_target = "lc_dll",
       	install_variables = {
         LUA_LIBDIR = "$(LIBDIR)",
       	}
     }
  }
}

