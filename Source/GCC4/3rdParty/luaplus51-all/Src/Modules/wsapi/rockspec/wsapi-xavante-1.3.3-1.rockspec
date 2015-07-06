package = "WSAPI-Xavante"

version = "1.3.3-1"

description = {
  summary = "Lua Web Server API - Xavante Handler",
  detailed = [[
    WSAPI is an API that abstracts the web server from Lua web applications. This is the rock
    that contains the Xavante adapter and launcher.
  ]],
  license = "MIT/X11",
  homepage = "http://www.keplerproject.org/wsapi"
}

dependencies = { "wsapi >= 1.3.3", "rings >= 1.2.3", "coxpcall >= 1.13", "xavante >= 2.2.0" }

source = {
  url = "http://github.com/downloads/keplerproject/wsapi/wsapi-1.3.3.tar.gz"
}

build = {
    type = "builtin",
    modules = {
	  ["wsapi.xavante"] = "src/wsapi/xavante.lua"
    },
    install = { bin = { "src/launcher/wsapi" } }
}
