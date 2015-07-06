package = "CGILua"

version = "cvs-3"

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
   "luafilesystem cvs",
}

source = {
   url = "cvs://:pserver:anonymous@cvs.luaforge.net:/cvsroot/cgilua",
   cvs_tag = "HEAD",
   dir = "cgilua"
}

build = {
   type = "make",
   build_pass = false,
   install_target = "install",
   install_variables = {
      LUA_DIR = "$(LUADIR)"
   }
}
