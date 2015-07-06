package = "luajson"
version = "0.10-1"
source = {
	url = "http://luaforge.net/frs/download.php/3637/luajson-0.10.tar.bz2",
	md5 = "0b6fa5e3a07daabe79241922b0bfda92"
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
		json = "src/json.lua",
		["json.util"] = "src/json/util.lua",
		["json.decode"] = "src/json/decode.lua",
		["json.decode.array"] = "src/json/decode/array.lua",
		["json.decode.calls"] = "src/json/decode/calls.lua",
		["json.decode.number"] = "src/json/decode/number.lua",
		["json.decode.object"] = "src/json/decode/object.lua",
		["json.decode.others"] = "src/json/decode/others.lua",
		["json.decode.strings"] = "src/json/decode/strings.lua",
		["json.decode.util"] = "src/json/decode/util.lua",
		["json.encode"] = "src/json/encode.lua",
		["json.encode.array"] = "src/json/encode/array.lua",
		["json.encode.calls"] = "src/json/encode/calls.lua",
		["json.encode.number"] = "src/json/encode/number.lua",
		["json.encode.object"] = "src/json/encode/object.lua",
		["json.encode.others"] = "src/json/encode/others.lua",
		["json.encode.strings"] = "src/json/encode/strings.lua"
	}
}
