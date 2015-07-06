#!/usr/bin/env cgilua.cgi

-- Basic application bootstraper
local name, path_info = cgilua.splitonfirst(cgilua.script_vpath)

if name == nil or name == "" then
  if io.open("index.lua") then
    cgilua.handle("index.lua")
  elseif io.open("index.lp") then
    cgilua.handle("index.lp")
  else
    error("No script defined")
  end
else
  local appdir = CGILUA_APPS.."/"..name
  local appinit = "init.lua"
  lfs.chdir(appdir)

  if io.open(appinit) then
    -- checks for authentication if necessary
    if cgilua.authentication and not cgilua.authentication.username() then
      cgilua.redirect(cgilua.authentication.checkURL())
    else
      -- prepares the application environment
      package.path = CGILUA_APPS.."/"..name.."/lua/?.lua;"..package.path
      cgilua.script_pdir = CGILUA_APPS.."/"..name
      cgilua.script_file = nil
      cgilua.script_vdir = "/"
      cgilua.urlpath = cgilua.urlpath.."/"..name
      cgilua.script_vpath = path_info
      cgilua.app_name = name

      cgilua.doif(appinit)
    end
  else
    cgilua.handle(cgilua.script_pdir..name..path_info)
  end
end
