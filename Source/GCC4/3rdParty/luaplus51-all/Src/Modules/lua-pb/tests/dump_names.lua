
local lp = require"lpeg"
local scanner = require"pb.proto.scanner"
local util = require"pb.proto.util"
local grammar = require"pb.proto.grammar"

-- read .proto file.
local f = assert(io.open(arg[1]))
local contents = f:read("*a")
f:close()

local captures = {
Package = function(name, ...) print("Package:", name, ...); return name; end,
Message = function(name, ...) print("Message:", name, ...); return name; end,
Name = util.C,
ID = util.C,
}

local rules = {
--[1] = lp.V'Message',
}

local patt = lp.P(grammar.apply(rules, captures))

--patt = (patt + scanner.ANY)^0
-- parse tokens.
assert(patt:match(contents))

print("Valid .proto file")

