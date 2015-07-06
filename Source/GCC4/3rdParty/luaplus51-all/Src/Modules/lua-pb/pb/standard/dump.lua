-- Copyright (c) 2010-2011 by Robert G. Jakabosky <bobby@neoawareness.com>
--
-- Permission is hereby granted, free of charge, to any person obtaining a copy
-- of this software and associated documentation files (the "Software"), to deal
-- in the Software without restriction, including without limitation the rights
-- to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
-- copies of the Software, and to permit persons to whom the Software is
-- furnished to do so, subject to the following conditions:
--
-- The above copyright notice and this permission notice shall be included in
-- all copies or substantial portions of the Software.
--
-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
-- IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
-- FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
-- AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
-- LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
-- OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
-- THE SOFTWARE.

local assert = assert
local pairs = pairs
local print = print
local error = error
local tostring = tostring
local setmetatable = setmetatable
local type = type
local sformat = string.format
local char = string.char

local mod_path = string.match(...,".*%.") or ''

local buffer = require(mod_path .. "buffer")
local new_buffer = buffer.new

local function append(buf, off, data)
	off = off + 1
	buf[off] = data
	return off
end

local function indent(buf, off, depth)
	return append(buf, off, ('  '):rep(depth))
end

--
-- Safe strings
--
local escapes = {}
for i=0,255 do
	escapes[char(i)] = sformat('\\%03o', i)
