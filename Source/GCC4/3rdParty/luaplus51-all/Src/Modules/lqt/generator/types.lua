#!/usr/bin/lua

--[[

Copyright (c) 2007-2009 Mauro Iazzi

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

--]]

local base_types = (...) or {}

local number_type = function(d) return {
	get = function(j)
		return 'lua_tonumber(L, '..tostring(j)..')', 1
	end,
	push = function(j) -- must handle arguments (e.g. in virtual callbacks) and return values
		return 'lua_pushnumber(L, '..tostring(j)..')', 1
	end,
	test = function(j) -- must handle arguments (e.g. in virtual callbacks) and return values
		return 'lqtL_isnumber(L, '..tostring(j)..')', 1
	end,
	onstack = 'number,',
	defect = d,
} end
local integer_type = function(d) return {
	get = function(j)
		return 'lua_tointeger(L, '..tostring(j)..')', 1
	end,
	push = function(j) -- must handle arguments (e.g. in virtual callbacks) and return values
		return 'lua_pushinteger(L, '..tostring(j)..')', 1
	end,
	test = function(j) -- must handle arguments (e.g. in virtual callbacks) and return values
		return 'lqtL_isinteger(L, '..tostring(j)..')', 1
	end,
	onstack = 'integer,',
	defect = d,
} end
local bool_type = function(d) return {
	get = function(j)
		return 'lua_toboolean(L, '..tostring(j)..')', 1
	end,
	push = function(j) -- must handle arguments (e.g. in virtual callbacks) and return values
		return 'lua_pushboolean(L, '..tostring(j)..')', 1
	end,
	test = function(j) -- must handle arguments (e.g. in virtual callbacks) and return values
		return 'lqtL_isboolean(L, '..tostring(j)..')', 1
	end,
	onstack = 'boolean,',
	defect = d,
} end

base_types['char const*'] = {
	get = function(j)
		return 'lua_tostring(L, '..tostring(j)..')', 1
	end,
	push = function(j) -- must handle arguments (e.g. in virtual callbacks) and return values
		return 'lua_pushstring(L, '..tostring(j)..')', 1
	end,
	test = function(j)
		return 'lqtL_isstring(L, '..tostring(j)..')', 1
	end,
	onstack = 'string,',
}
base_types['char'] = integer_type(3)
base_types['unsigned char'] = integer_type(3)
base_types['int'] = integer_type(1)
base_types['unsigned'] = integer_type(1)
base_types['unsigned int'] = integer_type(1)
base_types['short'] = integer_type(2)
base_types['short int'] = integer_type(2)
base_types['unsigned short'] = integer_type(2)
base_types['unsigned short int'] = integer_type(2)
base_types['short unsigned int'] = integer_type(2)
base_types['long'] = integer_type(0)
base_types['unsigned long'] = integer_type(0)
base_types['long int'] = integer_type(0)
base_types['unsigned long int'] = integer_type(0)
base_types['long unsigned int'] = integer_type(0)
base_types['long long'] = integer_type(0)
base_types['unsigned long long'] = integer_type(0)
base_types['long long int'] = integer_type(0)
base_types['unsigned long long int'] = integer_type(0)
base_types['float'] = number_type(1)
base_types['double'] = number_type(0)
base_types['double const&'] = number_type(1)
base_types['bool'] = bool_type()

---[[
base_types['bool*'] = {
	get = function(j)
		return 'lqtL_toboolref(L, '..j..')', 1
	end,
	push = function(j) -- must handle arguments (e.g. in virtual callbacks) and return values
		return 'lua_pushboolean(L, *'..tostring(j)..')', 1
	end,
	test = function(j) -- must handle arguments (e.g. in virtual callbacks) and return values
		return 'lqtL_isboolean(L, '..tostring(j)..')', 1
	end,
	onstack = 'boolean,',
}
base_types['int&'] = {
	get = function(j)
		return '*lqtL_tointref(L, '..j..')', 1
	end,
	push = function(j) -- must handle arguments (e.g. in virtual callbacks) and return values
		return 'lua_pushinteger(L, '..tostring(j)..')', 1
	end,
	test = function(j) -- must handle arguments (e.g. in virtual callbacks) and return values
		return 'lqtL_isinteger(L, '..tostring(j)..')', 1
	end,
	onstack = 'integer,',
}
base_types['int*'] = {
	get = function(j)
		return 'lqtL_tointref(L, '..j..')', 1
	end,
	push = function(j) -- must handle arguments (e.g. in virtual callbacks) and return values
		return 'lua_pushinteger(L, *'..tostring(j)..')', 1
	end,
	test = function(j) -- must handle arguments (e.g. in virtual callbacks) and return values
		return 'lqtL_isinteger(L, '..tostring(j)..')', 1
	end,
	onstack = 'integer,',
}
base_types['char**'] = {
	get = function(j)
		return 'lqtL_toarguments(L, '..tostring(j)..')', 1
	end,
	push = function(j) -- must handle arguments (e.g. in virtual callbacks) and return values
		return 'lqtL_pusharguments(L, '..tostring(j)..')', 1
	end,
	test = function(j) -- must handle arguments (e.g. in virtual callbacks) and return values
		return 'lua_istable(L, '..tostring(j)..')', 1
	end,
	onstack = 'table,',
}
--]]
base_types['std::string const&'] = {
	get = function(i)
		return 'std::string(lua_tostring(L, '..i..'), lua_objlen(L, '..i..'))', 1
	end,
	push = function(i)
		return 'lua_pushlstring(L, '..i..'.c_str(), '..i..'.size())', 1
	end,
	test = function(i)
		return 'lua_isstring(L, '..i..')', 1
	end,
	onstack = 'string,',
}
base_types['std::string'] = base_types['std::string const&']


return base_types
