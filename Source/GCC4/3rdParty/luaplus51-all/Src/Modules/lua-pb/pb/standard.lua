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
local error = error
local type = type
local sformat = string.format
local tsort = table.sort
local setmetatable = setmetatable

local mod_name = ...
local mod_path = string.match(mod_name,".*%.") or ''

local fpack = require(mod_name .. ".pack")
local funpack = require(mod_name .. ".unpack")

local fdump = require(mod_name .. ".dump")

local message = require(mod_name .. ".message")
local def_message, new_message, compile_message = message.def, message.new, message.compile

local utils = require(mod_path .. "utils")
local copy = utils.copy

local handlers = require(mod_path .. "handlers")
local new_handlers = handlers.new

local default_handler_list = {
encode = {
binary = fpack.register_msg,
text = fdump.register_msg,
},
decode = {
binary = funpack.register_msg,
},
}

for _type,callbacks in pairs(default_handler_list) do
	for format, cb in pairs(callbacks) do
		handlers.register(_type, format, cb)
	end
end

-- get the Metatable from a public interface table.  A special private value
-- is used to hid the Metatable from the user.
local mt_tag = {}
local function pub_to_mt(pub)
	return pub[mt_tag]
end

-- get root node
local function get_root(node)
	local parent = node['.parent']
	if parent then
		return get_root(parent)
	end
	-- found root.
	return node
end

-- search a node for a type with 'name'.
local function find_type(node, name)
	local _type
	-- check full name
	_type = node[name]
	if _type then return _type end
	-- check multi-level names (i.e. "OuterMessage.InnerMessage")
	for part in name:gmatch("([^.]+)") do
		_type = node[part]
		if not _type then
			-- part not found, abort search
			return nil
		end
		-- found part, now check it for the next part
		node = _type
	end
	return _type
end

local function check_package_prefix(node, name)
	-- check for package prefix.
	local package = node['.package']
	if package then
		package = package .. '.'
		local plen = #package
		if name:sub(1, plen) == package then
			-- matches, trim package prefix from name.
			return true, name:sub(plen + 1)
		end
		-- name is not in package.
		return false, name
	end
	-- no package prefix.
	return false, name
end

local function resolve_type_internal(node, name)
	-- check current node for type.
	local _type = find_type(node, name)
	if _type then return _type end
	-- check parent.
	local parent = node['.parent']
	if parent then
		return resolve_type_internal(parent, name)
	else
		-- no more parents, at root node.
		-- check if 'name' has the current package prefixed.
		local prefixed, sub_name = check_package_prefix(node, name)
		if prefixed then
			-- search for sub-type
			return resolve_type_internal(node, sub_name)
		end
		-- type not in current package, check imports.
		local imports = node['.imports']
		-- at root node, now check imports.
		if imports then
			for i=1,#imports do
				local import = imports[i].proto
				-- search each import
				_type = resolve_type_internal(imports[i].proto, name)
				if _type then return _type end
			end
		end
	end
	return nil
end

local function resolve_type(node, name)
	-- check for absolute type name.
	if name:sub(1,1) == '.' then
		name = name:sub(2) -- trim '.' from start.
		-- skip to root node.
		node = get_root(node)
	end
	return resolve_type_internal(node, name)
end

local function resolve_fields(node, fields)
	for i=1,#fields do
		local field = fields[i]
		-- check if field needs to be resolved
		if field.need_resolve then
			-- resolve user type
			local user_type = resolve_type(node, field.ftype)
			field.user_type_mt = pub_to_mt(user_type)
			field.need_resolve = nil
		end
	end
end

local function resolve_types(parent, types)
	for i=1,#types do
		local ast = types[i]
		local node = parent[ast.name]
		-- get node's metatable.
		local mt = pub_to_mt(node)
		if mt then
			-- check if node has fields.
			local fields = mt.fields
			if fields then
				resolve_fields(node, fields)
			end
		end
		-- resolve sub-types
		local types = ast.types
		if types then
			resolve_types(node, types)
		end
	end
end

