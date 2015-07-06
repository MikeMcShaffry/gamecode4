package = "Orbit"

version = "cvs-2"

description = {
  summary = "MVC for Lua Web Development",
  detailed = [[
     Orbit is a library for developing web applications according to
     the Model-View-Controller paradigm in Lua.
  ]],
  license = "MIT/X11",
  homepage = "http://www.keplerproject.org/orbit"
}

dependencies = { 'luafilesystem >= 1.5.0', 'lpeg >= 0.9', 'wsapi >= 1.3.4', 'mk' }

source = {
  url = "git://github.com/keplerproject/orbit.git",
}

build = {
   type = "builtin",
   modules = {
     orbit = "src/orbit.lua",
     ["orbit.model"] = "src/orbit/model.lua",
     ["orbit.schema"] = "src/orbit/schema.lua",
     ["orbit.pages"] = "src/orbit/pages.lua",
     ["orbit.ophandler"] = "src/orbit/ophandler.lua",
   },
   install = { bin = { "src/launchers/orbit", "src/launchers/op.cgi", "src/launchers/op.fcgi" } },
   copy_directories = { "doc", "samples", "test" }
}
