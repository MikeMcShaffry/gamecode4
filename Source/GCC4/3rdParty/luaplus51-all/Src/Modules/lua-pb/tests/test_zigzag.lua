local d_pack = require'pb.standard.pack'
local d_unpack = require'pb.standard.unpack'
local bit = require'bit'

local zigzag = d_pack.zigzag64
local unzigzag = d_unpack.unzigzag64

local first = arg[1] or -100000000
local last = arg[2] or 100000000
local bits = tonumber(arg[3] or 64)

if bits == 32 then
	zigzag = d_pack.zigzag32
	unzigzag = d_unpack.unzigzag32
else
	bits = 64
end

print(string.format("test range(%d <=> %d) bits=%d", first, last, bits))

for n=first,last do
	local n2 = unzigzag(zigzag(n))
	if n ~= n2 then
		assert(n == n2, string.format('%d ~= %d: zigzag=%d', n, n2, d_pack.zigzag64(n)))
	end
end