local function compile_types(parent, types)
	for i=1,#types do
		local ast = types[i]
		local node = parent[ast.name]
		-- get node's metatable.
		local mt = pub_to_mt(node)
		if mt then
			-- check if node has fields.
			local fields = mt.fields
			if fields then
				compile_message(node, mt, fields)
			end
		end
		-- compile sub-types
		local types = ast.types
		if types then
			compile_types(node, types)
		end
	end
end

local defines = {}
local function define_types(parent, types)
	if not types then return end
	for i=1,#types do
		local ast = types[i]
		local name = ast.name
		local node_type = ast['.type']
		local define = defines[node_type]
		if define then
			define(parent, name, ast)
		else
			error("No define function for:", node_type)
		end
	end
end

function defines.message(parent, name, ast)
	local mt = def_message(parent, name, ast)

	-- create encode/decode handlers for this message.
	mt.encode = new_handlers('encode', mt)
	mt.decode = new_handlers('decode', mt)

	-- define public interface.
	local pub = setmetatable({
	['.type'] = ast['.type'],
	['.parent'] = parent,
	},{
	-- make the 'public' table callable as the Message contructor.
	__call = function(tab, data)
		return new_message(mt, data)
	end,
	__index = function(tab, key)
		-- if key is the special tag value, then return this type's metatable.
		if key == mt_tag then return mt end
	end,
	-- hid this metatable.
	__metatable = false,
	})

	-- process sub-types
	define_types(pub, ast.types)
	-- add our public interface to parent's public interface
	parent[name] = pub

	return pub
end
defines.group = defines.message

function defines.enum(parent, name, ast)
	local pub = {}
	local values = copy(ast.values)

	-- create enum's metatable.
	local mt = {
	is_enum = true,
	values = values,
	}

	-- define public interface.
	local pub = setmetatable({
	['.type'] = ast['.type'],
	},{
	__index = function(tab, key)
		local val = values[key]
		if val then return val end
		-- if key is the special tag value, then return this type's metatable.
		if key == mt_tag then return mt end
	end,
	__newindex = function()
		error("ENUM read-only can't add/modify ENUM values.")
	end,
	-- hid this metatable.
	__metatable = false,
	})

	-- add our public interface to parent's public interface
	parent[name] = pub

	return pub
end

local function check_extension(extensions, tag)
	for i=1,#extensions do
		local extension = extensions[i]
		if type(extension) == 'number' then
			-- check single extension value.
			if extension == tag then return true end
		else
			-- check range
			local first, last = extension[1], extension[2]
			if first <= tag and tag <= last then return true end
		end
	end
	return false
end

-- field tag sort function.
local function sort_tags(f1, f2)
	return f1.tag < f2.tag
end

function defines.extend(parent, name, ast)
	-- find extended message
	local message = resolve_type(parent, name)
	-- validate extend
	assert(message, "Can't find extended 'message' " .. name)
	assert(message['.type'] == 'message', "Only 'message' types can be extended.")
	-- make sure the extended fields exists as extensions in the extended message.
	local m_mt = pub_to_mt(message)
	local extensions = m_mt.extensions
	assert(extensions, "Extended 'message' type has no extensions, can't extend it.")
	local fields = ast.fields
	-- check that each extend field is an extension in the extended message.
	for i=1,#fields do
		local field = fields[i]
		if not check_extension(extensions, field.tag) then
			-- invalid extension
			error(sformat("Missing extension for field '%s' in extend '%s'", field.name, name))
		end
	end

	local extend = defines.message(parent, name, ast)
	local mt = pub_to_mt(extend)
	-- copy fields from extended message.
	local m_fields = m_mt.fields
	local fields = mt.fields
	local tags = mt.tags
	local fcount = #fields
	for i=1,#m_fields do
		local field = m_fields[i]
		local name = field.name
		-- add to fields
		fcount = fcount + 1
		fields[fcount] = field
		fields[name] = field
		-- add to tags
		tags[field.tag] = field
		tags[field.tag_type] = field
	end
	tsort(fields, sort_tags)

	return extend
end

module(...)

function compile(ast)
	local proto = {
		['.package'] = ast.package,
		['.imports'] = ast.imports,
	}
	-- phaze one: define types.
	define_types(proto, ast.types)
	-- phaze two: resolve fields.
	resolve_types(proto, ast.types)
	-- phaze three: compile fields.
	compile_types(proto, ast.types)
	return proto
end

