local reallib
local fakelib
local machine
local extrasymbols = {}

local args = {...}
local errmsg
if args[1] then reallib = args[1] end
if args[2] then fakelib = args[2] end
if args[3] then machine = args[3] end
for i=4,#args do
	table.insert(extrasymbols, args[i])
end

if #args<3 then
	errmsg = "not enough parameters"
elseif not ({X86=true, AMD64=true, IA64=true})[machine] then
	errmsg = "'"..machine.."' is not a recognized machine architecture"
end
if errmsg then
	if errmsg then io.stderr:write("Error: "..errmsg.."\n") end
	io.stderr:write[[
Usage: mkforwardlib <reallib> <fakelib> <machine> [<extrasymbols>...]

    reallib         The name of the existing dll that you want to use.
    fakelib         The name of the fake dll that you want to create and that
                    will call the reallib instead.
    machine         The hardware architecture of your windows version among:
                    X86, AMD64, IA64
    extrasymbols    Additionnal symbols to export beyond standard Lua symbols
                    (if you have a very custom Lua dll).

Example: mkforwardlib lua51.dll lua5.1.dll X86
]]
	-- :TODO: add support for AM33, ARM, CEE, EBC, M32R, MIPS, MIPS16, MIPSFPU, MIPSFPU16, MIPSR41XX, SH4, SH5, THUMB
	os.exit(0)
end

local VisualC = os.getenv('VCPATH')
if not VisualC then
	if machine=='AMD64' then
		local PlatformSDK = assert(os.getenv"MSSDK", "MSSDK environment variable is not set, please set it to the path to your Platform SDK")
		VisualC = PlatformSDK.."\\Bin\\win64\\x86\\AMD64"
	elseif machine=='IA64' then
		local PlatformSDK = assert(os.getenv"MSSDK", "MSSDK environment variable is not set, please set it to the path to your Platform SDK")
		VisualC = PlatformSDK.."\\Bin\\win64"
	else
	--	local VisualCRoot = (os.getenv"PROGRAMFILES(X86)" or os.getenv"PROGRAMFILES").."\\Microsoft Visual Studio .NET 2003\\Vc7"
		local VisualCRoot = (os.getenv"PROGRAMFILES(X86)" or os.getenv"PROGRAMFILES").."\\Microsoft Visual Studio 8\\VC"
		VisualC = VisualCRoot.."\\bin"
	end
end

local symbols = {
	"luaL_addlstring",
	"luaL_addstring",
	"luaL_addvalue",
	"luaL_argerror",
	"luaL_buffinit",
	"luaL_callmeta",
	"luaL_checkany",
	"luaL_checkinteger",
	"luaL_checklstring",
	"luaL_checknumber",
	"luaL_checkoption",
	"luaL_checkstack",
	"luaL_checktype",
	"luaL_checkudata",
	"luaL_error",
	"luaL_findtable",
	"luaL_getmetafield",
	"luaL_gsub",
	"luaL_loadbuffer",
	"luaL_loadfile",
	"luaL_loadstring",
	"luaL_newmetatable",
	"luaL_newstate",
	"luaL_openlib",
	"luaL_openlibs",
	"luaL_optinteger",
	"luaL_optlstring",
	"luaL_optnumber",
	"luaL_prepbuffer",
	"luaL_pushresult",
	"luaL_ref",
	"luaL_register",
	"luaL_typerror",
	"luaL_unref",
	"luaL_where",
	"lua_atpanic",
	"lua_call",
	"lua_checkstack",
	"lua_close",
	"lua_concat",
	"lua_cpcall",
	"lua_createtable",
	"lua_dump",
	"lua_equal",
	"lua_error",
	"lua_gc",
	"lua_getallocf",
	"lua_getfenv",
	"lua_getfield",
	"lua_gethook",
	"lua_gethookcount",
	"lua_gethookmask",
	"lua_getinfo",
	"lua_getlocal",
	"lua_getmetatable",
	"lua_getstack",
	"lua_gettable",
	"lua_gettop",
	"lua_getupvalue",
	"lua_insert",
	"lua_iscfunction",
	"lua_isnumber",
	"lua_isstring",
	"lua_isuserdata",
	"lua_lessthan",
	"lua_load",
	"lua_newstate",
	"lua_newthread",
	"lua_newuserdata",
	"lua_next",
	"lua_objlen",
	"lua_pcall",
	"lua_pushboolean",
	"lua_pushcclosure",
	"lua_pushfstring",
	"lua_pushinteger",
	"lua_pushlightuserdata",
	"lua_pushlstring",
	"lua_pushnil",
	"lua_pushnumber",
	"lua_pushstring",
	"lua_pushthread",
	"lua_pushvalue",
	"lua_pushvfstring",
	"lua_rawequal",
	"lua_rawget",
	"lua_rawgeti",
	"lua_rawset",
	"lua_rawseti",
	"lua_remove",
	"lua_replace",
	"lua_resume",
	"lua_setallocf",
	"lua_setfenv",
	"lua_setfield",
	"lua_sethook",
	"lua_setlocal",
	"lua_setmetatable",
	"lua_settable",
	"lua_settop",
	"lua_setupvalue",
	"lua_status",
	"lua_toboolean",
	"lua_tocfunction",
	"lua_tointeger",
	"lua_tolstring",
	"lua_tonumber",
	"lua_topointer",
	"lua_tothread",
	"lua_touserdata",
	"lua_type",
	"lua_typename",
	"lua_xmove",
	"lua_yield",
--	"luaopen_base",
--	"luaopen_debug",
--	"luaopen_io",
--	"luaopen_math",
--	"luaopen_os",
--	"luaopen_package",
--	"luaopen_string",
--	"luaopen_table",
}

local command = {
	'"'..VisualC..'\\link.exe"',
	" -dll",
	" -nologo",
	" -noentry",
	" -machine:"..machine,
	" -incremental:no",
	" -nodefaultlib",
	" -out:"..fakelib..".dll",
	" -implib:"..fakelib..".lib",
}

for _,symbol in ipairs(symbols) do
	table.insert(command," /export:"..symbol.."="..reallib.."."..symbol)
end
for _,symbol in ipairs(extrasymbols) do
	table.insert(command," /export:"..symbol.."="..reallib.."."..symbol)
end

print(table.concat(command))
os.execute('"'..table.concat(command)..'"')
