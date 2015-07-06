--[==[
require "windows.reg"

--[[
Obtain keys simply by indexing the windows.reg namespace.
The toplevel keys are:
    HKCR, HKEY_CLASSES_ROOT,
    HKCC, HKEY_CURRENT_CONFIG,
    HKCU, HKEY_CURRENT_USER,
    HKLM, HKEY_LOCAL_MACHINE,
    HKU, HKEY_USERS,
--]]
local key1 = windows.reg.HKLM.Software.Sample
local key2 = windows.reg.HKEY_LOCAL_MACHINE.Software
local key3 = key2.Sample

key1:exists() -- check if key exists
key1:create() -- create key
key1:delete() -- delete key

-- iterate over subkey names
for keyname in key1:subkeys() do
    print(keyname)
end

-- iterate over value names
for valuename in key1:values() do
    print(valuename)
end

-- iterate over values (value names and data)
for valuename, valuedata in key1:values(true) do
    print(valuename, valuedata)
end

print(key1"version") -- query value
key1{version = "20050314T205100"} -- create REG_SZ value
key1:delete("version") -- delete value

key1{path = windows.reg.expand_sz("%windir%\\Lua")} -- create REG_EXPAND_SZ value
key1"path" -- returns "C:\\WINDOWS\\Lua"
local v = key1{"path"} -- get valuedata object
v.type -- returns reg.expand_sz
v.data -- returns "%windir%\\Lua"

--[[ keys
windows.reg.HKCR, windows.reg.HKEY_CLASSES_ROOT
windows.reg.HKCC, windows.reg.HKEY_CURRENT_CONFIG
windows.reg.HKCU, windows.reg.HKEY_CURRENT_USER
windows.reg.HKLM, windows.reg.HKEY_LOCAL_MACHINE
windows.reg.HKU, windows.reg.HKEY_USERS
--]]

--[[ values
reg.dword(num)
reg.qword(num)
reg.string(str)
reg.expand_sz(str)
reg.multi_sz(str1, ...)
reg.binary(str)
reg.binary(hex(str))
reg.type(4, ...)
--]]

--[[ auxiliar
reg.import(filename)
reg.export(key1.HKLM["Software\\Sample"], filename)
--]]
--]==]

local reg = module("windows.reg", package.seeall)
require "ex"
local hkey = windows.hkey

local regkey = {} -- metatable for regkey objects
local valuedatum = {} -- metatable for valuedatum objects
local _hkey, _name, _parent = "\\\\h", "\\\\n", "\\\\p"

-- "windows.sysinfo" required for expanding REG_EXPAND_SZ strings
local function expand(v, t)
	if t == hkey.REG_EXPAND_SZ then
		v = require"windows.sysinfo".ExpandEnvironmentStrings(v)
	end
	return v, t
end

-- ensure that the argument is a regkey
local function checkkey(k)
	if (getmetatable(k) ~= regkey) then
		error("Expected a regkey", 3)
	end
end

-- ensure that the regkey argument is not a top-level key
local function checknottop(k)
	return k[_parent] or error("Can't create or delete top-level keys", 3)
end

-- validate (sub)keyname
local function checkname(name)
	assert(not ({[_hkey]=true,[_name]=true,[_parent]=true})[name],
		"attempt to index " .. tostring(name))
	return true
end

-- a switch "statement"
local function switch(val, ...)
	return function(case)
		local f = case[val] or case.default
		if not f then return nil end
		return f(unpack(arg))
	end
end

-- return the full path of the given regkey
local function fullpath(k)
	checkkey(k)
	local path = k[_name]
	k = k[_parent]
	while k do
		path = k[_name] .. '\\' .. path
		k = k[_parent]
	end
	return path
end

-- for the given key, find and return the closest open parent key followed by
-- the relative path from this parent to the given key
local function relpath(k, path)
--	D("relpath(%s,%s)\n", tostring(k), tostring(path))
	checkkey(k)
	while not k[_hkey] do
		path = k[_name] .. '\\' .. path
		k = k[_parent]
	end
	return k, path
end

-- call the continuation with the hkey; if necessary, open or create the key as
-- specified by creat
local function with_hkey(k, creat, cont, a)
	D("with_hkey(%s,%s,%s)\n", tostring(k), creat, tostring(cont))
	assert(getmetatable(k) == regkey)
	local h = k[_hkey]
	if not h then
		local pk, path, ph, err = relpath(k[_parent], k[_name])
		ph = pk[_hkey]
		h, err = ph[creat](ph, path)
		if h == nil then return nil, err end
		rawset(k, _hkey, h)
	end
	return switch(type(cont), h, cont, a) {
		-- call continuation
		["function"] = function(h, cont, a) return cont(h, a) end,
		-- call hkey method
		["string"] = function(h, cont, a) return h[cont](h, a) end,
		-- return value
		default = function(_, cont) return cont end,
	}
