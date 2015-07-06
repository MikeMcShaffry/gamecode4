package = "CGILua"

version = "5.1.2rc1-1"

description = {
   summary = "Tool for creating dynamic Web pages and manipulating data from Web forms",
   detailed = [[
      CGILua allows the separation of logic and data handling from the
      generation of pages, making it easy to develop web applications with
      Lua. CGILua can be used with a variety of Web servers and, for each
      server, with different launchers. A launcher is responsible for the
      interaction of CGILua and the Web server, for example using ISAPI on
      IIS or mod_lua on Apache. 
   ]]
}

dependencies = {
   "lua >= 5.1",
   "luafilesystem 1.4.1rc1",
}

source = {
   url = "http://cgilua.luaforge.net/cgilua-5.1.2rc1.tar.gz"
}

build = {
   type = "make",
   build_pass = false,
   install_target = "install",
   install_variables = {
      LUA_DIR = "$(LUADIR)"
   }
}
