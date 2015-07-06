-----------------------------------------------------------------------------
-- Xavante URL patterns handler
--
-- Authors: Fabio Mascarenhas
-- Copyright (c) 2006 Kepler Project
--
-- $Id: patternhandler.lua,v 1.2 2008/03/10 23:38:31 mascarenhas Exp $
-----------------------------------------------------------------------------

local function path_iterator (path)
  return path_p, path
end

local function match_url (req, conf)
  local path = req.relpath
  for _, rule in ipairs(conf) do
    for _, pat in ipairs(rule.pattern) do
      local cap = { string.match(path, pat) }
      if #cap > 0 then 
	req.handler = rule.handler
	return cap 
      end
    end
  end
end

function xavante.patternhandler (conf)
  if not conf or type (conf) ~= "table" then return nil end
  
  return function (req, res)
	   local cap = match_url (req, conf) or {}
	   local h = req.handler or xavante.httpd.err_404
	   return h (req, res, cap)
	 end
end
