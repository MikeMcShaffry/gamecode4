
local lp = require"lpeg"
local grammar = require"pb.proto.grammar"

-- read .proto file.
local f = assert(io.open(arg[1]))
local contents = f:read("*a")
f:close()

-- parse tokens.
assert(grammar.check(contents))

print("Valid .proto file")

