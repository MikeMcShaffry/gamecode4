#!/usr/bin/lua

--[[

Copyright (c) 2007-2009 Mauro Iazzi
Copyright (c)      2008 Peter K?mmel
Copyright (c)      2010 Michal Kottman

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

local osseparator = package.config:sub(1,1)

local path = string.match(arg[0], '(.*'..osseparator..')[^%'..osseparator..']+') or ''
if path == "" then
	--- remove script name
	path = string.sub(arg[0], 1, #arg[0] - #'generator.lua')
end

local filename = nil
local dirname = nil
module_name = nil
template_file = path .. 'qtemplates.lua'
local typefiles = {}
local filterfiles = {}
output_includes = {
	'lqt_common.hpp',
}
output_path = '.'

do
	local i = 1
	while select(i, ...) do
		local argi = select(i, ...)
		if argi=='-n' then
			i = i + 1
			module_name = select(i, ...)
		elseif argi=='-i' then
			i = i + 1
			table.insert(output_includes, (select(i, ...)))
		elseif argi=='-t' then
			i = i + 1
			table.insert(typefiles, (select(i, ...)))
		elseif argi=='-f' then
			i = i + 1
			table.insert(filterfiles, (select(i, ...)))
		elseif argi=='-c' then
			i = i + 1
			template_file = select(i, ...)
		elseif argi=='-D' then
 			i = i + 1
			output_path = select(i, ...)
		else
			filename = filename and error'duplicate filename' or argi
		end
		i = i + 1
	end
end

if not output_path:match('[/\\]$') then output_path = output_path .. '/' end

local my_includes = ''
for _, i in ipairs(output_includes) do
	if string.match(i, '^<.+>$') then
		my_includes = my_includes .. '#include '..i..'\n'
	elseif string.match(i, '^".+"$') then
		my_includes = my_includes .. '#include '..i..'\n'
	else
		my_includes = my_includes .. '#include "'..i..'"\n'
	end
end
output_includes = my_includes .. '\n'

local readfile = function(fn)
	local f = assert(io.open(fn))
	local s = f:read'*a'
	f:close()
	return s
end

function fprint(f)
	return function(...)
		for i = 1, select('#',...) do
			f:write((i==1) and '' or '\t', tostring(select(i,...)))
		end
		f:write'\n'
		f:flush()
	end
end

_src = '_src'..osseparator
local debug = fprint(io.stderr)

local warn = true
local ignore_file = assert(io.open(output_path..'ignores_'..module_name..'.csv', 'w'))
function ignore(name, cause, context)
	if warn then
		ignore_file:write(name..';'..cause..';'..(context or '')..'\n')
	end
end

local xmlstream, idindex = dofile(path..'xml.lua')(readfile(filename))

-- Remove duplicate entries (~4300/20000 for QtCore)
local dups = {}
local remove = {}
for e in pairs(idindex) do
	if e.xarg and e.xarg.id and dups[e.xarg.id] then
		-- print('Duplicate!', dups[e.xarg.id], e.xarg.name, e.xarg.id)
		remove[e] = true
	end
	dups[e.xarg.id] = true
end
for e in pairs(remove) do
	idindex[e] = nil
end

package.path = path..'?.lua;' .. package.path

require('class_types')

----------------------------------------------------------------------------------

typesystem = dofile(path..'types.lua')
do
	local ts = {}
	for i, ft in ipairs(typefiles) do
		ts = assert(loadfile(ft))(ts)
	end
	setmetatable(typesystem, {
		__newindex = function(t, k, v)
			--debug('added type', k)
			ts[k] = v
		end,
		__index = function(t, k)
			local ret = ts[k]
			-- if not ret then debug("unknown type:", tostring(k), ret) end
			return ret
		end,
	})
end


-- helper function
function argument_name(tn, an)
	local ret
	if string.match(tn, '%(%*%)') then
		ret = string.gsub(tn, '%(%*%)', '(*'..an..')', 1)
	elseif string.match(tn, '%[.*%]') then
		ret = string.gsub(tn, '%s*(%[.*%])', ' '..an..'%1')
	else
		ret = tn .. ' ' .. an
	end
	return ret
end

local gen_id = 0
for e in pairs(idindex) do
	if e.xarg and e.xarg.id then
		local id = assert(tonumber(e.xarg.id:match("_(%d+)")))
		if id > gen_id then gen_id = id + 1 end
	end
end	

function next_id() gen_id = gen_id + 1; return "_" .. gen_id end


--- Constructs the code that pushes arguments to the Lua stack.
-- Returns the code as a string, and stack increment. In case that an unknown
-- type is encountered, nil and the unknown type is returned.
function make_pushlines(args)
	local pushlines, stack = '', 0
	for i, a in ipairs(args) do
		if not typesystem[a.xarg.type_name] then return nil, a.xarg.type_name end
		local apush, an = typesystem[a.xarg.type_name].push('arg'..i)
		pushlines = pushlines .. '    ' .. apush .. ';\n'
		stack = stack + an
	end
	return pushlines, stack
end

require 'enums'
require 'classes'

fullnames = {}

for e in pairs(idindex) do
	if e.xarg.fullname then fullnames[e.xarg.fullname] = e end
end

enums.preprocess(idindex)
classes.preprocess(idindex)

enums.process(idindex, typesystem)
classes.process(idindex, typesystem, filterfiles)

------------- BEGIN OUTPUT

enums.output()
classes.output()

--print_openmodule(module_name) -- does that

