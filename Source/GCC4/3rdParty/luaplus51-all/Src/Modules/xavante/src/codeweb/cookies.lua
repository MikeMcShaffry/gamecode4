-----------------------------------------------------------------------------
-- Xavante Cookies module
--
-- Author: Javier Guerra
-- inspired by CGILua cookies
-- Copyright (c) 2005 Kepler Project
-----------------------------------------------------------------------------

local url = require "socket.url"

module (arg and arg[1])

local function read_cookies (req)
	req.cookies = req.cookies or {}
	
	local cookies = req.headers.cookie or ""
	local lastcookie
	
	-- get all key="val" pairs
	for k,v in string.gfind (cookies, '([^%s;=]+)%s*=%s*"([^"]*)"') do
	
		if string.sub (k,1,1) == "$" then
			-- its an option for the last seen cookie
			if lastcookie then
				local optname = string.sub (k,2)
				req.cookies [lastcookie].options [optname] = v
			end
		else
			-- its a new cookie
			req.cookies [k] = {value = v, options = {}}
			lastcookie = k
		end
	end
end

function get (req, name)
	if not req.cookies then
		read_cookies (req)
	end
	
	local cookie_rec = req.cookies and req.cookies [name]
	if cookie_rec then
		return cookie_rec.value, cookie_rec.options
	end
end

function set (res, name, value, options)
	res.cookies = res.cookies or {}
	res.cookies [name] = {value = value, options = options}
end

function delete (res, name, options)
  options = options or {}
  options.expires = 1
  set (res, name, "xxx", options)
end

function set_res_cookies (res)
	if not res or not res.cookies then
		return
	end
	
	local set_cookie = {}
	for name, r in pairs (res.cookies) do
		local set_string = string.format ([[%s="%s";Version="1"]], name, r.value)
		
		local options = r.options
		if options then 
			if type (options.expires) == "number" then
				options.expires = os.date("!%A, %d-%b-%Y %H:%M:%S GMT", options.expires)
			end
			
			for k,v in pairs (r.options) do
				if v == "" or type (v) == "boolean" then
					set_string = set_string..string.format ([[;%s]], k)
				else
					set_string = set_string..string.format ([[;%s="%s"]], k, v)
				end
			end
		end
		
		table.insert (set_cookie, set_string)
	end
	res.headers ["Set-Cookie"] = table.concat (set_cookie)
end