end

-- recursively delete registry key
local function deletekey(h, path)
	local _, err = h:delete(path)
	if _ == nil then
		local sh = h:open(path)
		if sh then
			for sub in sh:enumkeys() do
				deletekey(sh, sub)
			end
			sh:close()
		end
		_, err = h:delete(path)
	end
	return _, err
end

-- recursively delete the key
-- XXX support removing values
local function delete(k, valuename)
--	D("delete(%s)\n", tostring(k))
	checkkey(k)
	checknottop(k)
	if valuename then
		return with_hkey(k, "open", "deletevalue", valuename)
	end
	local pk = k[_parent]
	-- First, close the key
	if k[_hkey] then
		k[_hkey]:close()
		rawset(k, _hkey, false)
	end
--	D("deleting %s\n", tostring(k))
	local pk, path = relpath(pk, k[_name])
	return deletekey(pk[_hkey], path)
end

-- hex encode the string in REGEDIT4 format
local function export_hex(line, len, data)
	if string.len(data) > 0 then
		table.insert(line, string.format("%.2x", string.byte(data)))
		len = len + 2
		--D("        012345678901234567890\n")
		--D("len=%2d>>%s%s<<\n", len, line[1], line[2])
		for i = 2, string.len(data) do
			if len > 75 then
				table.insert(line, string.format(",\\\n  %.2x", string.byte(data, i)))
				len = 4
				--D("        012345678901234567890\n")
				--D("len=%2d>>%s<<\n", len, "  HH")
			else
				table.insert(line, string.format(",%.2x", string.byte(data, i)))
				len = len + 3
			end
		end
		--D("entries: %d>>%s<<\n", table.getn(line), table.concat(line))
		--return table.concat(line)
	end
end

-- quote the string
local function quote(s)
	return string.gsub(s, '"', '\\"')
end

-- convert a 4-byte to a number
local function dword(d)
	return string.byte(d,1) + 256*(string.byte(d,2) + 256*(string.byte(d,3) + 256*string.byte(d,4)))
end

-- recursively export key k named name to file f
local function export_key(f, k, name)
	f:write('[', name, ']\n')
	for name,data,typ in with_hkey(k, "open", "enumvalues") do
		local line = {string.format('"%s"=',name)}
		switch (typ, data, line, typ) {
			[hkey.REG_SZ] = function(data, line)
				table.insert(line, string.format('"%s"', quote(data)))
			end,
			[hkey.REG_DWORD] = function(data, line)
				table.insert(line, string.format('dword:%.8x', dword(data)))
			end,
			[hkey.REG_BINARY] = function(data, line)
				table.insert(line, 'hex:')
				export_hex(line, string.len(line[1]) + string.len(line[2]), data)
			end,
			default = function(data, line, typ)
				table.insert(line, string.format('hex(%x):', typ))
				export_hex(line, string.len(line[1]) + string.len(line[2]), data)
			end
		}
		f:write(table.concat(line), '\n')
	end
	f:write('\n')
	for subkeyname in k:subkeys() do
		export_key(f, k[subkeyname], name .. '\\' .. subkeyname)
	end
end

local function export(k, filename)
	--D("export(%s,%q)\n", tostring(k), filename)
	local f,err = io.open(filename, "w")
	if not f then
		return nil, err
	end
	f:write("REGEDIT4\n\n")
	-- XXX check that the key exists
	local res = export_key(f, k, assert(fullpath(k), "fullpath failed"))
	f:close()
	return res
end

-- index regkeys
function regkey:__index(arg)
--	D("__index:(%s)\n", tostring(arg))
	checkkey(self)
	return switch(type(arg), self, arg) {
		["string"] = function(self, arg)
			checkname(arg)
			-- Perform sanity check on subkey?
			local k = setmetatable( -- make regkey
				{[_hkey]=false, [_name]=arg, [_parent]=self},
				regkey)
			rawset(self, arg, k)
			return k
		end,
		default = function()
			error("String expected", 2)
		end
	}
end

-- k:exists()
-- k:create(subkeyname)
-- k:delete()
-- k:delete(subkeyname)
-- for keyname in k:subkeys() do
-- for valuename, valuedata, valuetype in k:values() do
-- k:export(filename)
local function method(k, methodname, arg)
	D("method(%s,%q,%s)\n", tostring(k), methodname, tostring(arg))
	-- XXX remove it?
	rawset(k, methodname, nil)
	return switch(methodname, k, arg) {
		["exists"] = function(k)
			return with_hkey(k, "open", true)
		end,
		["create"] = function(k)
			return with_hkey(k, "create", true)
		end,
		["delete"] = delete,
		["subkeys"] = function(k)
			return with_hkey(k, "open", "enumkeys")
		end,
		["values"] = function(k)
			return with_hkey(k, "open", "enumvalues")
		end,
		["export"] = export,
		default = function(_, name)
			error("Unknown method: " .. name, 4)
		end,
	}
