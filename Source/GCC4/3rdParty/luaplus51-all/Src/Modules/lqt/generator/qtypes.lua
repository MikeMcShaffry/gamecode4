#!/usr/bin/lua

--[[

Copyright (c) 2007-2008 Mauro Iazzi

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

local qt_types = (...) or {}

--[[
qt_types['QSizeF'] = {
	get = function(i) return 'QSizeF(lua_tonumber(L, '..i..'), lua_tonumber(L, '..i..'+1))', 2, 'QSizeF' end,
	push = function(i) return 'lua_pushnumber(L, '..i..'.width()), lua_pushnumber(L, '..i..'.height())', 2 end,
	test = function(i) return '(lqtL_isnumber(L, '..i..') && lqtL_isnumber(L, '..i..'+1))', 2 end,
	onstack = 'number,number,',
}
qt_types['QSizeF const&'] = qt_types['QSizeF']

qt_types['QSize'] = {
	get = function(i) return 'QSize(lua_tointeger(L, '..i..'), lua_tointeger(L, '..i..'+1))', 2, 'QSize' end,
	push = function(i) return 'lua_pushinteger(L, '..i..'.width()), lua_pushinteger(L, '..i..'.height())', 2 end,
	test = function(i) return '(lqtL_isinteger(L, '..i..') && lqtL_isinteger(L, '..i..'+1))', 2 end,
	onstack = 'integer,integer,',
}
qt_types['QSize const&'] = qt_types['QSize']

qt_types['QPoint'] = {
	get = function(i) return 'QPoint(lua_tointeger(L, '..i..'), lua_tointeger(L, '..i..'+1))', 2, 'QPoint' end,
	push = function(i) return 'lua_pushinteger(L, '..i..'.x()), lua_pushinteger(L, '..i..'.y())', 2 end,
	test = function(i) return '(lqtL_isinteger(L, '..i..') && lqtL_isinteger(L, '..i..'+1))', 2 end,
	onstack = 'integer,integer,',
}
qt_types['QPoint const&'] = qt_types['QPoint']

qt_types['QPointF'] = {
	get = function(i) return 'QPointF(lua_tonumber(L, '..i..'), lua_tonumber(L, '..i..'+1))', 2, 'QPointF' end,
	push = function(i) return 'lua_pushnumber(L, '..i..'.x()), lua_pushnumber(L, '..i..'.y())', 2 end,
	test = function(i) return '(lqtL_isnumber(L, '..i..') && lqtL_isnumber(L, '..i..'+1))', 2 end,
	onstack = 'number,number,',
}
qt_types['QPointF const&'] = qt_types['QPointF']

qt_types['QRect'] = {
	get = function(i) return 'QRect(lua_tointeger(L, '..i..'), lua_tointeger(L, '..i..'+1), lua_tointeger(L, '..i..'+2), lua_tointeger(L, '..i..'+3))', 4, 'QRect' end,
	push = function(i) return 'lua_pushinteger(L, '..i..'.x()), lua_pushinteger(L, '..i..'.y()), lua_pushinteger(L, '..i..'.width()), lua_pushinteger(L, '..i..'.height())', 4 end,
	test = function(i) return '(lqtL_isinteger(L, '..i..') && lqtL_isinteger(L, '..i..'+1) && lqtL_isinteger(L, '..i..'+2) && lqtL_isinteger(L, '..i..'+3))', 4 end,
	onstack = 'integer,integer,integer,integer,',
}
qt_types['QRect const&'] = qt_types['QRect']

qt_types['QRectF'] = {
	get = function(i) return 'QRectF(lua_tonumber(L, '..i..'), lua_tonumber(L, '..i..'+1), lua_tonumber(L, '..i..'+2), lua_tonumber(L, '..i..'+3))', 4, 'QRectF' end,
	push = function(i) return 'lua_pushnumber(L, '..i..'.x()), lua_pushnumber(L, '..i..'.y()), lua_pushnumber(L, '..i..'.width()), lua_pushnumber(L, '..i..'.height())', 4 end,
	test = function(i) return '(lqtL_isnumber(L, '..i..') && lqtL_isnumber(L, '..i..'+1) && lqtL_isnumber(L, '..i..'+2) && lqtL_isnumber(L, '..i..'+3))', 4 end,
	onstack = 'number,number,number,number,',
}
qt_types['QRectF const&'] = qt_types['QRectF']
]]
	
qt_types['QByteArray'] = {
	get = function(i) return 'QByteArray(lua_tostring(L, '..i..'), lua_objlen(L, '..i..'))', 1, 'QByteArray' end,
	push = function(i) return 'lua_pushlstring(L, '..i..'.constData(), '..i..'.size())', 1 end,
	test = function(i) return 'lqtL_isstring(L, '..i..')', 1 end,
	onstack = 'string,',
}
qt_types['QByteArray const&'] = qt_types['QByteArray']

qt_types['QList<QByteArray>'] = {
	get = function(i) return 'lqtL_getStringList(L, '..i..')', 1, 'QList<QByteArray>' end,
	push = function(i) return 'lqtL_pushStringList(L, '..i..')', 1 end,
	test = function(i) return 'lua_istable(L, '..i..')', 1 end,
	onstack = 'table,',
}

if not getmetatable(qt_types) then
	setmetatable(qt_types, {
		__index = function(t, k)
			if type(k)=='string'
				and string.match(k, '^QFlags<[%w:]+>$') then
				local e = string.match(k, '^QFlags<([%w:]+)>$')
				if not qt_types[e] then return nil end
				e = string.gsub(e, '::', '.')
				local ret = {
					get = function(i)
						return '('..k..'(lqtL_getflags(L, '..i..', "'..e..'")))', 1
					end,
					push = function(i)
						return 'lqtL_pushflags(L, '..i..', "'..e..'")', 1
					end,
					test = function(i)
						return 'lqtL_isflags(L, '..i..')', 1
					end,
					onstack = 'table,',
				}
				return ret
			end
		end,
	})
end

return qt_types
