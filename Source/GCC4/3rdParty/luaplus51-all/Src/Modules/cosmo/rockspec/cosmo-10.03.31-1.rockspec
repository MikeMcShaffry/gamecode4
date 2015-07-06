package = "Cosmo"

version = "10.03.31-1"

description = {
  summary = "Safe templates for Lua",
  detailed = [[
Cosmo is a "safe templates" engine. It allows you to fill nested templates, 
providing many of the advantages of Turing-complete template engines, 
without without the downside of allowing arbitrary code in the templates.
  ]],
  license = "MIT/X11",
  homepage = "http://cosmo.luaforge.net"
}

dependencies = { "lpeg >= 0.9" }

source = {
  url = "http://github.com/downloads/mascarenhas/cosmo/cosmo-10.03.31.tar.gz"
}

build = {
   type = "builtin",
   modules = {
     cosmo = "src/cosmo.lua",
     ["cosmo.fill"] = "src/cosmo/fill.lua",
     ["cosmo.grammar"] = "src/cosmo/grammar.lua",
   },
   copy_directories = { "doc", "samples", "tests" }
}