end
escapes['"'] = '\\"'
escapes["'"] = "\\'"
escapes["\\"] = "\\\\"
escapes["\r"] = "\\r"
escapes["\n"] = "\\n"
escapes["\t"] = "\\t"
-- safe chars
local safe = [=[`~!@#$%^&*()_-+={}[]|:;<>,.?/]=]
for i=1,#safe do
	local c = safe:sub(i,i)
	escapes[c] = c
end
local function safe_string(data)
	return data:gsub([[([^%w ])]], escapes)
end

module(...)

----------------------------------------------------------------------------------
--
--  dump code.
--
----------------------------------------------------------------------------------

local basic = {
-- varint types
int32 = function(buf, off, val)
	return append(buf, off, sformat("%d", val))
end,
int64 = function(buf, off, val)
	return append(buf, off, sformat("%d", val))
end,
sint32 = function(buf, off, val)
	return append(buf, off, sformat("%d", val))
end,
sint64 = function(buf, off, val)
	return append(buf, off, sformat("%d", val))
end,
uint32 = function(buf, off, val)
	return append(buf, off, sformat("%u", val))
end,
uint64 = function(buf, off, val)
	return append(buf, off, sformat("%u", val))
end,
bool = function(buf, off, val)
	return append(buf, off, (val == 0) and "false" or "true")
end,
enum = function(buf, off, val)
	return append(buf, off, val)
end,
-- 64-bit fixed
fixed64 = function(buf, off, val)
	return append(buf, off, sformat("%u", val))
end,
sfixed64 = function(buf, off, val)
	return append(buf, off, sformat("%d", val))
end,
double = function(buf, off, val)
	return append(buf, off, tostring(val))
end,
-- Length-delimited
string = function(buf, off, val)
	off = append(buf, off, '"')
	off = append(buf, off, safe_string(val))
	return append(buf, off, '"')
end,
bytes = function(buf, off, val)
	off = append(buf, off, '"')
	off = append(buf, off, safe_string(val))
	return append(buf, off, '"')
end,
-- 32-bit fixed
fixed32 = function(buf, off, val)
	return append(buf, off, sformat("%u", val))
end,
sfixed32 = function(buf, off, val)
	return append(buf, off, sformat("%d", val))
end,
float = function(buf, off, val)
	return append(buf, off, sformat("%.8g", val))
end,
}

local dump_fields
local dump_unknown_fields

local wire_types = {
[0] = function(buf, off, val, depth)
	return append(buf, off, sformat(": %u", val))
end,
[1] = function(buf, off, val, depth)
	return append(buf, off, sformat(": 0x%016x", val))
end,
[2] = function(buf, off, val, depth)
	if type(val) == 'table' then
		off = append(buf, off, " {\n")
		off = dump_unknown_fields(buf, off, val, depth + 1)
		off = indent(buf, off, depth)
		return append(buf, off, "}")
	end
	off = append(buf, off, ': "')
	off = append(buf, off, safe_string(val))
	return append(buf, off, '"')
end,
[3] = function(buf, off, val, depth)
	off = append(buf, off, " {\n")
	off = dump_unknown_fields(buf, off, val, depth + 1)
	off = indent(buf, off, depth)
	return append(buf, off, "}")
end,
[4] = nil, -- End group
[5] = function(buf, off, val, depth)
	return append(buf, off, sformat(": 0x%08x", val))
end,
}

local function dump_field(buf, off, field, val, depth)
	-- indent
	off = indent(buf, off, depth)
	-- dump field name
	off = append(buf, off, field.name)

	-- dump field
	local dump = field.dump
	if dump then
		if field.is_enum then
			off = append(buf, off, ": ")
			off = dump(buf, off, val)
		else
			off = append(buf, off, " {\n")
			off = dump(buf, off, val, depth + 1)
			off = indent(buf, off, depth)
			off = append(buf, off, "}")
		end
	else
		dump = basic[field.ftype]
		off = append(buf, off, ": ")
		off = dump(buf, off, val)
	end
	-- newline
	off = append(buf, off, "\n")
	return off
end

local function dump_repeated(buf, off, field, arr, depth)
	for i=1, #arr do
		off = dump_field(buf, off, field, arr[i], depth)
	end
	return off
end

function dump_unknown_fields(buf, off, unknowns, depth)
	for i=1,#unknowns do
		local field = unknowns[i]
		-- indent
		off = indent(buf, off, depth)
		-- dump field name
		off = append(buf, off, tostring(field.tag))
		-- dump field
		local dump = wire_types[field.wire]
		if not dump then
			error("Invalid unknown field wire_type=" .. tostring(field.wire))
		end
		off = dump(buf, off, field.value, depth)
		-- newline
		off = append(buf, off, "\n")
	end
	return off
end

local function dump_fields(buf, off, msg, fields, depth)
	local data = msg['.data']
	for i=1,#fields do
		local field = fields[i]
		local val = data[field.name]
		if val then
			if field.is_repeated then
				-- dump repeated field
				off = dump_repeated(buf, off, field, val, depth)
			else -- is basic type.
				off = dump_field(buf, off, field, val, depth)
			end
		end
	end
	-- dump unknown fields
	local unknowns = data.unknown_fields
	if unknowns then
		return dump_unknown_fields(buf, off, unknowns, depth)
	end

	return off
end

local function group(buf, off, msg, fields, depth)
	-- dump group fields.
	return dump_fields(buf, off, msg, fields, depth)
end

local function message(buf, off, msg, fields, depth)
	-- dump message fields.
	return dump_fields(buf, off, msg, fields, depth)
end

local register_fields

local function get_type_dump(mt)
	local dump = mt.dump
	-- check if this type has a dump function.
	if not dump then
		-- create a dump function for this type.
		if mt.is_enum then
			dump = basic.enum
		elseif mt.is_message then
			local fields = mt.fields
			dump = function(buf, off, msg, depth)
				return message(buf, off, msg, fields, depth)
			end
			register_fields(mt, fields)
		elseif mt.is_group then
			local fields = mt.fields
			dump = function(buf, off, msg, depth)
				return group(buf, off, msg, fields, depth)
			end
			register_fields(mt, fields)
		end
		-- cache dump function.
		mt.dump = dump
	end
	return dump
end

function register_fields(mt, fields)
	-- check if the fields where already registered.
	if mt.dump then return end
	for i=1,#fields do
		local field = fields[i]
		-- check if the field is a user type
		local user_type_mt = field.user_type_mt
		if user_type_mt then
			field.dump = get_type_dump(user_type_mt)
		end
	end
end

function register_msg(mt)
	local fields = mt.fields
	-- setup 'dump' function for this message type.
	get_type_dump(mt)
	-- create encode callback closure for this message type.
	return function(msg, depth)
		local buf = new_buffer()

		local off = message(buf, 0, msg, fields, depth or 0)

		local data = buf:pack(1, off, true)
		buf:release()
		return data
	end
end

