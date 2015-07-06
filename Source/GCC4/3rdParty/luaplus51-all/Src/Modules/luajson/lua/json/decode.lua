--[[
	Licensed according to the included 'LICENSE' document
	Author: Thomas Harning Jr <harningt@gmail.com>
]]
local lpeg = require("lpeg")

local error = error

local object = require("json.decode.object")
local array = require("json.decode.array")

local merge = require("json.util").merge
local util = require("json.decode.util")

local setmetatable, getmetatable = setmetatable, getmetatable
local assert = assert
local ipairs, pairs = ipairs, pairs
local string_char = require("string").char

local require = require
module("json.decode")

local modulesToLoad = {
	"array",
	"object",
	"strings",
	"number",
	"calls",
	"others"
}
local loadedModules = {
}

default = {
	unicodeWhitespace = true,
	initialObject = false
}

local modes_defined = { "default", "strict", "simple" }

simple = {}

strict = {
	unicodeWhitespace = true,
	initialObject = true
}

-- Register generic value type
util.register_type("VALUE")
for _,name in ipairs(modulesToLoad) do
	local mod = require("json.decode." .. name)
	for _, mode in pairs(modes_defined) do
		if mod[mode] then
			_M[mode][name] = mod[mode]
		end
	end
	loadedModules[name] = mod
	-- Register types
	if mod.register_types then
		mod.register_types()
	end
end

-- Shift over default into defaultOptions to permit build optimization
local defaultOptions = default
default = nil


local function buildDecoder(mode)
	mode = mode and merge({}, defaultOptions, mode) or defaultOptions
	local ignored = mode.unicodeWhitespace and util.unicode_ignored or util.ascii_ignored
	-- Store 'ignored' in the global options table
	mode.ignored = ignored

	local value_id = util.types.VALUE
	local value_type = lpeg.V(value_id)
	local object_type = lpeg.V(util.types.OBJECT)
	local array_type = lpeg.V(util.types.ARRAY)
	local grammar = {
		[1] = mode.initialObject and (ignored * (object_type + array_type)) or value_type
	}
	for _, name in pairs(modulesToLoad) do
		local mod = loadedModules[name]
		mod.load_types(mode[name], mode, grammar)
	end
	-- HOOK VALUE TYPE WITH WHITESPACE
	grammar[value_id] = ignored * grammar[value_id] * ignored
	grammar = lpeg.P(grammar) * ignored * lpeg.Cp() * -1
	return function(data)
		local ret, next_index = lpeg.match(grammar, data)
		assert(nil ~= next_index, "Invalid JSON data")
		return ret
	end
end

-- Since 'default' is nil, we cannot take map it
local defaultDecoder = buildDecoder(default)
local prebuilt_decoders = {}
for _, mode in pairs(modes_defined) do
	if _M[mode] ~= nil then
		prebuilt_decoders[_M[mode]] = buildDecoder(_M[mode])
	end
end

--[[
Options:
	number => number decode options
	string => string decode options
	array  => array decode options
	object => object decode options
	initialObject => whether or not to require the initial object to be a table/array
	allowUndefined => whether or not to allow undefined values
]]
function getDecoder(mode)
	mode = mode == true and strict or mode or default
	local decoder = mode == nil and defaultDecoder or prebuilt_decoders[mode]
	if decoder then
		return decoder
	end
	return buildDecoder(mode)
end

function decode(data, mode)
	local decoder = getDecoder(mode)
	return decoder(data)
end

local mt = getmetatable(_M) or {}
mt.__call = function(self, ...)
	return decode(...)
end
setmetatable(_M, mt)
