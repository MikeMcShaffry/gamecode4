import "pipe"

p = pipe.popen("cmd")
p.stdin:write("dir\ndir /?\nexit\n")

while true do
	str = p.stdout:read()
	if not str then break end
	print(str)
end

