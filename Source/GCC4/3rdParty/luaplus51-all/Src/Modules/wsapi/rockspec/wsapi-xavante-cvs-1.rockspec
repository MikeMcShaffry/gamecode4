package = "WSAPI-Xavante"

version = "cvs-1"

description = {
  summary = "Lua Web Server API - Xavante Handler",
  detailed = [[
    WSAPI is an API that abstracts the web server from Lua web applications. This is the rock
    that contains the Xavante adapter and launcher.
  ]],
  license = "MIT/X11",
  homepage = "http://www.keplerproject.org/wsapi"
}

dependencies = { "wsapi cvs", "rings >= 1.2.3", "coxpcall >= 1.13", "xavante >= 2.2.0" }

source = {
  url = "git://github.com/keplerproject/wsapi.git"
}

build = {
    type = "builtin",
    modules = {
	  ["wsapi.xavante"] = "src/wsapi/xavante.lua"
    },
    install = { bin = { "src/launcher/wsapi" } }
}
