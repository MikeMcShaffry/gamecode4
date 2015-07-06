
local pb = require"pb"

if #arg < 2 then
	print("Usage: " .. arg[0] .. " <input file> <output file>")
	return
end

local f = assert(io.open(arg[1]))
local bin = assert(f:read('*a'))
assert(f:close())

local msg, off = pb.decode_raw( bin)

pb.print(msg)

bin, len = msg:Serialize()

local f = assert(io.open(arg[2], 'w'))
assert(f:write(bin))
assert(f:close())

