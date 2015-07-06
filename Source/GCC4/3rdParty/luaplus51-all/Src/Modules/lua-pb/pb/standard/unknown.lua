-- Copyright (c) 2010, Robert G. Jakabosky <bobby@sharedrealm.com> All rights reserved.

local concat = table.concat
local setmetatable = setmetatable

local mt = {}
mt.__index = mt

local function new(tag)
	return setmetatable({tag = tag}, mt)
end

function mt:addField(tag, wire, value)
	self[#self + 1] = { tag=tag, wire=wire, value=value }
end

function mt:addVarint(tag, value)
	return self:addField(tag, 0, value)
end

function mt:addFixed32(tag, value)
	return self:addField(tag, 5, value)
end

function mt:addFixed64(tag, value)
	return self:addField(tag, 1, value)
end

function mt:addLengthDelimited(tag, value)
	return self:addField(tag, 2, value)
end

function mt:addGroup(tag)
	local field = new(tag)
	self:addField(tag, 3, field)
	return field
end

module(...)

_M.new = new

setmetatable(_M, {
__call = function(_) return new() end
})

