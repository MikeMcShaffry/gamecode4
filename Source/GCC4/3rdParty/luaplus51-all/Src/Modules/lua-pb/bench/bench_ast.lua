
local parser = require"pb.proto.parser"
local bench = require"bench.bench"

-- read .proto file.
local f = assert(io.open(arg[1]))
local contents = f:read("*a")
f:close()

bench.memtest(".proto AST parse", 100, parser.parse, contents)
bench.speedtest(".proto AST parse", 10000, parser.parse, contents)

