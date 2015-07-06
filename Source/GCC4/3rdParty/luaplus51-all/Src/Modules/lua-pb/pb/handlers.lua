-- Copyright (c) 2011, Robert G. Jakabosky <bobby@sharedrealm.com> All rights reserved.

local setmetatable = setmetatable
local rawset = rawset
local error = error
local unpack = unpack
local print = print

-- handler callback registry.
local handler_registry = setmetatable({
}, {
__index = function(reg, _type)
	-- create a new handler type.
	local list = {}
	rawset(reg, _type, list)
	return list
end,
})

module(...)

-- create a new specialized handler
function new(_type, ...)
	local params = {...}
	-- get format list for the handler type.
	local list = handler_registry[_type]
	return setmetatable({}, {
	__index = function(handlers, format)
		-- new format, create a specialized handler for this format.
			-- get callback for this format.
		local cb = list[format]
		if not cb then return nil end
			-- generate specialized handler for this format.
		local handler = cb(unpack(params))
		rawset(handlers, format, handler)
		return handler
	end,
	})
end

function register(_type, format, callback)
	-- get format list for the handler type.
	local list = handler_registry[_type]
	list[format] = callback
end

