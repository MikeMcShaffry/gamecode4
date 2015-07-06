package = "Coxpcall"

version = "1.13.0-2"

source = {
  url = "http://luaforge.net/frs/download.php/3406/coxpcall-1.13.0.tar.gz",
}

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

build = {
   type = "module",
   modules = { coxpcall = "src/coxpcall.lua" }
}
