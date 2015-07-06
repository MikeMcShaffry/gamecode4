local io = require("io")

local version = assert((...), "Requires rock version on command-line")

local template = [=[
package = "luajson"
version = %VERSION%
source = {
	url = "git://github.com/harningt/luajson.git"
}
description = {
	summary = "customizable JSON decoder/encoder",
	detailed = [[
		LuaJSON is a customizable JSON decoder/encoder using
		LPEG for parsing.
	]],
	homepage = "http://github.com/harningt/luajson",
	maintainer = "Thomas Harning <harningt@gmail.com>",
	license = "MIT/X11"
}
dependencies = {
	"lua >= 5.1",
	"lunit >= 0.4",
	"lpeg >= 0.8.1"
}
build = {
	type = "module",
	modules = {
%MODULES%
	}
}
]=]

local in_modules = io.popen("find lua -type f -name '*.lua' -not -iname '.*' | sort", "r")
local modules = in_modules:read("*a")
in_modules:close()

modules = modules:gsub("lua/([^\n]*)%.lua", function(module)
	return "\t\t[" .. ("%q"):format(module:gsub("/",".")) .. "] = " .. ("%q"):format("lua/" .. module .. ".lua") .. ","
end)
local out = template:gsub("%%(.-)%%", {
	VERSION = ("%q"):format(version),
	MODULES = modules
})
print(out)
