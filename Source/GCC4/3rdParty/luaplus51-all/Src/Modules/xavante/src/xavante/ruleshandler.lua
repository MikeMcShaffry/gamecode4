-----------------------------------------------------------------------------
-- Xavante URL rules handler
--
-- Authors: Javier Guerra and Fabio Mascarenhas
-- Copyright (c) 2006-2007 Kepler Project
--
-- $Id: ruleshandler.lua,v 1.2 2007/08/20 22:20:44 carregal Exp $
-----------------------------------------------------------------------------

-- this is a coroutine-based iterator:
-- path_perputer takes a path and yields once for each handler key to try
--		first is the full path
--		next, anything with the same extension on the same directory
--		next, anything on the directory
--		strips the last subdirectory from the path, and repeats the last two patterns
--		for example, if the query was /first/second/file.ext , tries:
--			/first/second/file.ext
--			/first/second/*.ext
--			/first/second/*
--			/first/*.ext
--			/first/*
--			/*.ext
--			/*
--		and, if the query was for a directory like /first/second/last/ , it tries:
--			/first/second/last/
--			/first/second/
--			/first/
--			/
local function path_permuter (path)
	coroutine.yield (path)
	local _,_,ext = string.find (path, "%.([^./]*)$")
	local notdir = (string.sub (path, -1) ~= "/")
	
	while path ~= "" do
		path = string.gsub (path, "/[^/]*$", "")
		if notdir then
			if ext then
				coroutine.yield (path .."/*."..ext)
			end
			coroutine.yield (path .."/*")
		else
			coroutine.yield (path.."/")
		end
	end
end

-- given a path, returns an iterator to traverse all permutations
local function path_iterator (path)
	return coroutine.wrap (function () path_permuter (path) end)
end

-- parses the url, and gets the appropiate handler function
-- starts with the full path, and goes up to the root
-- until it finds a handler for the request method
local function match_url (req, conf)
	local path = req.relpath
	local h, set
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

function xavante.ruleshandler (conf)
	if not conf or type (conf) ~= "table" then return nil end
	
	return function (req, res)
		match_url (req, conf)
		local h = req.handler or xavante.httpd.err_404
		return h (req, res)
	end
end
