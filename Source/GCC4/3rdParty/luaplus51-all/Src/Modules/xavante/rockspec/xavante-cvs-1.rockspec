package = "Xavante"

version = "cvs-1"

description = {
  summary = "Lua Web Server Library",
  detailed = [[
    Xavante is a Lua HTTP 1.1 Web server that uses a modular architecture based on URI mapped handlers.
    This rock installs Xavante as a library that other applications can use.
  ]],
  license = "MIT/X11",
  homepage = "http://www.keplerproject.org/xavante"
}

dependencies = { 'luasocket', 'copas cvs', 'luafilesystem cvs' }

source = {
  url = "cvs://:pserver:anonymous@cvs.luaforge.net:/cvsroot/xavante",
  cvs_tag = "HEAD"
}

build = {
   type = "make",
   build_pass = false,
   install_target = "install",
   install_variables = {
     LUA_DIR = "$(LUADIR)",
   }
}
