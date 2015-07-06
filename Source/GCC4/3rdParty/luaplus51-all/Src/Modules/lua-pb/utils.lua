-- Copyright (c) 2010 by Robert G. Jakabosky <bobby@sharedrealm.com>
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

--local getmetatable = debug.getmetatable
local d_getmetatable = debug.getmetatable
local getmetatable = getmetatable
local collectgarbage = collectgarbage
local loadstring = loadstring
local assert = assert
local type = type
local print = print
local tonumber = tonumber
local tostring = tostring
local pairs = pairs
local string = string
local sbyte = string.byte
local io = io

module(...)

--
-- test if string is printable text.
--
local allowed_special = {
  [0] = true, -- null
  [9] = true, -- tab
  [10] = true, -- new line
  [13] = true, -- carriage return
} 
function is_string(bytes)
  local c
  local max = bytes:len()
  for i=1,max do
    c = bytes:byte(i)
    if c >= 127 then -- not ascii character
      return false
    elseif c < 32 then  -- control character range
      if not allowed_special[c] then
        -- control characters between NULL and Space
        return false
      elseif c == 0 and i < max then
        -- null byte only allowed at end.
        return false
      end
    end
  end
  return true
end

--
-- hex dump functions.
--
local write = io.write
local format = string.format
local function printf(fmt, ...)
	return write(format(fmt, ...))
end
local rep = string.rep
function hex_print(buf)
	for byte=1, #buf, 16 do
		local chunk = buf:sub(byte, byte+15)
		printf('%08X  ',byte-1)
		chunk:gsub('.', function (c) printf('%02X ',sbyte(c)) end)
		write(rep(' ',3*(16-#chunk)))
		write(' ',chunk:gsub('[^%w%p ]','.'),"\n") 
	end
end
function hex_dump(buf)
	local out = ''
	for byte=1, #buf, 16 do
		local chunk = buf:sub(byte, byte+15)
		out = out .. format('%08X  ',byte-1)
		out = out .. chunk:gsub('.', function (c) return format('%02X ',sbyte(c)) end)
		out = out .. rep(' ',3*(16-#chunk))
		out = out .. ' ' .. chunk:gsub('[^%w%p ]','.') .. "\n" 
	end
	return out
end

--
-- Recursive Lua value dump.
--
local dump_recur

local function dump_meta_recur(seen, obj, depth, dbg)
	local mt
	if dbg > 1 then
		-- get the true metatable.
		mt = d_getmetatable(obj)
	else
		-- get any un-hidden metatable.
		mt = getmetatable(obj)
	end
	if not mt then return '' end
	local out = ', mt: ' .. tostring(mt) .. ' '
	-- check if this metatable has been seen already.
	if seen[mt] then 
		return out .. "Already dumped..."
	end
	return out .. dump_recur(seen, mt, depth, dbg)
end

function dump_recur(seen, obj, depth, dbg)
	local out
	local t = type(obj)
	-- if not a table just convert to string.
	if t ~= "table" then
		if t == "string" then
			if is_string(obj) then
				out = '"' .. obj .. '"'
			else
				if #obj > 32 then
					out = '<bin>\n' .. hex_dump(obj) .. '\n</bin>'
				else
					out = '<bin>0x' ..
						obj:gsub('.', function (c) return format('%02X',sbyte(c)) end) .. '</bin>'
				end
			end
		else
			out = tostring(obj)
			if dbg > 0 then
				out = out .. dump_meta_recur(seen, obj, depth, dbg) 
			end
		end
		return out
	end
	-- check if table has a __tostring metamethod.
	if dbg == 0 then
		local mt = d_getmetatable(obj)
		if mt then
			local tostr = mt.__tostring
			if tostr then
				out = '"' .. tostr(obj) .. '"'
				return out
			end
		end
	end
	-- check if this table has been seen already.
	if seen[obj] then 
		return "Already dumped " .. tostring(obj)
	end
	seen[obj] = true
	-- restrict max depth.
	if depth >= 10 then
		return "{... max depth reached ...}"
	end
	depth = depth + 1
	-- output table key/value pairs
	local tabs = string.rep("  ",depth)
	local out
	if dbg > 0 then
		out = tostring(obj) .. " {\n"
	else
		out = "{\n"
	end
	for k,v in pairs(obj) do
		if type(k) ~= "number" then
			out = out .. tabs .. '[' .. dump_recur(seen, k, depth, dbg) .. '] = ' ..
				dump_recur(seen, v, depth, dbg) .. ',\n'
		else
			out = out .. tabs .. '[' .. k .. '] = ' .. dump_recur(seen, v, depth, dbg) .. ',\n'
		end
	end
	out = out .. tabs:sub(1,-3) .. "}"
	if dbg > 0 then
		out = out .. dump_meta_recur(seen, obj, depth, dbg)
	end
	return out
end

function dbg_dump(obj, dbg)
	local seen = {}
	return dump_recur(seen, obj, 0, tonumber(dbg or 1))
end

function dump(obj, dbg)
	local seen = {}
	return dump_recur(seen, obj, 0, tonumber(dbg or 0))
end

--
-- Simple table dup function
--
local function copy_recur(seen, tab)
	if type(tab) ~= 'table' then return tab end
	local new = seen[tab]
	-- if this table has been copied.
	if new then return new end
	-- create new table.
	new = {}
	seen[tab] = new
	-- copy values from table to new table.
	for k,v in pairs(tab) do
		k = copy_recur(seen, k)
		v = copy_recur(seen, v)
		new[k] = v
	end
	return new
end

function copy(tab)
	local seen = {}
	return copy_recur(seen, tab)
end

--
-- memory usage functions.
--
local function sizeof2(size, no_gc, test, ...)
	local garbage_size= collectgarbage"count"*1024 - size
	if no_gc then
		collectgarbage"restart"
	end
	collectgarbage"collect"
	collectgarbage"collect"
	size= collectgarbage"count"*1024 - size
	if no_gc then
		print("used size: " .. size .. " garbage: " .. (garbage_size - size))
	else
		print("used size: " .. size)
	end
	test = nil
	return size, ...
end

local function sizeof1(no_gc, test, ...)
	local size=0
	if type(test) == "string" then
		test=assert(loadstring(test))
	end
	collectgarbage"collect"
	collectgarbage"collect"
	if no_gc then
		collectgarbage"stop"
	end
	size=collectgarbage"count"*1024
	return sizeof2(size, no_gc, test, test(...))
end

function sizeof(...)
	return sizeof1(false, ...)
end
function sizeof_no_gc(...)
	return sizeof1(true, ...)
end

