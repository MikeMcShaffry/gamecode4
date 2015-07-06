#!/usr/bin/lua

-- Orbit pages launcher, extracts script to launch

pcall(require, "luarocks.require")

local common = require "wsapi.common"

local ok, err = pcall(require, "wsapi.fastcgi")

if not ok then
  io.stderr:write("WSAPI FastCGI not loaded:\n" .. err .. "\n\nPlease install wsapi-fcgi with LuaRocks\n")
  os.exit(1)
end

local ok, err = pcall(require, "cosmo")

if not ok then
  io.stderr:write("Cosmo not loaded:\n" .. err .. "\n\nPlease install cosmo with LuaRocks\n")
  os.exit(1)
end

local ONE_HOUR = 60 * 60
local ONE_DAY = 24 * ONE_HOUR

local op_loader = common.make_isolated_launcher{
  filename = nil,          -- if you want to force the launch of a single script
  launcher = "op.fcgi",    -- the name of this launcher
  modname = "orbit.pages", -- WSAPI application that processes the script
  reload = false,          -- if you want to reload the application on every request
  period = ONE_HOUR,       -- frequency of Lua state staleness checks
  ttl = ONE_DAY,           -- time-to-live for Lua states
  vars =                   -- order of checking for the path of the script
   { "SCRIPT_FILENAME",
     "PATH_TRANSLATED" } 
}

wsapi.fastcgi.run(op_loader)
