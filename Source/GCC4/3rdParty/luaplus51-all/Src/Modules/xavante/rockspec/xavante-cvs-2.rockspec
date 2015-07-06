package = "Xavante"

version = "cvs-2"

description = {
  summary = "Lua Web Server Library",
  detailed = [[
    Xavante is a Lua HTTP 1.1 Web server that uses a modular architecture based on URI mapped handlers.
    This rock installs Xavante as a library that other applications can use.
  ]],
  license = "MIT/X11",
  homepage = "http://www.keplerproject.org/xavante"
}

dependencies = { 'luasocket', 'copas >= 1.1.5', 'luafilesystem >= 1.4.2' }

source = {
  url = "git://github.com/keplerproject/xavante.git"
}

local XAVANTE_LUAS = { "src/xavante/cgiluahandler.lua", 
	         "src/xavante/encoding.lua",
	         "src/xavante/filehandler.lua", 
	         "src/xavante/httpd.lua", 
	         "src/xavante/mime.lua", 
	         "src/xavante/patternhandler.lua", 
	         "src/xavante/redirecthandler.lua", 
	         "src/xavante/vhostshandler.lua", 
	         "src/xavante/indexhandler.lua", 
	         "src/xavante/urlhandler.lua", 
	         "src/xavante/ruleshandler.lua" }

build = {
   type = "module",
   modules = {
     sajax = "src/sajax/sajax.lua",
     xavante = "src/xavante/xavante.lua"
   }
}

for i = 1,#(XAVANTE_LUAS) do
   local src = XAVANTE_LUAS[i]
   local mod = "xavante." .. src:match("^src/xavante/([^%.]+)%.lua")
   build.modules[mod] = src 
end
