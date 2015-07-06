-----------------------------------------------------------------------------
-- Xavante URL paths handler
--
-- Authors: Javier Guerra
-- Copyright (c) 2006-2007 Kepler Project
--
-- $Id: urlhandler.lua,v 1.3 2007/08/20 22:20:44 carregal Exp $
-----------------------------------------------------------------------------

local function path_p (s, p)
	if not p then return s end
	if p=="" then return nil end
	return string.gsub (p, "[^/]*/?$", "")
end 

local function path_iterator (path)
	return path_p, path
end

local function match_url (req, conf)
	local path = req.relpath
	local h = nil
	for p in path_iterator (path) do
		h = conf [p]
		if h then
			req.match = p
			break
		end
	end
	
	if req.match then
		local _,_,pfx = string.find (req.match, "^(.*/)[^/]-$")
		assert (string.sub (path, 1, string.len (pfx)) == pfx)
		req.relpath = string.sub (path, string.len (pfx)+1)
	end
	req.handler = h
end

function xavante.urlhandler (conf)
	if not conf or type (conf) ~= "table" then return nil end
	
	return function (req, res)
		match_url (req, conf)
		local h = req.handler or xavante.httpd.err_404
		return h (req, res)
	end
end