
local pb = require"pb"
local decode_msg = pb.decode

if #arg < 1 then
	print("Usage: " .. arg[0] .. " <raw protobuf encoded message>")
	return
end

local f = assert(io.open(arg[1]))
local bin = assert(f:read('*a'))
assert(f:close())

local msg, off = decode_msg(nil, bin)

pb.print(msg)

