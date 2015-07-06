-- CGILua user configuration file
-- $Id: config.lua,v 1.1 2008/06/30 14:30:00 carregal Exp $

-- Looks for a Lua script in the web directory with the same name as
-- the executable
-- Useful for shorter URLs, just be careful with an eventual name clashing
-- when using this option
-- cgilua.use_executable_name = true

-- Enables CGILua authentication
-- cgilua.doif (CGILUA_CONF.."/authentication_conf.lua")

-- Emulating old behavior loading file "env.lua" from the script's directory
--[[
cgilua.addopenfunction (function ()
  cgilua.doif ("env.lua")
end)
--]]

-- Basic configuration for using sessions
--[[
require"cgilua.session"
cgilua.session.setsessiondir (CGILUA_TMP)
-- The following function must be called by every script that needs session.
local already_enabled = false
function cgilua.enablesession ()
  if already_enabled then
    return
  else
    already_enabled = true
  end
  cgilua.session.open ()
  cgilua.addclosefunction (cgilua.session.close)
end
--]]

-- Optional compatibility values
-- cgilua.preprocess = cgilua.handlelp
-- cgilua.includehtml = cgilua.lp.include
