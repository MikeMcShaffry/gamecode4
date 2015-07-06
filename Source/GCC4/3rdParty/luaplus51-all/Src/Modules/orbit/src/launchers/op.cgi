#!/usr/bin/lua

-- Orbit pages launcher, extracts script to launch
-- from SCRIPT_FILENAME/PATH_TRANSLATED

pcall(require, "luarocks.require")

local common = require "wsapi.common"
local cgi = require "wsapi.cgi"

local ok, err = pcall(require, "cosmo")

if not ok then
  io.stderr:write("Cosmo not loaded:\n" .. err .. "\n\nPlease install cosmo with LuaRocks\n")
  os.exit(1)
end

local op = require "orbit.pages"

local arg_filename = (...)

local function op_loader(wsapi_env)
  common.normalize_paths(wsapi_env, arg_filename, "op.cgi", { "SCRIPT_FILENAME", "PATH_TRANSLATED" })
  return op.run(wsapi_env)
end 

cgi.run(op_loader)
