-----------------------------------------------------------------------------
-- Xavante webDAV Properties with file backend
-- Author: Javier Guerra
-- Copyright (c) 2005 Javier Guerra
-----------------------------------------------------------------------------

require "lfs"

module (arg and arg[1])

local props_mt = { __index = {} }
local props = props_mt.__index

-- equivalent to 'mkdir -p path'
local function mkdir_p (path)
	local walk = ""
	for p in string.gfind (path, "[^/]+") do
		walk = walk.."/"..p
		attr = lfs.attributes (walk)
		if not attr then
			lfs.mkdir (walk)
		else
			if attr.mode ~= "directory" then
				return nil, string.format ("%s existe y no es directorio", walk)
			end
		end
	end
end

function props:getPropNames (path)
	path = self.rootDir..path
	local function gen ()
		local attr = lfs.attributes (path)
		if not attr or attr.mode ~= "directory" then return nil end

		for prop in lfs.dir (path) do
			if string.sub (prop, 1,1) ~= "." then
				coroutine.yield (prop)
			end
		end
	end

	return coroutine.wrap (gen)
end

function props:getProp (path, propname)
	local f = io.open (self.rootDir..path.."/"..propname)
	if not f then return nil end

	local val = f:read ("*a")
	f:close ()

	return val
end

function props:setProp (path, propname, value)
	if not value then
		os.remove (self.rootDir..path.."/"..propname)
		return
	end
	
	mkdir_p (self.rootDir..path)
	local f = assert (io.open (self.rootDir..path.."/"..propname, "wb"))
	
	f:write (value)
	f:close ()
end

function makeProps (params)
	params = params or {}
	
	params.rootDir = params.rootDir or ".PROPS/"
	mkdir_p (params.rootDir)

	return setmetatable (params, props_mt)
end