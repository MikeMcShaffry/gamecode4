package = "Coxpcall"

version = "cvs-2"

description = {
  summary = "Coroutine safe xpcall and pcall",
  detailed = [[
 Encapsulates the protected calls with a coroutine based loop, so errors can
 be dealed without the usual Lua 5.x pcall/xpcall issues with coroutines
 yielding inside the call to pcall or xpcall.
  ]],
  license = "MIT/X11",
  homepage = "http://coxpcall.luaforge.net"
}

dependencies = { }

source = {
  url = "git://github.com/keplerproject/coxpcall.git"
}

build = {
   type = "module",
   modules = { coxpcall = "src/coxpcall.lua" }
}
