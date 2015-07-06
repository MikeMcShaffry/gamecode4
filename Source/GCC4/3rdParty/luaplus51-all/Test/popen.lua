require "pipe"

p = pipe.popen("dir")
if not p then
	print("unable to open pipe")
	io.exit(0)
end

while true do
	str = p.stdout:read("*l")
	if not str then
		break
	end
	print(str)
end
