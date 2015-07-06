---------------------------------------------------------------------
-- XML-RPC server
-- See Copyright Notice in license.html
---------------------------------------------------------------------

local assert, pcall, setmetatable, type, unpack = assert, pcall, setmetatable, type, unpack
local cgilua = require"cgilua"
local os     = require"os"
local string = require"string"
local table  = require"table"
local xmlrpc = require"xmlrpc"

module("xmlrpc.server")

---------------------------------------------------------------------
local function respond (resp)
	cgilua.header ("Date", os.date())
	cgilua.header ("Server", "Me")
	cgilua.header ("Content-length", string.len (resp))
	cgilua.header ("Connection", "close")
	cgilua.contentheader ("text", "xml")
	cgilua.put (resp)
end

---------------------------------------------------------------------
function assert (cond, msg)
	if not cond then
		respond (xmlrpc.srvEncode (
			{ code = 2, message = msg, },
			true
		))
		--os.exit() -- !!!!!!!!!!!
	end
end

cgilua.seterroroutput (function (msg)
	respond (xmlrpc.srvEncode ({ code = 2, message = msg, }, true))
end)

---------------------------------------------------------------------
local function decodedata (doc)
	local method, arg_table = xmlrpc.srvDecode (doc)
	assert (type(method) == "string", "Invalid `method': string expected")
	local t = type(arg_table)
	assert (t == "table" or t == "nil", "Invalid table of arguments: not a table nor nil")

	local func = xmlrpc.dispatch (method)
	assert (type(func) == "function", "Unavailable method")

	return func, (arg_table or {})
end

---------------------------------------------------------------------
local function callfunc (func, arg_table)
	local result = { pcall (func, unpack (arg_table)) }
	local ok = result[1]
	if not ok then
		result = { code = 3, message = result[2], }
	else
		table.remove (result, 1)
		if #result == 1 then
			result = result[1]
		end
	end
	return ok, result
end

---------------------------------------------------------------------
function xmlrpc.server:new()
  local o = { methods = { } }
  setmetatable (o, self)
  self.__index = self
  return o
end

---------------------------------------------------------------------
function xmlrpc.server:register(name, service)
  assert (type(name) == "string", "Invalid `name': string expected")
  self.methods[name] = service
end

---------------------------------------------------------------------
function xmlrpc.server:handle()
	xmlrpc.srvMethods (self.methods)
	local func, arg_table = decodedata (cgi[1])
	local ok, result = callfunc (func, arg_table)
	local r = xmlrpc.srvEncode (result, not ok)
	respond (r)
end
