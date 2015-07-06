require("xavante")
require("xavante.httpd")
require("wsapi.xavante")
require("wsapi.request")
require("xmlrpc")

--- XML-RPC WSAPI handler
-- @param wsapi_env WSAPI environment
function wsapi_handler(wsapi_env)
   local headers = { ["Content-type"] = "text/xml" }
   local req = wsapi.request.new(wsapi_env)
   local method, arg_table = xmlrpc.srvDecode(req.POST.post_data)
   local func = xmlrpc.dispatch(method)
   local result = { pcall(func, unpack(arg_table or {})) }
   local ok = result[1]
   if not ok then
      result = { code = 3, message = result[2] }
   else
      table.remove(result, 1)
      if table.getn(result) == 1 then
         result = result[1]
      end
   end

   local r = xmlrpc.srvEncode(result, not ok)
   headers["Content-length"] = tostring(#r)

   local function xmlrpc_reply(wsapienv)
      coroutine.yield(r)
   end

   return 200, headers, coroutine.wrap(xmlrpc_reply)
end

-- XML-RPC exported functions
xmlrpc_exports = {}

--- Get simple string.
-- @return simple string
function xmlrpc_exports.hello_world()
   return "Hello World"
end

local rules = {{ match = ".", with = wsapi.xavante.makeHandler(wsapi_handler) }}
local config = { server = {host = "*", port = 12345}, defaultHost = { rules = rules} }

xmlrpc.srvMethods(xmlrpc_exports)
xavante.HTTP(config)
xavante.start()
