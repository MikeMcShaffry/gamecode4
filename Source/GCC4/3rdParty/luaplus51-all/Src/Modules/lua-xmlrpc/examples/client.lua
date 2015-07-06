require("xmlrpc.http")

local ok, res = xmlrpc.http.call("http://localhost:12345", "hello_world")
assert(ok, string.format("XML-RPC call failed on client: %s", tostring(res)))

print("Result: " .. tostring(res))

