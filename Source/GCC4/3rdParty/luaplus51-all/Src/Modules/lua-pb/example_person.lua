-- load lua-pb first.
require"pb"

-- now you can use require to load person.proto
require"person"

local msg = person.Person()
msg.name = "John Doe"
msg.id = 1234
msg.email = "jdoe@example.com"
pb.print(msg)

print("Encode person message to binary.")
local bin = msg:Serialize()
print("bytes =", #bin)

print("Decode person message from binary.")
local msg2 = person.Person():Parse(bin)
pb.print(msg2)

