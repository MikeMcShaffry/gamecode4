
local lp = require"lpeg"
local scanner = require"pb.proto.scanner"

local patt = lp.Ct(
	--(lp.C(scanner.TOKEN) + scanner.SPACE + scanner.COMMENT)^0
	(lp.C(scanner.ANY))^0
	* (scanner.EOF + scanner.error"invalid character")
)

-- read .proto file.
local f = assert(io.open(arg[1]))

-- parse tokens.
local tokens = patt:match(f:read("*a"))
f:close()

-- dump tokens
for i=1,#tokens do
	io.write(tokens[i])
end

