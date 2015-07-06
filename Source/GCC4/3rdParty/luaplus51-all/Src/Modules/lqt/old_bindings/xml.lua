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


local xml_parser = {

strip_escapes = function (self, s)
	s = string.gsub(s, '&gt;', '>')
	s = string.gsub(s, '&lt;', '<')
	return s
end,

parseargs = function (self, s)
	local arg = {}
	string.gsub(s, "([%w_]+)=([\"'])(.-)%2", function (w, _, a)
		arg[self:strip_escapes(w)] = self:strip_escapes(a)
	end)
	return arg
end,

collect = function (self, s)
	local stack = {}
	local top = {}
	table.insert(stack, top)
	local ni,c,label,xarg, empty
	local i, j = 1, 1
	while true do
		ni,j,c,label,xarg, empty = string.find(s, "<(%/?)(%w+)(.-)(%/?)>", j)
		if not ni then break end
		local text = string.sub(s, i, ni-1)
		if not string.find(text, "^%s*$") then
			table.insert(top, text)
		end
		local inserted = nil
		if empty == "/" then  -- empty element tag
			inserted = {tag=label, attr=self:parseargs(xarg), empty=1}
			table.insert(top, inserted)
		elseif c == "" then   -- start tag
			top = {tag=label, attr=self:parseargs(xarg)}
			inserted = top
			table.insert(stack, top)   -- new level
		else  -- end tag
	    local toclose = table.remove(stack)  -- remove top
			top = stack[#stack]
			if #stack < 1 then
				error("nothing to close with "..label)
			end
			if toclose.tag ~= label then
				error("trying to close "..toclose.label.." with "..label)
			end
			table.insert(top, toclose)
		end
		if inserted then
			for a, v in pairs(inserted.attr) do
				if type(self[a])=='table' then
					self[a][v] = inserted
				end
			end
		end
		i = j+1
	end
	local text = string.sub(s, i)
	if not string.find(text, "^%s*$") then
		table.insert(stack[stack.n], text)
	end
	if #stack > 1 then
		error("unclosed "..stack[stack.n].label)
	end
	return stack[1]
end,
}

return xml_parser
