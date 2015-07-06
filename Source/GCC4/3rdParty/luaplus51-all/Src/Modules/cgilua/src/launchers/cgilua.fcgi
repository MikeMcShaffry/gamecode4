#!/usr/bin/env lua

-- CGILua (SAPI) launcher, extracts script to launch
-- either from the command line (use #!cgilua in the script)
-- or from SCRIPT_FILENAME/PATH_TRANSLATED

pcall(require, "luarocks.require")

local common = require "wsapi.common"

local ok, err = pcall(require, "wsapi.fastcgi")

if not ok then
  io.stderr:write("WSAPI FastCGI not loaded:\n" .. err .. "\n\nPlease install wsapi-fcgi with LuaRocks\n")
  os.exit(1)
end

local ONE_HOUR = 60 * 60
local ONE_DAY = 24 * ONE_HOUR

local sapi_loader = common.make_isolated_launcher{
  filename = nil,           -- if you want to force the launch of a single script
  launcher = "cgilua.fcgi", -- the name of this launcher
  modname = "wsapi.sapi",   -- WSAPI application that processes the script
  reload = false,           -- if you want to reload the application on every request
  period = ONE_HOUR,        -- frequency of Lua state staleness checks
  ttl = ONE_DAY,            -- time-to-live for Lua states
  vars =                    -- order of checking for the path of the script
   { "SCRIPT_FILENAME",
     "PATH_TRANSLATED" } 
}

wsapi.fastcgi.run(sapi_loader)
