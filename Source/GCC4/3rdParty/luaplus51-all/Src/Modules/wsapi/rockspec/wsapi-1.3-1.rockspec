package = "WSAPI"

version = "1.3-1"

description = {
  summary = "Lua Web Server API",
  detailed = [[
    WSAPI is an API that abstracts the web server from Lua web applications. This is the rock
    that contains the base WSAPI libraries plus the CGI adapters.
  ]],
  license = "MIT/X11",
  homepage = "http://www.keplerproject.org/wsapi"
}

dependencies = { "luafilesystem >= 1.5.0" }

source = {
  url = "http://github.com/downloads/keplerproject/wsapi/wsapi-1.3.tar.gz"
}

build = {
  type = "builtin",
  modules = {
    ["wsapi.common"] = "src/wsapi/common.lua",
    ["wsapi.request"] = "src/wsapi/request.lua",
    ["wsapi.response"] = "src/wsapi/response.lua",
    ["wsapi.util"] = "src/wsapi/util.lua",
    ["wsapi.cgi"] = "src/wsapi/cgi.lua",
    ["wsapi.sapi"] = "src/wsapi/sapi.lua",
    ["wsapi.ringer"] = "src/wsapi/ringer.lua",
  },
  copy_directories = { "samples", "doc", "tests" },
  install = { bin = { "src/launcher/wsapi.cgi" } }
}
