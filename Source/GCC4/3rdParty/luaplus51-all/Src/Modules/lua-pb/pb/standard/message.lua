-- Copyright (c) 2011, Robert G. Jakabosky <bobby@sharedrealm.com> All rights reserved.

local error = error
local assert = assert
local tostring = tostring
local setmetatable = setmetatable
local rawget = rawget

local mod_path = string.match(...,".*%.") or ''

local repeated = require(mod_path .. "repeated")
local new_repeated = repeated.new

local buffer = require(mod_path .. "buffer")
local new_buffer = buffer.new

local unknown = require(mod_path .. "unknown")
local new_unknown = unknown.new

local mod_parent_path = mod_path:match("(.*%.)[^.]*%.")
local utils = require(mod_parent_path .. "utils")
local copy = utils.copy

local _M = {}

local basic_types = {
-- Varint types
int32 = true, int64 = true,
uint32 = true, uint64 = true,
sint32 = true, sint64 = true,
bool = true,
-- 64-bit fixed
fixed64 = true, sfixed64 = true, double = true,
-- Length-delimited
string = true, bytes = true,
-- 32-bit fixed
fixed32 = true, sfixed32 = true, float = true,
}

local msg_tag = {}
local function new_message(mt, data)
	-- check if data is already a message of this type.
	if data and data[msg_tag] == mt then return data end
	-- create new message.
	local msg = setmetatable({ ['.data'] = {}}, mt)
	-- if data is nil, then message is empty.
	if not data then return msg end

	-- copy data into message
	local fields = mt.fields
	for i=1,#fields do
		local field = fields[i]
		local name = field.name
		local value = data[name]
		if value then
			msg[name] = value
		end
	end
	return msg
end
_M.new = new_message

function _M.def(parent, name, ast)
	local methods = {}
	local fields = copy(ast.fields)
	local tags = {}

	-- create Metatable for Message/Group.
	local is_group = (ast['.type'] == 'group')
	local mt = {
	name = name,
	is_message = not is_group,
	is_group = is_group,
	fields = fields,
	methods = methods,
	tags = tags,
	extensions = copy(ast.extensions),
	-- hid this metatable.
	__metatable = false,
	}

	function mt.__index(msg, name)
		local data = rawget(msg, '.data') -- field data.
		-- get field value.
		local value = data[name]
		-- field is already set, just return the value
		if value then return value end
		-- check field for a default value.
		local field = fields[name] -- field info.
		if field then return field.default end
		-- check methods
		local method = methods[name]
		if method then return method end
		-- check for unknown field.
		if name == 'unknown_fields' then
			-- create Unknown field set object
			value = new_unknown()
			data.unknown_fields = value
			return value
		end
		-- check for special 'msg_tag'
		if name == msg_tag then
			-- return metatable.  This is for message type validation.
			return mt
		end
		error("Invalid field:" .. name)
	end
	function mt.__newindex(msg, name, value)
		local data = rawget(msg, '.data') -- field data.
		-- get field info.
		local field = fields[name]
		if not field then error("Invalid field:" .. name) end
		-- check if field is a message/group
		local new = field.new
		if new then
			value = new(value)
		end
		data[name] = value
	end
	function mt.__tostring(msg)
		local data = rawget(msg, '.data') -- field data.
		local str = tostring(data)
		return str:gsub('table', name)
	end

	-- create message contructor
	local function new_msg(data)
		return new_message(mt, data)
	end
	mt.new = new_msg

	-- process fields
	for i=1,#fields do
		local field = fields[i]
		-- field rule to 'is_<rule>' mapping.
		field['is_' .. field.rule] = true
		-- check if the field is a basic type.
		if basic_types[field.ftype] then
			-- basic type
			field.is_basic = true
		else
			-- field is a user type, it needs to be resolved.
			field.need_resolve = true
		end
	end

	-- Type methods
	if is_group then
		-- store group tag in metatable.
		mt.tag = ast.tag
	else
		-- top-level message Serialize/Parse functions
		function methods:SerializePartial(format, depth)
			format = format or 'binary'
			local encode = mt.encode[format]
			if not encode then
				return false, "Unsupported serialization format: " .. format
			end
			return encode(self, depth)
		end
		function methods:Merge(data, format, off)
			format = format or 'binary'
			local decode = mt.decode[format]
			if not decode then
				return false, "Unsupported serialization format: " .. format
			end
			return decode(self, data, off or 1)
		end
		function methods:Serialize(format, depth)
			-- validate message before serialization.
			local init, errmsg = self:IsInitialized()
			if not init then return init, errmsg end
			-- now serialize message
			return self:SerializePartial(format, depth)
		end
		function methods:Parse(data, format, off)
			-- Clear message before parsing it.
			self:Clear()
			-- Merge message data into empty message.
			return self:Merge(data, format, off)
		end
	end

	-- common methods.
		-- Clear()
	function methods:Clear()
		local data = rawget(self, '.data') -- field data.
		for i=1,#fields do
			local field = fields[i]
			data[field.name] = nil
		end
	end
		-- IsInitialized()
	function methods:IsInitialized()
		local data = rawget(self, '.data') -- field data.
		for i=1,#fields do
			local field = fields[i]
			local name = field.name
			local val = data[name]
			if val then
				-- check if group/message/repeated fields are intializied
				if field.is_complex then
					local init, errmsg = val:IsInitialized()
					if not init then return init, errmsg end
				end
			elseif field.is_required then
				return false, "Missing required field: " .. name
			end
		end
		-- this group/message is intialized.
		return true
	end
		-- MergeFrom()
	function methods:MergeFrom(msg2)
		local data = rawget(self, '.data') -- field data.  This is for raw field access.
		for i=1,#fields do
			local field = fields[i]
			local name = field.name
			local val2 = msg2[name]
			if val2 then
				-- check if field is a group/message/repeated
				if field.is_complex then
					local val = data[name]
					-- create new group/message if field is nil.
					if not val then
						val = field.new()
						data[name] = val
					end
					-- merge sub group/message.
					val:MergeFrom(val2)
				else
					-- simple value, just copy it.
					data[name] = val2
				end
			end
		end
	end
		-- CopyFrom()
	function methods:CopyFrom(msg2)
		-- first clear this message.
		self:Clear()
		-- then merge new data.
		self:MergeFrom(msg2)
	end

	return mt
end

function _M.compile(node, mt, fields)
	local tags = mt.tags
	for i=1,#fields do
		local field = fields[i]
		-- packed arrays have a length
		field.has_length = field.is_packed
		-- get field tag
		local tag = field.tag
		-- check if the field is a user type
		local user_type_mt = field.user_type_mt
		if user_type_mt then
			-- get new function from metatable.
			field.new = user_type_mt.new
			if field.is_group then
				field.is_complex = true
			elseif user_type_mt.is_enum then
				field.is_enum = true
			else
				field.has_length = true
				field.is_message = true
				field.is_complex = true
			end
		elseif field.is_packed then
			-- packed basic type
			field.is_basic = true
		else
			-- basic type
			field.is_basic = true
		end
		-- if field is repeated, then create a new 'repeated' type for it.
		if field.is_repeated then
			field.new = new_repeated(field)
			field.is_complex = true
		end
		tags[tag] = field
	end
end

return _M
