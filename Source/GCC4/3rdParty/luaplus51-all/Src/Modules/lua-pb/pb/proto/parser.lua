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

local lower = string.lower
local tremove = table.remove
local tsort = table.sort
local assert = assert

local mod_path = string.match(...,".*%.") or ''

local lp = require"lpeg"
local scanner = require(mod_path .. "scanner")
local grammar = require(mod_path .. "grammar")

-- field tag sort function.
local function sort_tags(f1, f2)
	return f1.tag < f2.tag
end

-- create sub-table if it doesn't exists
local function create(tab, sub_tab)
	if not tab[sub_tab] then
		tab[sub_tab] = {}
	end
	return tab[sub_tab]
end

local Cap = function(...) return ... end
local function node_type(node)
	return node['.type']
end
local function make_node(node_type, node)
	node = node or {}
	node['.type'] = node_type
	return node
end
local function CapNode(ntype, ...)
	local fields = {...}
	local fcount = #fields
	return function(...)
		local node = make_node(ntype, {...})
		local idx = 0
		-- process named fields
		for i=1,fcount do
			local name = fields[i]
			local val = tremove(node, 1)
			node[name] = val
		end
		return node
	end
end

-- process captures from the body of message/group/extend nodes.
local function message_body(...)
	local fields = {}
	local body = {
		fields = fields,
		...
	}
	local types
	local tcount = 0
	local fcount = 0
	-- check for empty message.
	if #body == 1 then
		if type(body[1]) == 'string' then
			body[1] = nil
		end
	end
	-- process sub-nodes
	for i=1,#body do
		-- remove sub-node
		local sub = body[i]
		local sub_type = node_type(sub)
		body[i] = nil
		if sub_type == 'field' then
			-- map 'name' -> field
			fields[sub.name] = sub
			-- map order -> field
			fcount = fcount + 1
			fields[fcount] = sub
		elseif sub_type == 'extensions' then
			local list = create(body, 'extensions')
			local idx = #list
			-- append extensions
			for i=1,#sub do
				local range = sub[i]
				idx = idx + 1
				list[idx] = range
			end
		else
			if tcount == 0 then
				types = create(body, 'types')
			end
			-- map 'name' -> sub-type
			types[sub.name] = sub
			-- add sub-types to array
			tcount = tcount + 1
			types[tcount] = sub
		end
	end
	-- sort fields by tag
	tsort(fields, sort_tags)
	return body
end

local captures = {
[1] = function(...)
	local types = {}
	local proto = {
		types = types,
		...
	}
	local tcount = 0
	for i=1,#proto do
		local sub = proto[i]
		local sub_type = node_type(sub)
		proto[i] = nil
		if sub_type == 'option' then
			create(proto, 'options')
			proto.options[sub.name] = sub.value
		elseif sub_type == 'package' then
			proto.package = sub.name
		elseif sub_type == 'import' then
			local imports = create(proto, 'imports')
			imports[#imports + 1] = sub
		elseif sub_type == 'service' then
			create(proto, 'services')
			proto.services[sub.name] = sub
		else
			-- map 'name' -> type
			types[sub.name] = sub
			-- add types to array
			tcount = tcount + 1
			types[tcount] = sub
		end
	end
	return proto
end,
Package = CapNode("package",
	"name"
),
Import = CapNode("import",
	"file"
),
Option = CapNode("option",
	"name", "value"
),
Message = function(name, body)
	local node = make_node('message', body)
	node.name = name
	return node
end,
MessageBody = message_body,
Extend = function(name, body)
	local node = make_node('extend', body)
	node.name = name
	return node
end,
ExtendBody = message_body,
Group = function(rule, name, tag, body)
	local group_ftype = 'group_' .. name
	local group = make_node('group', body)
	group.name = group_ftype
	group.tag = tag
	local field = make_node('field',{
		rule = rule,
		ftype = group_ftype,
		name = name,
		tag = tag,
		is_group = true,
	})
	return group, field
end,
Enum = function(name, ...)
	local node = make_node('enum', {...})
	local options
	local values = {}
	node.name = name
	node.values = values
	for i=1,#node do
		-- remove next sub-node.
		local sub = node[i]
		local sub_type = node_type(sub)
		node[i] = nil
		-- option/enum_field
		if sub_type == 'option' then
			if not options then
				options = {} -- Enum has options
			end
			options[sub.name] = sub.value
		else
			-- map 'name' -> value
			values[sub[1]] = sub[2]
			-- map value -> 'name'
			values[sub[2]] = sub[1]
		end
	end
	node.options = options
	return node
end,
EnumField = function(...)
	return {...}
end,
Field = function(rule, ftype, name, tag, options)
	local field = make_node('field', {
		rule = rule,
		ftype = ftype,
		name = name,
		tag = tag,
		options = options,
	})
	-- process common options.
	if options then
		field.default = options.default
		field.is_deprecated = options.deprecated
		field.is_packed = options.packed
		if field.is_packed then
			assert(field.rule == 'repeated', "Only 'repeated' fields can be packed.")
		end
	end
	return field
end,
FieldOptions = function(...)
	local options = {...}
	for i=1,#options,2 do
		-- remove next option from list
		local name = options[i]
		options[i] = nil
		local value = options[i+1]
		options[i+1] = nil
		-- set option.
		options[name] = value
	end
	return options
end,
Extensions = CapNode("extensions"
),
Extension = function(first, last)
	if not last then
		-- single value.
		return first
	end
	-- range
	return {first, last}
end,
Service = CapNode("service",
	"name"
),
rpc = CapNode("rpc",
	"name", "request", "response"
),

Name = Cap,
GroupName = Cap,
ID = Cap,
Constant = Cap,
IntLit = tonumber,
SNumLit = tonumber,
StrLit = function(quoted)
	assert(quoted:sub(1,1) == '"')
	return quoted:sub(2,-2)
end,
BoolLit = function(bool)
	bool = lower(bool)
	return (bool == 'true')
end,
FieldRule = Cap,
Type = Cap,
}

local ast_patt = lp.P(grammar.apply({}, captures))
	* (scanner.EOF + scanner.error"invalid character")

module(...)

function parse(contents)
	return ast_patt:match(contents)
end