end

-- set in the given hkey the values in values
local function hkey_setvalues(h, values)
	for vname,vdata in pairs(values) do
		if getmetatable(vdata) == valuedatum then
			h:setvalue(vname, vdata.data, vdata.type)
		else
			h:setvalue(vname, vdata)
		end
	end
	return true
end

-- valuedata = k"valuename" -- EXPAND_SZ strings are automatically expanded
-- value = k{"valuename"} print(value.data, value.type)
-- k{valuename1 = valuedata1, valuename2 = windows.reg.binary("data")}
function regkey:__call(x, y)
	checkkey(self)
	return switch (type(x), self, x, y) {
		["string"] = function(k, x)
			-- k"valuename" -- queryvalue
			return with_hkey(k, "open",
				function(h, valuename)
					return expand(h:queryvalue(valuename))
				end, x)
		end,
		["table"] = function(k, x, y)
			if getmetatable(x) == regkey then
				-- k:method(...)
				return method(x, k[_name], y)
			else
				-- k{...} -- setvalue
				return with_hkey(k, "create", hkey_setvalues, x)
			end
		end,
--		["function"] = function() end
		default = function() error("Expected string or table", 2) end,
	}
end

-- windows.reg.HKLM.Software.FooRun = {}
-- Various functions.
function regkey:__newindex(key, value)
	D("__newindex:(%s,%s,%s)\n", tostring(self), tostring(key), tostring(value))
	return switch (type(value), self[key], value) {
		["table"] = function(k, values)
			-- key = { ... } -- replace/enforce all values
			D("<enforce> %s %s\n", tostring(k), tostring(values))
			return with_hkey(k, "create",
				function(h, values)
					-- remove values not in our table
					for name in h:enumvalues() do
						if values[name] == nil then
							h:deletevalue(name)
						end
					end
					-- set the remaining values
					return hkey_setvalues(h, values)
				end, values)
		end,
		default = function()
			assert(nil, "Cannot assign to registry keys")
		end,
	}
end

-- return a representation for the regkey
function regkey:__tostring()
--	D("__tostring\n")
	checkkey(self)
	return string.format("{%s,%s,%s}",
		tostring(self[_hkey]),
		tostring(self[_parent]),
		self[_name])
end
--[[
--]]

-- functions for making a valuedatum {data=Data, type=Type}
function expand_sz(str)
	return setmetatable({data=str,type=hkey.REG_EXPAND_SZ}, valuedatum)
end
function multi_sz(...)
	return setmetatable(
		{
			data = table.concat(arg, '\0') .. '\0\0',
			type = hkey.REG_MULTI_SZ
		},
		valuedatum)
end
function binary(str)
	return setmetatable(
		{
			data = str,
			type = hkey.REG_BINARY
		},
		valuedatum)
end
function other(o, t)
	if type(o) ~= "table" or type(t) ~= "nil" then
		o = { data=o, type=t }
	end
	return setmetatable(o, valuedatum)
end

local function _path_key(system_user)
	if system_user == 'system' then
		return HKLM.System.CurrentControlSet.Control["Session Manager"].Environment
	elseif system_user == 'user' then
		return HKCU.Environment
	else
		error("Expected 'system' or 'user' as the PATH kind.")
	end
end

function get_path(system_user)
	return _path_key(system_user)("PATH")
end

function set_path(system_user, new_path)
	_path_key(system_user) { PATH = new_path }
	
	-- See http://support.microsoft.com/?kbid=104011
	-- SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM) "Environment", SMTO_ABORTIFHUNG, 5000, &dwReturnValue);
	require 'alien'
	alien.load('user32.dll')
	alien.user32.SendMessageTimeoutA:types('int', 'int', 'int', 'int', 'string', 'int', 'int', 'pointer')
	local buffer = alien.buffer(4)
	return alien.user32.SendMessageTimeoutA(0xffff, 0x001a, 0, "Environment", 0x0002, 5000, buffer) ~= 0
end


-- create our top-level keys
do
	local _G = getfenv(1)
	local function top(full, short)
		local k = setmetatable(
			{[_hkey]=hkey[full], [_name]=full, [_parent]=false},
			regkey)
		_G[full] = k
		_G[short] = k
	end
	top("HKEY_CLASSES_ROOT",   "HKCR")
	top("HKEY_CURRENT_CONFIG", "HKCC")
	top("HKEY_CURRENT_USER",   "HKCU")
	top("HKEY_LOCAL_MACHINE",  "HKLM")
	top("HKEY_USERS",          "HKU")
end

function D(...)
--	io.stderr:write(string.format(unpack(arg)))
end

return reg
