-----------------------------------------------------------------------------
-- A Browser Class for easy Web Automation with Lua-cURL
-- Author: Kai Uwe Jesussek
-- RCS ID: $Id: browser.lua,v 0.1 2011/03/11 23:55:20 kai Exp $
-----------------------------------------------------------------------------

if cURL == nil then
	local cURL = require("cURL")
end
if table == nil then
	local table = require("table")
end
if string == nil then
	local string = require("string")
end

local string = require("string")
local base = _G
local table = require("table")

USERAGENT = "Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1)" --windows xp internet explorer 6.0

--this function joins 2 urls (absolute or relative)
function url_join(_base, _url)
	assert(type(_url) == "string")

	if _base == nil or _base == "" then
		return _url
	end

	assert(type(_base) == "string")
	local base = url_split(_base)
	local url = url_split(_url)


	local protocol = base.protocol
	local host = base.host

	local path = ""
	local port = ""

	if url.protocol ~= nil then
		protocol = url.protocol
		if url.path ~= nil then
			path = url.path
		end
		if url.port ~= nil and url.port ~= "" then
			port = url.port
		end
		if url.host ~= nil then
			host = url.host
		end
	else
		if _url:sub(1,2) == "//" then
			--set host and path
			host, port, path = _url:match("^//([^;/%?]+)(:?%d*)(/?.*)")
			if path == nil then
				path = ""
			end
		elseif _url:sub(1,1) == "/" then
			port = base.port
			--replace path
			path = _url
		else
			--combine paths :(
			path = base.path:match("^(.*)/[^/]*")
			port = base.port
			if path ~= nil then
				path = path .. "/" .. _url
			else
				path = _url
			end
		end

	end
	local ret = protocol .. "://" .. host .. port .. path
	return ret
end

--this function splits an url into its parts
function url_split(_url)
	--print(_url)
	local ret = {}
	--test ipv6
	ret.protocol, ret.host, ret.port, ret.path = _url:match("^(https?)://(%[[0-9a-fA-F:]+%])(:?%d*)(.*)$")

	if ret.host == nil then
		--fall back to ipv4
		ret.protocol, ret.host, ret.port, ret.path = _url:match("^(https?)://([^:/]+)(:?%d*)(.*)$")
	end
	return ret
end


-----------------------------------------------------------------------------
-- Encodes a string into its escaped hexadecimal representation
-- Input
--   s: binary string to be encoded
-- Returns
--   escaped representation of string binary
-- taken from Lua Socket and added underscore to ignore (MIT-License)
-----------------------------------------------------------------------------
function escape(s)
    return string.gsub(s, "([^A-Za-z0-9_])", function(c)
        return string.format("%%%02x", string.byte(c))
    end)
end

-----------------------------------------------------------------------------
-- Encodes a string into its escaped hexadecimal representation
-- Input
--   s: binary string to be encoded
-- Returns
--   escaped representation of string binary
-- taken from Lua Socket
-----------------------------------------------------------------------------
function unescape(s)
    return string.gsub(s, "%%(%x%x)", function(hex)
        return string.char(base.tonumber(hex, 16))
    end)
end

-- from encodes a key, value dictionary table
function tblencode (_arguments)
	local ret = ""
	if _arguments == nil or next(_arguments) == nil then   -- no _arguments or empty _arguments?
		return ret
	end

	--iterate over each key -> value pairs and urlencode them
	for key, vals in pairs(_arguments) do
		if type(vals) ~= "table" then
			vals = {vals}
		end
		for i,val in ipairs(vals) do
			ret = ret .. "&"..key.. "=" ..escape(val)
		end
	end

	--cut off leadin '&'
	return string.sub(ret,2)
end


--function helper for result
--taken from luasocket page (MIT-License)
local function build_w_cb(t)
	return function(s,len)
		table.insert(t, s)
	return len,nil
	end
end

--function helper for headers
--taken from luasocket page (MIT-License)
local function h_build_w_cb(t)
	return function(s,len)
		--stores the received data in the table t
		--prepare header data
		name, value = s:match("(.-): (.+)")
		if name and value then
			t.headers[name] = value:gsub("[\n\r]", "")
		else
			code, codemessage = string.match(s, "^HTTP/.* (%d+) (.+)$")
			if code and codemessage then
				t.code = tonumber(code)
				t.codemessage = codemessage:gsub("[\n\r]", "")
			end
		end
	return len,nil
	end
end


--the browser easy to use interface
browser = {}
function browser:new(_share)
	if _share == nil then
		_share = cURL.share_init()
		_share:setopt_share("COOKIE")
		_share:setopt_share("DNS")
	end
	local object = { url = nil, share = _share}
	setmetatable(object, {__index = browser})
	return object
end

--this function sets the proxy variables for the prepare function
function browser:setProxy(_proxy, _proxytype)
	self.proxy = _proxy
	self.proxytype = _proxytype
	print("setting proxy", self.proxy, self.proxytype)
end

--this function prepares a request
function browser:prepare(post_data, urlencoded)
	local req = cURL.easy_init()
	req:setopt_share(self.share)
	req:setopt_url(self.url)
	req:setopt_useragent(USERAGENT)
	if self.proxy ~= nil and self.proxytype ~= nil then
		req:setopt_proxy(self.proxy)
		req:setopt_proxytype(self.proxytype)
	end
	if self.caInfoPath ~= nil then
		req:setopt_cainfo(self.caInfoPath)
	end
	if post_data ~= nil then
		if urlencoded and type(post_data) == "table" then
			post_data = tblencode(post_data)
		end

		if type(post_data) == "string" then
			req:setopt_post(1)
			req:setopt_postfields(post_data)
			req:setopt_postfieldsize(#post_data)
		else
			req:post(post_data)
		end
	end
	return req
end

--this function sets the url
function browser:setUrl(url)
	--appends a leading / to url if needed
	if self.url and self.url:match("^(https?://[^/]+)$") then
		self.url = self.url .. "/"
	end
	self.url = url_join(self.url or "", url)
end

--opens a webpage :) only the first parameter is required
function browser:open(url, post_data, redirect, urlencoded)
	local redirect = redirect or true
	local urlencoded = urlencoded == nil
	local ret = {}
	response_body = {}
	ret.headers = {}

	self:setUrl(url)
	local req = self:prepare(post_data, urlencoded)

	req:perform({headerfunction=h_build_w_cb(ret), writefunction=build_w_cb(response_body)})

	self:setUrl(url)
	ret.body = table.concat(response_body)

	if redirect and ret.headers and (ret.headers.Location or ret.headers.location) and (ret.code == 301 or ret.code == 302) then
		return self:open(url_join(self.url, ret.headers.Location or ret.headers.location), nil, extra_headers, redirect)
	end
	return ret

end
--opens a webpage :) only the first and second parameters are required
function browser:save(url, filename, post_data)
	local ret = {}
	ret.headers = {}
	self:setUrl(url)
	local req = self:prepare(post_data, false)
	file = io.open(filename)
	req:perform({headerfunction=h_build_w_cb(ret), writefunction=function(str) file:write(str) end })
	file:close()
end

function browser:setCaInfo(path)
	self.caInfoPath = path
end

--[[ usage examples:
-- b = browser:new()
-- resp = b:open("http://www.html-kit.com/tools/cookietester/")
-- print(resp.body)
-- table.foreach(resp.headers, print)
--]]
b = browser:new()
resp = b:open("http://www.html-kit.com/tools/cookietester/")
print(resp.body)
table.foreach(resp.headers, print)
