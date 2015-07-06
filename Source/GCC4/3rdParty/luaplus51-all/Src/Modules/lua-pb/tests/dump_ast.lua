
local parser = require"pb.proto.parser"

local utils = require"utils"

-- read .proto file.
local f = assert(io.open(arg[1]))
local contents = f:read("*a")
f:close()

-- parse .proto to AST
local parser = parser.parse(contents)

print(utils.dump(parser))

print("Valid .proto file")

