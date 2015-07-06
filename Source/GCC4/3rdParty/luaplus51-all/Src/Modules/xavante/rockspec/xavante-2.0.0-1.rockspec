package = "Xavante"

version = "2.0.0-1"

source = {
  url = "http://luaforge.net/frs/download.php/3425/xavante-2.0.0.tar.gz",
}

description = {
  summary = "Lua Web Server Library",
  detailed = [[
    Xavante is a Lua HTTP 1.1 Web server that uses a modular architecture based on URI mapped handlers.
    This rock installs Xavante as a library that other applications can use.
  ]],
  license = "MIT/X11",
  homepage = "http://www.keplerproject.org/xavante"
}

dependencies = {
	'copas >= 1.1.3',
	'luasocket >= 2.0.2',
	'luafilesystem >= 1.4.1'
}

build = {
   type = "make",
   build_pass = false,
   install_target = "install",
   install_variables = {
     LUA_DIR = "$(LUADIR)",
   }
}
