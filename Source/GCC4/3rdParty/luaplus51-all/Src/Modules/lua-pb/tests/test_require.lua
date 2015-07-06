
local pb = require"pb"

local utils = require"utils"

-- load .proto file.
local proto = require(arg[1])

print(utils.dbg_dump(proto))

print("Valid .proto file")

