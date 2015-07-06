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

MyWidget = {
	event = function (b,e,...)
		local mt, env = getmetatable(e), debug.getfenv(e)
		print(mt, env)
		table.foreach(env, print)
		print(b.__qtype,e.__qtype,e:type(),...)
	end,
	__base = { QPushButton=QPushButton },
	__index = function(t, k) return QPushButton.__index(t, k) end,
	__newindex = QPushButton.__newindex,
}

MyWidget.new = function(...)
	local ret = QPushButton.new(...)
	debug.setmetatable(ret, MyWidget)
	print(ret, getmetatable(ret), MyWidget )
	return ret
end

b = MyWidget.new()

print(b, b.__index, '->', type(b.__index(b, "show")))
b:show()

return true, tostring(getmetatable(b)), tostring(MyWidget)


