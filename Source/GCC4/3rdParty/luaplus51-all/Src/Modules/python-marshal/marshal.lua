module(..., package.seeall)

local _handlers = {}

local function get_id(self)
	local id
	self.curPos, id = self.buffer:unpack('b', self.curPos)
	if not id then return nil end
--	print(id, string.char(id))
	return string.char(id)
end


local function load(self)
	local handler = _handlers[get_id(self)]
	if not handler then return nil end
	return handler(self)
end


local function loadall(self)
	local out = {}
	while true do
		local value = self:load()
		if not value then return out end
		out[#out + 1] = value
	end
end


local function get_dict(self)
	local hash = {}

	while true do
		local key = self:load()
		if not key then break end
		hash[key] = self:load()
	end

	return hash
end


local function get_none(self)
	return "**NONE**"
end


local function get_int(self)
	local value
	self.curPos, value = self.buffer:unpack('i', self.curPos)
	return value
end


local function get_long(self)
	local value
	self.curPos, value = self.buffer:unpack('l', self.curPos)
	return value
end


local function get_float(self)
	local size
	local value

	self.curPos, size = self.buffer:unpack('b', self.curPos)
	if size == 4 then
		self.curPos, value = self.buffer:unpack('f', self.curPos)
	elseif size == 8 then
		self.curPos, value = self.buffer:unpack('d', self.curPos)
	else
		error()
	end

	return value
end


local function get_string(self)
	local value
	self.curPos, value = self.buffer:unpack('a', self.curPos)
	return value
end


local function get_list(self)
	local list = {}
	local size
	self.curPos, size = self.buffer:unpack('I', self.curPos)
	for index = 1, size do
		list[index] = load()
	end
	return list
end


_handlers['N'] = get_none
_handlers['i'] = get_int
_handlers['l'] = get_long
_handlers['f'] = get_float
_handlers['s'] = get_string
_handlers['('] = get_list
_handlers['['] = get_list
_handlers['{'] = get_dict
--_handlers['x'] = get_complex

local metatable =
{
	load = load,
	loadall = loadall,
}

metatable.__index = metatable

function new(buffer)
	local marshal =
	{
		curPos = 1,
		buffer = buffer,
	}
	setmetatable(marshal, metatable)
	return marshal
end

