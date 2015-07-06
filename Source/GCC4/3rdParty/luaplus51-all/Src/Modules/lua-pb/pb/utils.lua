-- Copyright (c) 2011 by Robert G. Jakabosky <bobby@sharedrealm.com>
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

local type = type
local pairs = pairs

module(...)

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
	if tab then
		local seen = {}
		return copy_recur(seen, tab)
	end
	return tab
end

