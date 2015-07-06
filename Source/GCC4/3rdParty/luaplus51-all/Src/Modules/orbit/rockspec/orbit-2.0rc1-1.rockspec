package = "Orbit"

version = "2.0rc1-1"

description = {
  summary = "MVC for Lua Web Development",
  detailed = [[
     Orbit is a library for developing web applications according to
     the Model-View-Controller paradigm in Lua.
  ]],
  license = "MIT/X11",
  homepage = "http://www.keplerproject.org/orbit"
}

dependencies = { 'wsapi 1.0rc1', 'luafilesystem 1.4.1rc1', 'cosmo 8.04.14' }

source = {
  url = "http://orbit.luaforge.net/orbit-2.0rc1.tar.gz",
}

build = {
   type = "make",
   build_pass = true,
   install_target = "install-rocks",
   install_variables = {
     PREFIX  = "$(PREFIX)",
     LUA_BIN = "/usr/bin/env lua",
     LUA_DIR = "$(LUADIR)",
     BIN_DIR = "$(BINDIR)"
   }
}
