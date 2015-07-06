-----------------------------------------------------------------------------
-- Xavante webDAV file repository
-- Author: Javier Guerra
-- Copyright (c) 2005 Javier Guerra
-----------------------------------------------------------------------------

require "lfs"
require "xavante.mime"
require "socket.url"

module (arg and arg[1])

local source_mt = { __index = {} }
local source = source_mt.__index

local resource_mt = { __index = {} }
local resource = resource_mt.__index

function source:getRoot ()
	return self.rootDir
end


function source:getResource (rootUrl, path)
	local diskpath = self.rootDir .. path
	local attr = lfs.attributes (diskpath)
	if not attr then return end

	local _,_,pfx = string.find (rootUrl, "^(.*/)[^/]-$")

	if attr.mode == "directory" and string.sub (path, -1) ~= "/" then
		path = path .."/"
	end
	
	return setmetatable ({
		source = self,
		path = path,
		diskpath = diskpath,
		attr = attr,
		pfx = pfx
	}, resource_mt)
end

function source:createResource (rootUrl, path)
	local diskpath = self.rootDir .. path
	local attr = lfs.attributes (diskpath)
	if not attr then
		io.open (diskpath, "w"):close ()
		attr = lfs.attributes (diskpath)
	end
	
	local _,_,pfx = string.find (rootUrl, "^(.*/)[^/]-$")

	return setmetatable ({
		source = self,
		path = path,
		diskpath = diskpath,
		attr = attr,
		pfx = pfx
	}, resource_mt)
end

function source:createCollection (rootUrl, path)
	local diskpath = self.rootDir .. path
	return lfs.mkdir (diskpath)
end

local _liveprops = {}

_liveprops["DAV:creationdate"] = function (self)
	return os.date ("!%a, %d %b %Y %H:%M:%S GMT", self.attr.change)
end

_liveprops["DAV:displayname"] = function (self)
	local name = ""
	for part in string.gfind (self.path, "[^/]+") do
		name = part
	end
	return name
end

_liveprops["DAV:source"] = function (self)
	return self:getHRef ()
end


_liveprops["DAV:supportedlock"] = function (self)
	return [[<D:lockentry>
<D:lockscope><D:exclusive/></D:lockscope>
<D:locktype><D:write/></D:locktype>
</D:lockentry>
<D:lockentry>
<D:lockscope><D:shared/></D:lockscope>
<D:locktype><D:write/></D:locktype>
</D:lockentry>]]
end


_liveprops["DAV:getlastmodified"] = function (self)
	return os.date ("!%a, %d %b %Y %H:%M:%S GMT", self.attr.modification)
end

_liveprops["DAV:resourcetype"] = function (self)
	if self.attr.mode == "directory" then
		return "<D:collection/>"
	else
		return ""
	end
end

_liveprops["DAV:getcontenttype"] = function (self)
	return self:getContentType ()
end
_liveprops["DAV:getcontentlength"] = function (self)
	return self:getContentSize ()
end

function resource:getContentType ()
	if self.attr.mode == "directory" then
		return "httpd/unix-directory"
	end
	local _,_,exten = string.find (self.path, "%.([^.]*)$")
	exten = exten or ""
	return xavante.mimetypes [exten] or ""
end

function resource:getContentSize ()
	if self.attr.mode == "file" then
		return self.attr.size
	else return 0
	end
end

function resource:getContentData ()
	local function gen ()
		local f = io.open (self.diskpath, "rb")
		if not f then
			return
		end

		local block
		repeat
			block = f:read (8192)
			if block then
				coroutine.yield (block)
			end
		until not block
		f:close ()
	end

	return coroutine.wrap (gen)
end

function resource:addContentData (b)
	local f = assert (io.open (self.diskpath, "a+b"))
	f:seek ("end")
	f:write (b)
	f:close ()
end

function resource:delete ()
	local ok, err = os.remove (self.diskpath)
	if not ok then
		err = string.format ([[HTTP/1.1 424 %s]], err)
	end
	return ok, err
end

function resource:getItems (depth)
	local gen
	local path = self.path
	local diskpath = self.diskpath
	local rootdir = self.source.rootDir

	if depth == "0" then
		gen = function () coroutine.yield (self) end

	elseif depth == "1" then
		gen = function ()
				if self.attr.mode == "directory" then
					if string.sub (diskpath, -1) ~= "/" then
						diskpath = diskpath .."/"
					end
					if string.sub (path, -1) ~= "/" then
						path = path .."/"
					end
					for entry in lfs.dir (diskpath) do
						if string.sub (entry, 1,1) ~= "." then
							coroutine.yield (self.source:getResource (self.pfx, path..entry))
						end
					end
				end
				coroutine.yield (self)
			end

	else
		local function recur (p)
			local attr = assert (lfs.attributes (rootdir .. p))
			if attr.mode == "directory" then
				for entry in lfs.dir (rootdir .. p) do
					if string.sub (entry, 1,1) ~= "." then
						recur (p.."/"..entry)
					end
				end
			coroutine.yield (self.source:getResource (self.pfx, p))
			end
		end
		gen = function () recur (path) end
	end
	
	if gen then return coroutine.wrap (gen) end
end

function resource:getPath ()
	return self.path
end

function resource:getHRef ()
	local _,_,sfx = string.find (self.path, "^/*(.*)$")
	return self.pfx..sfx
end

function resource:getPropNames ()
	return pairs (_liveprops)
end

function resource:getProp (propname)
	local liveprop = _liveprops [propname]
	if liveprop then
		return liveprop (self)
	end
end

function resource:setProp (propname, value)
	return false
end

function makeSource (params)
	params = params or {}
	params.rootDir = params.rootDir or "./"

	return setmetatable (params, source_mt)
end